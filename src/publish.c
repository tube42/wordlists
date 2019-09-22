
/*
 * for parsing and filtering a text file and creating a new in our own binary format
 * we also do some frequency analysis
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "common.h"
#include "wordlist.h"

typedef struct {
	char *name;
	char *vowels, *consonants;
	int min, max;
} config;

/* parsing utils */
void make_lower_case(char *str) {
	while(*str)
	*str++ |= 32;
}


/*******************************************************************************/

int config_check_word(config *c, char *word) {
	int i;

	for(i = 0; word[i]; i++) {
		if(!strchr(c->vowels, word[i]) && !strchr(c->consonants, word[i]))
			return 0;
	}
	return 1;
}

void config_load(char *filename, config *c)
{
	FILE *fp;
	int i, j;
	char line[1024 + 1];
	char *tmp;

	fp = fopen(filename, "r");
	if(!fp) {
		printf("Unable to open config %s for reading\n", filename);
		exit(20);
	}

	memset(c, 0, sizeof(config));

	// read each line
	while(read_line(fp, line, 1024 + 1)) {
	char *key, *data;

		/* ignore empty lines */
		tmp = trim(line);
		if(strlen(tmp) == 0)
			continue;

	data = strchr(line, '=');
	if(!data) {
		fprintf(stderr, "BAD CONFIG: '%s'\n", line);
		exit(20);
	}

	*data = '\0';
	data = trim(data + 1);
	key = trim(line);

	if(!strcmp(key, "min")) c->min = atol(data);
	else if(!strcmp(key, "max")) c->max = atol(data);
	else if(!strcmp(key, "name")) c->name = strdup(data);
	else if(!strcmp(key,"vowels")) c->vowels = strdup(data);
	else if(!strcmp(key,"consonants")) c->consonants = strdup(data);
	else {
		fprintf(stderr, "UNKNWON CONFIGURATION: '%s=%s'\n", key, data);
		exit(20);
	}
	}
	fclose(fp);

	if(!c->name || !c->vowels || !c->consonants) {
		fprintf(stderr, "configuration is not complete\n");
		exit(20);
	}

	// sanity check
	for(i = 0; c->vowels[i]; i++) {
		if(strchr(c->consonants, c->vowels[i]) != NULL) {
			fprintf(stderr, "consonants & vowels overlap: '%s' vs '%s'\n",
				c->consonants, c->vowels);
				exit(20);
		}
	}

	// dump configuration:
	fprintf(stderr, "CONFIGURATION IS:\n");
	fprintf(stderr, " name				 = %s\n", c->name);
	fprintf(stderr, " min				  = %d\n", c->min);
	fprintf(stderr, " max				  = %d\n", c->max);
	fprintf(stderr, " letters			  = %s + %s\n", c->vowels, c->consonants);
}


/*******************************************************************************/

void words_load_and_filter(buffer *mem, config *c, char *filename)
{
	FILE *fp;
	char buffer[1024 + 1];
	char *tmp;
	int len, total = 0;

	int stats_too_small = 0;
	int stats_too_large = 0;
	int stats_not_ascii = 0;
	int stats_bad_char = 0;


	fp = fopen(filename, "r");
	if(!fp) {
		fprintf(stderr, "Could not open input file '%s' for reading!\n", filename);
		exit(20);
	}

	while( read_line(fp, buffer, 1024+1)) {
	len = strlen(buffer);

	if(!config_check_word(c, buffer)) {
			stats_bad_char++;
			if(stats_bad_char < 10)
				fprintf(stderr, "Word '%s' is outside the charset.\n", buffer);
			continue;
		}

		if(! is_ascii(buffer)) {
			stats_not_ascii++;
			if(stats_not_ascii < 10)
				fprintf(stderr, "Warning: '%s' is not ASCII\n", buffer);
			continue;
		}

	if(len < c->min) {
		stats_too_small++;
		continue;
	}

	if(len > c->max) {
		stats_too_large ++;
		continue;
	}

	make_lower_case(buffer);
	buffer_add( mem, strdup(buffer));
	total++;
	}

	fprintf(stderr, "Finished parsing, kept %d words.\n"
		"Dropped %d+%d words due to size, %d+%d for being non-ascii or outside charset\n",
		total, stats_too_small, stats_too_large,
			stats_not_ascii, stats_bad_char);
	fclose(fp);
}


/*******************************************************************************/

void mappings_load(char *filename, mappings *mp)
{
	FILE *fp;

	if(!filename) {
		mp->count = 0;
	} else {
	if(! (fp = fopen(filename, "rb"))) {
			fprintf(stderr, "Could not open map file '%s' for reading!\n", filename);
			exit(20);
		}
		mappings_read(fp, mp);
		fclose(fp);
	}
}

