
/*
 * this file demonstrates the search
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "wordlist.h"


int mapping_find_code(wordlist *wl, int code, char *out)
{
	int i;
	for(i = 0; i < wl->count_charset; i++) {
		if(wl->chars[i].unicode == code) {
			*out = wl->chars[i].code;
			return 1;
		}
	}
	return 0;
}

int remove_unicode(wordlist *wl, char *str)
{
	int n, m , code, seen;
	char *in;

	seen = 0;
	n = strlen(str);
	in = str;

	while(n > 0) {
		m = utf8_read(str, n, &code);
		if(m > 1) {
			if(!mapping_find_code(wl, code, in++)) {
				fprintf(stderr, "Found unicode not in this language: %02x\n", code);
				return 0;
			}
		} else {
			*in++ = code;
		}
		n -= m;
		str += m;
	}
	*in = '\0'; /* terminate it */
	return seen;
}

/* just for testing */
int linear_search(wordlist *wl, char *str)
{
	char *tmp = wl->words +1;

	while(*tmp != '\0') {
	if(! strcmp(tmp, str)) return 1;
	while(*tmp++ != '\0') ;
	}
	return 0;
}

void do_search(wordlist *wl)
{
	char line[64 + 1];
	int s1, s2;
	long t1, t2;

	for(;;) {
	fprintf(stdout, "Enter word (press ^C or type . to exit): ");
	fflush(stdout);

	if(! read_line(stdin, line, 64+1))
		return;

		if(!strcmp(line, ".") || strlen(line) == 0)
		return;

		if(remove_unicode(wl, line)) {
			printf("Replaced input with '%s' to remove unicode\n", line);
		}

		t1 = clock();
		s1 =  wordlist_lookup(wl, line);
		t1 = clock() - t1;

		t2 = clock();
		s2 = linear_search(wl, line);
		t2 = clock() - t2;

	printf("Search result for '%s':\n", line);
		printf("  Binary: %3d, %5d ms\n", s1, t1);
		printf("  Linear: %3d, %5d ms\n", s2, t2);
	}
}

/****************************************************************/

int main(int argc, char **argv)
{
	wordlist *wl;
	FILE *fp;

	if(argc != 2) {
		printf("Usage %s <wordlist.bin>\n", argv[0]);
		exit(20);
	}

	fp = fopen(argv[1], "rb");
	if(!fp) {
		fprintf(stderr, "Unable to open %s\n", argv[1]);
		exit(20);
	}

	wl = wordlist_load(fp);
	fclose(fp);

	printf("Loaded wordlist %s: %d words, %d  chars & %d bytes in total\n",
		wl->name, wl->count_words, wl->count_charset, wl->size_words);

	do_search(wl);
	wordlist_free(wl);
	return 0;
}

