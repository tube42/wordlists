
#ifndef _WORDLIST_H_
#define  _WORDLIST_H_

#define WORDLIST_MAGIC	0x776c6462 /* wldb */

enum {
	FOUND_NONE = 0,
	FOUND_EXACT,
	FOUND_PREFIX
};

#define FLAG_VOWEL 0x01

typedef struct {
	char code;
	unsigned char flags;
	int count;
	int unicode;
} charinfo;

typedef struct {
	/* header */
	int size_words;
	int count_charset;
	int count_words;

	/* data */
	char name[64];
	charinfo *chars;
	char *words;
} wordlist;

extern int read_int32(FILE *fp);
extern void write_int32(FILE *fp, int);

extern wordlist *wordlist_load(FILE *fp);
extern void wordlist_write(FILE *fp, wordlist *list);


extern void wordlist_free(wordlist *);
extern int wordlist_lookup(wordlist *, char *word);

#endif /* _WORDLIST_H_ */