/*******************************************************************************/

void wordlist_build(buffer *b, config *c, mappings *mp, wordlist *wl)
{
	int i, n;
	int revmap[256];
	char *tmp;

	memset(wl, 0, sizeof(wordlist));
	strncpy(wl->name, c->name, 64);
	wl->count_words = b->curr;
	wl->count_charset = strlen(c->vowels) + strlen(c->consonants);

	// build charset
	wl->chars = malloc( wl->count_charset * sizeof(charinfo));
	if(!wl->chars) {
		fprintf(stderr, "Could not allocate memory for charset\n");
		exit(20);
	}

	for(i = 0; c->vowels[i]; i++) {
		wl->chars[i].code = c->vowels[i];
		wl->chars[i].flags = FLAG_VOWEL;
		wl->chars[i].count = 0;
		wl->chars[i].unicode = c->vowels[i];
	}
	n = i;
	for(i = 0; c->consonants[i]; i++) {
		wl->chars[n + i].code = c->consonants[i];
		wl->chars[n + i].flags = 0;
		wl->chars[n + i].count = 0;
		wl->chars[n +i].unicode = c->consonants[i];
	}

	// create chars reverse map
	for(i = 0; i < 256; i++)
		revmap[i] = -1;
	for(i = 0; i < wl->count_charset; i++)
		revmap[ wl->chars[i].code] = i;

	// add unicode mappings
	for(i = 0; i < wl->count_charset; i++) {
		mapping *m = mappings_find_byte(mp, wl->chars[i].code);
		if(m)
			wl->chars[i].unicode = m->from;
	}

	// compute word and size statistics
	wl->size_words = 2; // for two final '\0'
	for(i = 0; i < b->curr; i++) {
		tmp = b->data[i];
		wl->size_words++; // for '\0';
		while(*tmp) {
			wl->size_words++;
			if(revmap[ *tmp ] == -1) {
				fprintf(stderr, "Internal error on %s\n", b->data[i]);
			}
			wl->chars[ revmap[ *tmp ] ].count++;
			tmp++;
		}
	}

	wl->words = malloc(wl->size_words);
	if(!wl->words) {
		fprintf(stderr, "Could not allocate memory for words\n");
		exit(20);
	}

	sort_words(b);
	tmp = wl->words;
	memset(tmp, 0, wl->size_words);

	for(i = 0; i < b->curr; i++) {
		strcpy(tmp, b->data[i]);
		tmp += strlen(b->data[i]) + 1;
	}
}

void wordlist_save(wordlist *w, char *filename)
{
	FILE *fp;

	int total_size = 0;
	int i;

	if(! (fp = fopen(filename, "wb"))) {
		fprintf(stderr, "Could not open map file '%s' for writing!\n", filename);
		exit(20);
	}

	wordlist_write(fp, w);
	fclose(fp);
}

void show_stats(wordlist *wl)
{
	int i;

	printf("Wordlist: %d words, %d  chars,  %d bytes in total\n",
		wl->count_words, wl->count_charset,  wl->size_words);

	for(i = 0; i < wl->count_charset; i++) {
		printf(" %3d: %c -> %04x flags=%02x count=%d\n", i,
			wl->chars[i].code,
			wl->chars[i].unicode,
			wl->chars[i].flags,
			wl->chars[i].count);
	}
	printf("\n");
}

/*******************************************************************************/

int main(int argc, char **argv)
{
	int i, retv = 20;
	char *name_in = 0;
	char *name_out = 0;
	char *name_config = 0;
	char *name_map = 0;


	config c;
	wordlist wl;
	mappings mp;
	buffer b;

	FILE *fp_in, *fp_out, *fp_config;

	/* parse parameters QUICK & DIRTY style... */
	for(i = 1; i < argc-1; i += 2) {
	if( !strcmp("-i", argv[i])) name_in = argv[i+1];
	else if( !strcmp("-o", argv[i])) name_out = argv[i+1];
	else if( !strcmp("-c", argv[i])) name_config = argv[i+1];
	else if( !strcmp("-m", argv[i])) name_map = argv[i+1];
	else {
		printf("Unknown parameter: %s\n", argv[i]);
		return 3;
	}
	}

	if(!name_in || !name_out || !name_config) {
	printf("Usage: %s -c <input.config> -i <input.txt> -o <output>\n", argv[0]);
	return 3;
	}

	// now load everything
	config_load(name_config, &c);
	mappings_load(name_map, &mp);

	buffer_init( &b);
	words_load_and_filter(&b, &c, name_in);

	wordlist_build(&b, &c, &mp, &wl);

	/* debug */
	show_stats(&wl);


	/* write output and clean up */
	wordlist_save(&wl, name_out);
	buffer_free( &b, 1);

	return 0;
}
