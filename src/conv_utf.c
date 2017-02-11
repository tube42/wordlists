/*
 * Convert UTF8 to our ASCII format
 */

#include <stdio.h>
#include <stdlib.h>
/*
$ od -tx1 NounsRus.txt  | head
0000000 d0 b0 d0 b0 d1 80 d0 be d0 bd d0 be d0 b2 d1 89
0000020 d0 b8 d0 bd d0 b0 20 d0 b0 d0 b1 d0 b0 d0 b6 d1
0000040 83 d1 80 20 d0 b0 d0 b1 d0 b0 d0 b7 20 d0 b0 d0
0000060 b1 d0 b0 d0 b7 d0 b8 d1 8f 20 d0 b0 d0 b1 d0 b0
0000100 d0 ba 20 d0 b0 d0 b1 d0 b0 d0 ba d0 b0 20 d0 b0
0000120 d0 b1 d0 b0 d0 bd d0 b4 d0 be d0 bd 20 d0 b0 d0
0000140 b1 d0 b1 d0 b0 d1 82 20 d0 b0 d0 b1 d0 b1 d0 b0
0000160 d1 82 d0 b8 d1 81 d0 b0 20 d0 b0 d0 b1 d0 b1 d0
0000200 b0 d1 82 d0 b8 d1 81 d1 81 d0 b0 20 d0 b0 d0 b1
0000220 d0 b1 d0 b0 d1 82 d1 81 d1 82 d0 b2 d0 be 20 d0
*/

struct utfc {
   int cnt;
   char data[8];     
};
struct mapping {
    struct utfc from;
    char to;
};

struct mapping mappings[44];
int mapping_cnt = 0;


/* --------------------------------------------------------------- */

int utfc_get_bytes(int c)
{
    int cnt = 0;
    
    c &= 0xFF; /* just in case a char is not a char*/
    
    while( c & 0x80) {
        cnt++;
        c <<= 1;
    }
    
    if(!cnt) cnt = 1;
    // if(cnt) cnt--;
    
    return cnt;
}

int utfc_equal(struct utfc *a, struct utfc *b)
{
    int i;
    if(a->cnt != b->cnt) return 0;
    
    for(i = 0; i < a->cnt; i++)
        if(a->data[i] != b->data[i])
            return 0;
    
    return 1;
}

void utfc_write(FILE *fp, struct utfc *c)
{
    int i;    
    for(i = 0; i < c->cnt; i++)
       fputc(c->data[i], fp);     
}

int utfc_read(FILE *fp, struct utfc *o)
{
    int i, c, cnt;
    
    c = fgetc(fp);
    if(c == EOF) {
        o->cnt = 0;
        return 0;
    }
    
    cnt = utfc_get_bytes(c);    
    
    o->data[0] = c;
    o->cnt = cnt;
    for(i = 1; i < cnt; i++)
        o->data[i] = fgetc(fp);
    
    return 1;
}

/* --------------------------------------------------------------- */


struct mapping *mapping_find(struct utfc *c) 
{
    int i;
    for(i = 0; i < mapping_cnt; i++) {        
        struct mapping *m = & mappings[i];
        if(utfc_equal(c, & m->from))
            return m;
    }
    return 0;
}

void mapping_read(FILE *fp)
{
    int i, j, c;
    struct utfc u;
    
    for(mapping_cnt = 0;;mapping_cnt++) {
        do {
            c = fgetc(fp);
        } while(c == '\r' || c == '\n');
        
        if(!utfc_read(fp, &u))
            break;
        
        mappings[mapping_cnt].to = c;
        mappings[mapping_cnt].from = u;        
    }
    
    // DEBUG
    printf("LOADED MAPPING:\n");
    for(i = 0; i < mapping_cnt; i++) {
        printf("%d: %c -> ", i, mappings[i].to);
        for(j = 0; j < mappings[i].from.cnt; j++)
            printf("%c", mappings[i].from.data[j]);
        
        printf("  (");
        for(j = 0; j < mappings[i].from.cnt; j++)
            printf("%d ", 0xFF & mappings[i].from.data[j]);
        printf(")\n");
    }
}

/* --------------------------------------------------------------- */

void do_convert(FILE *fpin, FILE *fpout)
{
    struct utfc u;
    struct mapping *m;
    
    while(!feof(fpin)) {
        if(!utfc_read(fpin, &u))
            break;      
        
        if(u.cnt == 1) {
            char c = u.data[0];
            
            if(c == '\r') continue;
            if(c == '\n' || c == ' ' || c == '\t') {
                fprintf(fpout, "\n");
                continue;        
            }
        }
        
        m = mapping_find(&u);
        if(m) {
            fputc(m->to, fpout);            
        } else {
            int i;
            for(i = 0; i < u.cnt; i++)
                fputc( u.data[i], fpout);
        }
        
            
    }    
}

/* ----------------------------------------------------------------------- */
int main(int argc, char **argv)
{
    FILE *fpin, *fpout, *fpmap;
    
    if(argc != 4) {
        fprintf(stderr, "Usage: %s <UTF8 file> <map file> <ASCII file>\n", argv[0]);
        exit(3);
    }
    
    if(! (fpin = fopen(argv[1], "rb")) ) {
        fprintf(stderr, "Could not open '%s' for reading\n", argv[1]);
        exit(20);
    }
    
    if(! (fpmap = fopen(argv[2], "rb")) ) {
        fprintf(stderr, "Could not open '%s' for reading\n", argv[2]);
        exit(20);
    }
    
    
    if(! (fpout = fopen(argv[3], "wb"))) {
        fprintf(stderr, "Could not open '%s' for writing\n", argv[3]);
        exit(20);
    }
    
    mapping_read(fpmap);
    do_convert(fpin, fpout);
    
    fclose(fpin);
    fclose(fpout);
    fclose(fpmap);
    
    return 0;
    
}
