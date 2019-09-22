
#ifndef _COMMON_H_
#define _COMMON_H_

typedef struct  {
	int curr, max;
	char **data;
} buffer;

typedef struct {
	int from;
	char to;
} mapping;

typedef struct {
	int count;
	mapping items[44];
} mappings;


extern int read_line(FILE *fp, char *buffer, int size);

extern void buffer_init(buffer *b);
extern void buffer_free(buffer *b, int contents_too);
extern void buffer_add(buffer *b, char *data);
extern int buffer_lookup(buffer *b, char *data);
extern void sort_words( buffer *b);

extern int is_space(char c);
extern int is_ascii(char *str);
extern char *trim(char *str);

extern int utf8_size_byte(char firstbyte);
extern int utf8_size_code(int code);
extern int utf8_read(char *buffer, int len, int *out);
extern int utf8_write(int code, char *buffer, int len);

extern void mappings_read(FILE *fp, mappings *mp);
extern mapping *mappings_find_code(mappings *mp, int code);
extern mapping *mappings_find_byte(mappings *mp, char c);

#endif /* _COMMON_H_ */
