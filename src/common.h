
#ifndef _COMMON_H_
#define  _COMMON_H_


/* configuration, change or uncomment these */

typedef struct {
    unsigned char cnt;
    char code;
    unsigned char data[6];
} onemap;

typedef struct {
    int size;
    onemap maps[40];
} mappings;

typedef struct  {
    int curr, max;
    char **data;
} buffer;

typedef struct {
    int freq;
    int points;
    int is_vowel;
    char letter;
} charstats;

typedef struct {
    int size;   
    int freq_total;
    charstats chars[48];
} liststats;

typedef struct {
    int size;
    char *words;
    char name[64];
    int size_min;
    int size_max;
    int allow_names;
    int allow_abbreviations;
    liststats stats;
    mappings map;
} wordlist;



extern void write_int32(FILE *fp, int);
extern int read_int32(FILE *fp);
extern int read_line(FILE *fp, char *buffer, int size);

extern void buffer_init(buffer *b);
extern void buffer_free(buffer *b, int contents_too);
extern void buffer_add(buffer *b, char *data);
extern int buffer_lookup(buffer *b, char *data);
extern void sort_words( buffer *b);


extern wordlist * wordlist_load(FILE *fp);
extern void wordlist_free(wordlist *);
extern int wordlist_lookup(wordlist *, char *word);


extern int is_space(char c);
extern char *trim(char *str);


#endif /* _COMMON_H_ */
