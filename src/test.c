
/*
 * this file demonstrates s the search
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "common.h"

/* to fix encoding mismatch problems for the swedish locale */
char encoding_map[256];

void setup_encoding()
{
    int i;
    for(i = 0; i < 256; i++) encoding_map[i] = i;
    
    encoding_map[ 0x86] = 0xe5; // å
    encoding_map[ 0x84] = 0xe4; // ä
    encoding_map[ 0x94] = 0xf6; // ö
    
    encoding_map[ 0x8f] = 0xe5; // Å
    encoding_map[ 0x8e] = 0xe4; // Ä
    encoding_map[ 0x99] = 0xf6; // Ö    
}

void fix_encoding(char *str)
{
    char *tmp = str;
    
    while(*tmp) {
        *tmp = encoding_map[ 0xFF & *tmp];
        *tmp++;
    }
}

/****************************************************************/

// just for testing and to show how slow it is :)
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
    
    for(;;) {
        fprintf(stdout, "Enter word: ");
        fflush(stdout);
        
        if(! read_line(stdin, line, 64+1))
            return;
        
        if(line[0] == '\0')
            return;
        
        fix_encoding(line); // XXX: encoding hach
        
        
        printf("Search result for '%s': %d\n", line, wordlist_lookup(wl, line));
//        printf("Linear: %d\n", linear_search(wl, line));
    }

}

/****************************************************************/

int main(int argc, char **argv)
{
    wordlist *list;
    FILE *fp;
        
    if(argc == 2) {
        fp = fopen(argv[1], "rb");
        if(!fp) {
            printf("Unable top topen %s\n", argv[1]);
            return 20;
        }
    } else {
        printf("Usage %s dictionary.bin\n", argv[0]);
        return 3;
    }
    
    setup_encoding();
    list = wordlist_load(fp);
    {
        // DEBUG
        int i;
        
        printf("Dictionary: '%s', size %d-%d, %s %s\n",
               list->name, list->size_min, list->size_max,
               list->allow_names ? "includes names" : "",
               list->allow_abbreviations ? "includes abbreviations" : ""
               );
        
        for(i = 0; i < list->stats.size; i++) 
            printf("%3d:  %c/%02x -> Freq=%-8d Point=%3d Type=%s\n", i, 
                   list->stats.chars[i].letter,
                   list->stats.chars[i].letter & 0xFF,
                   list->stats.chars[i].freq,
                   list->stats.chars[i].points,
                   list->stats.chars[i].is_vowel ? "vowel" : "consonant"
                   );
    }
    
    if(list) {        
        do_search(list);        
        /* clean up */
        wordlist_free(list);
    } else  printf("Failed to read the dictionary\n");
    
    
    /* Cleanup */
    fclose(fp);
    return 0;
}

