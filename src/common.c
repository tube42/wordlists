
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

/******************************************************************/

/* 
 * find using the modified binary search  for continuous data.
 * return 1 if found, -1 if word was found as prefix to a longer word, 0 if not found at all
 */
int wordlist_lookup(wordlist *list, char *word)
{
    int mid, low = 1, high = list->size - 1;
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
//            printf("Partial: %s\n", dict + tmp);
        }
        
        n = strcmp(dict + tmp, word);
        
//        printf("%d %d->%d %s<->%s\n", n, mid, tmp, dict+tmp, word); // DEBUG
        
        if(n == 0) return 1;
        else if(n < 0) low = mid + 1;
        else high = mid;
    }
    
    // see if it was partial match or no match at all
    return partial ? -1 : 0;
}


/* read the whole dictionary into a large array & report its size */
wordlist * wordlist_load(FILE *fp)
{
    int i, size = 0;
    wordlist *list;
    liststats *stats;
    char *ret;
    
    list = (wordlist *) malloc( sizeof(wordlist));
    if(!list) return 0;
    
    size = read_int32(fp);
    
    /* minus initial size */
    size -= 4;
    ret = malloc(size);
    if(ret) {
        char *tmp = ret;
        list->words = tmp;
        list->size = size;
        while(size) {
            int n = fread(tmp, 1, size, fp);
            if(n < 1) {
                free(ret);
                return 0;
            }
            size -= n;
            tmp += n;
        }
    }
    
    /* read configuration */
    for(i = 0; i < 64; i++) {
        char c = fgetc(fp);
        list->name[i] = c;
        if(c == '\0') break;
    }
    
    list->size_min = read_int32(fp);
    list->size_max = read_int32(fp);
    list->allow_names = read_int32(fp);
    list->allow_abbreviations = read_int32(fp);
    
    /* now read stats */    
    stats = &(list->stats);
    
    stats->freq_total = 0;    
    stats->size = read_int32(fp);
    if(stats->size < 1 || stats->size >= 48) {
        
        fprintf(stderr, "INTERNAL: bad stats in wordlist\n");
        wordlist_free( list);
        return 0;
    } else {
        for(i = 0; i < stats->size; i++) {
            int tmp = read_int32(fp);
            stats->chars[i].letter = tmp & 0xFF;            
            stats->chars[i].points = (tmp >> 8) & 0xFF;
            stats->chars[i].is_vowel = (tmp & 0x10000) ? 1 : 0;            
            stats->chars[i].freq = read_int32(fp);            
            stats->freq_total += stats->chars[i].freq;
        }
    }
    
    /* read the optinal mappings */
    if(fread(& list->map, 1, sizeof(mappings), fp) != sizeof(mappings)) {
        list->map.size = 0;
    }
    
    fprintf(stderr, "INFO: read %d mappings...\n", list->map.size);
    
    return list;
}

void wordlist_free(wordlist *l)
{
    if(l) {
        if(l->words) {
            free(l->words);
            l->words = 0;
        }
        
        free(l);
    }
}
    
/****************************************************/
// encoding quick hack
void fix_line_encoding(char *str)
{
    char *src = str;
    char *dst = str;
    int c1, c2;
    int i;
/*    
    // debug
    printf("ITS '%s'\n", str);
    for(i = 0; str[i]; i++) printf("\t%02d: %c/%02x\n", i, str[i], str[i]);
  */  
    while(c1 = (*src++) & 0xFF) {
        if((c1 & 0xFF) == 0xC3) {      
            c2 = (*src++) & 0xFF;
            if(c2 == 0xa5) *dst++ = 'å';
            else if(c2 == 0xa4) *dst++ = 'ä';
            else if(c2 == 0xb6) *dst++ = 'ö';
            else if(c2 == 0x85) *dst++ = 'Å';
            else if(c2 == 0x84) *dst++ = 'Ä';
            else if(c2 == 0x96) *dst++ = 'Ö';            
            else {
                fprintf(stderr, "Unknown UTF-8 char in line: %c/%02x in '%s'\n", c2, c2, str);
                exit(20);
            }
        } else {
            *dst++ = c1;
        }
    }
    *dst = '\0';
}
/****************************************************/
/* these are here to avoid endianess problems */
void write_int32(FILE *fp, int v)
{
    int i;
    
    for(i = 0; i < 4; i++) {
        int x = (v >> (24 - i * 8)) & 0xFF;
        fputc(x, fp);
    }    
    
}

int read_int32(FILE *fp)
{
    int i, ret = 0;
    for(i = 0; i < 4; i++) {
        int c = fgetc(fp);        
        ret = (ret << 8) | c;
    }
    return ret;
}

int read_line(FILE *fp, char *buffer, int size)
{
    char *tmp;
    if( !fgets(buffer, size-1, fp)) return 0;
    
    buffer[size] = '\0'; /* just in case */
    
    // remove enter
    for(tmp = buffer; *tmp != '\0'; tmp++) {
        if(*tmp == '\r' || *tmp == '\n') {
            *tmp = '\0';
            break;
        }
    }    
    
    fix_line_encoding(buffer);
    return 1;
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
