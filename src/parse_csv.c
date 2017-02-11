
/*
 * parse CSV files and extract words
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"


int main(int argc, char **argv)
{    
    char buffer[1024+1];
    
    while( read_line(stdin, buffer, 1024+1)) {
        char *tmp = strchr(buffer, ',');
        char *tmp2;
        
        if(!tmp) continue;
        
        while( tmp > buffer && (*tmp == ',' || *tmp == '\"' || is_space(*tmp)) ) 
            tmp--;
        
        tmp++;
        *tmp = '\0';
        
        
        tmp2 = buffer;
        while(tmp2 < tmp && (*tmp2 == ',' || *tmp2 == '\"' || is_space(*tmp2)))
            tmp2++;
        
        if(tmp2 == tmp) continue;
                    
        printf("%s\n", tmp2);
    }

    return 0;
}

