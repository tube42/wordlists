
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

/* io helper functions */

int read_line(FILE *fp, char *buffer, int size)
{
	char *tmp;

	for(;;) {
		if( !fgets(buffer, size-1, fp))
			return 0;

		buffer[size] = '\0'; /* just in case */

		// remove enter
		for(tmp = buffer; *tmp != '\0'; tmp++) {
			if(*tmp == '\r' || *tmp == '\n') {
				*tmp = '\0';
				break;
			}
		}

		// remove comments
		if( (tmp = strchr(buffer, '#')) )
			*tmp = '\0';

		if(strlen(buffer) > 0)
			return 1;
	}
}

mapping *mappings_find_code(mappings *mp, int code)
{
	int i;
	for(i = 0; i < mp->count; i++) {
	if(mp->items[i].from == code)
		return &mp->items[i];
	}
	return 0;
}

mapping *mappings_find_byte(mappings *mp, char c)
{
	int i;
	for(i = 0; i < mp->count; i++)
		if(mp->items[i].to == c)
			return &mp->items[i];
	return NULL;
}

void mappings_read(FILE *fp, mappings *mp)
{
	int i, n, m, code;
	char buffer[64];

	for(mp->count = 0; ;mp->count++) {
		if(!read_line(fp, buffer, 64))
			break;

		n = strlen(buffer);
		if(n < 2 || n > 5) {
			fprintf(stderr, "Unexpected line in mapping: '%s'\n", buffer);
			exit(20);
		}

		m = utf8_read(buffer + 1, n - 1, &code);
		if( m != n -1) {
			fprintf(stderr, "Bad unicode or additional data in mapping : '%s'\n", buffer);
			exit(20);
		}
		mp->items[ mp->count].from = code;
		mp->items[ mp->count].to = buffer[0];
	}
}


/****************************************************/

void buffer_init(buffer *b)
{
	b->curr = 0;
	b->max = 0;
	b->data = 0;
}

void buffer_free(buffer *b, int contents_too)
{
	int i;

	if(contents_too)
	for(i = 0; i < b->curr; i++)
		free(b->data[i]);

	free(b->data);
	b->data = 0;
}

void buffer_add(buffer *b, char *data)
{
	if(b->curr >= b->max) {
	b->max = (b->max == 0) ? 1024 : b->max * 2;
	b->data = realloc( b->data, sizeof(char *) * b->max);
	}
	b->data[b->curr] = data;

	b->curr ++;
}

int buffer_lookup(buffer *b, char *data)
{
	int i;

	for(i = 0; i < b->curr; i++)  {
	if(!strcmp( data, b->data[i])) {
		return 1;
	}
	}

	return 0;
}

/********************************************************/

int quicksort_partition(char **data, int p, int r)
{
	int j, i = p -1;
	char *tmp, *x = data[r];

	for(j = p; j < r; j++) {
	if( strcmp(x, data[j]) > 0) {
		tmp = data[++i];
		data[i] = data[j];
		data[j] = tmp;
	}
	}

	tmp = data[++i];
	data[i] = data[r];
	data[r] = tmp;
	return i;
}

void quicksort(char **data, int p, int r)
{

	int q;
	if(p < r) {
	q = quicksort_partition(data, p, r);
	quicksort(data, p, q-1);
	quicksort(data, q+1, r);
	}
}

void sort_words( buffer *b)
{
	int i, j;
	char *tmp;

	/* first scramble the words to avoid quicksort worst case  ... */
	for(i = 0; i < b->curr; i++) {
	j = rand() % b->curr;

	tmp = b->data[i];
	b->data[i] = b->data[j];
	b->data[j] = tmp;
	}

	/* ... now let Tony do his magic */
	quicksort(b->data, 0, b->curr-1);
}



/***************************************************************/

int is_space(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int is_ascii(char *str)
{
	for( ; *str;  str++) {
		if(*str & 0x80)
			return 0;
	}
	return 1;
}

char *trim(char *str)
{
	char *tmp;
	int len;

	while( is_space(*str))
	str++;

	if(*str != '\0') {
	tmp = str + strlen(str) - 1;

	while( tmp >= str && is_space(*tmp))
		tmp--;

	tmp[1] = '\0';
	}

	return str;
}

/* UTF-8 */

static char utf8_maskin[4] = { 0x80, 0xe0, 0xf0, 0xf8};
static char utf8_maskout[4] = { 0x00, 0xc0, 0xe0, 0xf0};

#define UTF8_MASK2_IN 0xC0
#define UTF8_MASK2_OUT 0x80

int utf8_size_byte(char firstbyte) {
	int i;
	for(i = 0; i < 4; i++)
		if( (firstbyte & utf8_maskin[i]) == utf8_maskout[i])
			return i + 1;
	return 0;
}

int utf8_size_code(int code) {
	if(code <= 0x7F) return 1;
	if(code <= 0x77F) return 2;
	if(code <= 0xFFFF) return 3;
	if(code <= 0x10FFFF) return 4;
	return 0;
}

int utf8_read(char *buffer, int len, int *out)
{
	int code;
	int i, size;

	if(len < 1)
		return 0;

	size = utf8_size_byte( buffer[0] );
	if(size > len)
		return -1;

	code = (unsigned char)(buffer[0] & ~utf8_maskin[size - 1]);
	for(i = 1; i < size; i++)
		code = (code << 6) | (unsigned char)(buffer[i] & ~UTF8_MASK2_IN);
	*out = code;
	return size;
}

int utf8_write(int code, char *buffer, int len)
{
	int i, size;

	size = utf8_size_code(code);
	if(size == 0 || size > len)
		return -1;

	for(i = 1; i < size; i++) {
		buffer[size - i] = UTF8_MASK2_OUT | (code & ~UTF8_MASK2_IN);
		code >>= 6;
	}

	buffer[0] = utf8_maskout[size-1] | code;
	return size;
}