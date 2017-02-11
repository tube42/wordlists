
/*
 * modify raw text file by adding and removing words
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"



int main(int argc, char **argv)
{
    FILE *fp;
    wordlist *words;
    
    
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <wordlist.bin>\n", argv[0]);
        exit(3);
    }
    
    fp = fopen(argv[1], "rb");
    if(!fp) {
        fprintf(stderr, "Unable to open '%s' for reading\n", argv[1]);
        exit(20);
        
    }
    words = wordlist_load(fp);
    if(words) {
        int i, j, k;
        
        for(k = 0; k < 2; k++) {
            printf("{");
            for(i = 0; i < words->map.size; i++) {
                onemap *map = & words->map.maps[i];
                
                if(i != 0) printf(", ");
                
                if(k == 0)
                    printf("'%c':'", map->code);
                else
                    printf("%d:'", map->code & 0xFF);
                
                for(j = 0; j < map->cnt; j++) 
                    printf("%c", map->data[j]);
                printf("'");
            }
            printf("}\n");
        }
        
    } else {
        fprintf(stderr, "Could not load wordlist from '%s'\n", argv[1]);        
    }
    
    fclose(fp);
    
    return 0;
}

