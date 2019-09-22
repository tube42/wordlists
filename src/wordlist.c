
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wordlist.h"

/* helper functions */
int read_all(FILE *fp, void *buffer, int size)
{
	char *tmp = buffer;
	int n, total = 0;
	while(size > 0) {
		n = fread(tmp, 1, size, fp);
		if(n <= 0)
			break;
		tmp += n;
		total += n;
		size -= n;
	}
	return total;
}

int write_all(FILE *fp, void *buffer, int size)
{
	char *tmp = buffer;
	int n, total = 0;
	while(size > 0) {
		n = fwrite(tmp, 1, size, fp);
		if(n <= 0)
			break;
		tmp += n;
		total += n;
		size -= n;
	}
	return total;
}


/* these are here to avoid endianess problems */
int read_int32(FILE *fp)
{
	int i, ret = 0;
	for(i = 0; i < 4; i++) {
	int c = fgetc(fp);
	ret = (ret << 8) | c;
	}
	return ret;
}


void write_int32(FILE *fp, int v)
{
	int i;
	for(i = 0; i < 4; i++) {
	int x = (v >> (24 - i * 8)) & 0xFF;
	fputc(x, fp);
	}
}



/*
 * find using the modified binary search  for continuous data.
 * return 1 if found, -1 if word was found as prefix to a longer word, 0 if not found at all
 */
int wordlist_lookup(wordlist *list, char *word)
{
	int mid, low = 1, high = list->size_words - 1;
	int tmp, n;
	int len = strlen(word);
	int partial = 0;

	char *dict = list->words;

	while(low < high) {
		mid = (low + high) / 2;

		/* first find the real start of the selected string */
		tmp = mid;
		while( tmp > 0 && dict[tmp] != '\0') tmp--;
		tmp++;

		// check for a partial match
		n = strncmp( dict + tmp, word, len);
		if(n == 0) {
			partial = 1;
		}

		n = strcmp(dict + tmp, word);

		if(n == 0) return FOUND_EXACT;
		else if(n < 0) low = mid + 1;
		else high = mid;
	}

	// see if it was partial match or no match at all
	return partial ? FOUND_PREFIX : FOUND_NONE;
}


/* read the whole dictionary into a large array & report its size */
wordlist *wordlist_load(FILE *fp)
{
	int sizechars, i;
	unsigned int magic;
	wordlist *list;

	magic = read_int32(fp);
	if(magic != WORDLIST_MAGIC) {
		return 0;
	}

	list = (wordlist *) malloc( sizeof(wordlist));
	if(list) {
		bzero(list, sizeof(wordlist));
		list->size_words = read_int32(fp);
		list->count_charset = read_int32(fp);
		list->count_words = read_int32(fp);

		sizechars = sizeof(charinfo) * list->count_charset;
		list->chars = malloc(sizechars);
		if(list->chars) {
			list->words = malloc(list->size_words);
			if(list->words) {
				if(read_all(fp, list->name, 64) == 64 &&
					read_all(fp, list->words, list->size_words) == list->size_words) {

					list->name[64-1] = '\0'; /* force string termination */

					for(i = 0; i < list->count_charset; i++) {
						list->chars[i].code = fgetc(fp);
						list->chars[i].flags = fgetc(fp);
						list->chars[i].count = read_int32(fp);
						list->chars[i].unicode = read_int32(fp);
					}
					return list;
				}
				free(list->words);
			}
			free(list->chars);
		}
		free(list);
	}
	return 0;
}

void wordlist_write(FILE *fp, wordlist *list) {
	int i;

	write_int32(fp, WORDLIST_MAGIC);
	write_int32(fp, list->size_words);
	write_int32(fp, list->count_charset);
	write_int32(fp, list->count_words);

	write_all(fp, list->name, 64);
	write_all(fp, list->words, list->size_words);

	for(i = 0; i < list->count_charset; i++) {
		fputc(list->chars[i].code, fp);
		fputc(list->chars[i].flags, fp);
		write_int32(fp, list->chars[i].count);
		write_int32(fp, list->chars[i].unicode);
	 }
}

void wordlist_free(wordlist *l)
{
	if(l) {
	if(l->words) {
		free(l->words);
		l->words = 0;
	}

		if(l->chars) {
		free(l->chars);
		l->chars = 0;
	}
	free(l);
	}
}