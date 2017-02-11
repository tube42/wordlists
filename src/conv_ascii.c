/*
 * Convert 8-bit formats
 */

#include <stdio.h>
#include <stdlib.h>


char map1[256];
char map2[256];

/* --------------------------------------------------------------- */

void mapping_read(FILE *fp)
{
    int i, c;
    
    for(i = 0; i < 256; i++) {
        map1[i] = i;
        map2[i] = i;
    }
    
    while(!feof(fp)) {
        do {
            c = fgetc(fp);
            if(c == EOF) return;
        } while(c == '\r' || c == '\n');
        
        i = fgetc(fp);
        
        printf("Mapped %c -> %c\n", c, i); // DEBUG
        
        map1[ c & 0xFF] = i & 0xFF;
        map2[ i & 0xFF] = c & 0xFF;
        
    }
}

/* --------------------------------------------------------------- */

void do_convert(FILE *fpin, FILE *fpout)
{
    int c1, c2;
    
    while(!feof(fpin)) {        
        c1 = fgetc(fpin);
        if(c1 == EOF) break;
        
        c2 = map2[c1 & 0xFF];

        fputc(c2, fpout);                        
    }    
}

/* ----------------------------------------------------------------------- */
int main(int argc, char **argv)
{
    FILE *fpin, *fpout, *fpmap;
    
    if(argc != 4) {
        fprintf(stderr, "Usage: %s <ASCII in file> <map file> <ASCII out file>\n", argv[0]);
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
