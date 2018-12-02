
/*
 * modify raw text file by adding and removing words
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"


buffer words_all, words_add, words_remove;



void add_words_to(buffer *to, buffer *from)
{

    buffer tmp;
    int i;

    buffer_init(&tmp);


    sort_words(to);
    for(i = 0; i < from->curr; i++) {
        if(!buffer_lookup(to, from->data[i]) ) {
            printf("ADDING %s\n", from->data[i]);
            buffer_add( &tmp, from->data[i]);
        }
    }

    for(i = 0; i < tmp.curr; i++) {
        buffer_add( to, tmp.data[i]);
    }

    buffer_free(&tmp, 0);
}

void load_words(char *filename, buffer *words, int critical)
{
    FILE *fp;
    char line[1024 + 1], *word;
    buffer tmp;
    int i;

    fp = fopen(filename, "r");
    if(!fp) {
        if(!critical) {
            fprintf(stderr, "WARNING: could not open '%s'\n", filename);
            return;
        } else {
            fprintf(stderr, "ERROR: Unable to open '%s' for reading\n", filename);
            exit(20);
        }
    }

    /* sort the old words so we can use lookup*/
    sort_words(words);

    buffer_init(&tmp);
    for(;;) {
        int n = read_line(fp, line, 1024);
        if(n <= 0) break;

        word = trim(line);
        if(*word == '\0') continue; /* empty line */

        if(!buffer_lookup(words, word))
            buffer_add(&tmp, strdup(word) );
    }
    fclose(fp);

    /* add temp buffer to output buffer */
    for(i = 0; i < tmp.curr; i++)
        buffer_add(words, tmp.data[i]);

    buffer_free(&tmp, 0);

}

void save_words_without(char *filename, buffer *words, buffer *without)
{
    int i, rcnt;
    FILE *fp;

    fp = fopen(filename, "w");
    if(!fp) {
        fprintf(stderr, "ERROR: unable to open '%s' for writing\n", filename);
        exit(20);
    }

    sort_words(words);
    sort_words(without);

    /* print some of the removed words */
#define MAX_REMOVE_PRINT 10
    for(rcnt = 0, i = 0; i < words->curr; i++) {
      if(!buffer_lookup(without, words->data[i])) {
	fprintf(fp, "%s\n", words->data[i]);
      } else {
	rcnt++;
	if(rcnt < MAX_REMOVE_PRINT) {
	  printf("REMOVING: %s\n", words->data[i]);
	} else {
	  printf("REMOVING: ...\n");
	  break;
	}
      }
    }

    fclose(fp);
}

/****************************************************************/

int main(int argc, char **argv)
{
    int i, count;
    char *str_in = 0;
    char *str_out = 0;


    buffer_init(& words_add);
    buffer_init(& words_remove);
    buffer_init(& words_all);

    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if( i >= argc -1) {
                fprintf(stderr, "ERROR: missing file argument: '%s'\n", argv[i]);
                return 2;
            }
            switch(argv[i][1]) {
            case 'i':
                if(!str_in) {
                    str_in = argv[++i];
                    load_words(str_in, &words_all, 1);
                } else {
                    fprintf(stderr, "ERROR: input specified multiple times\n");
                    return 3;
                }
                break;

            case 'o':
                if(!str_out) {
                    str_out = argv[++i];
                } else {
                    fprintf(stderr, "ERROR: out specified multiple times\n");
                    return 3;
                }
                break;

            case 'a':
                load_words(argv[++i], &words_add, 0);
                break;

            case 'r':
                load_words(argv[++i], &words_remove, 0);
                break;

            default:
                fprintf(stderr, "ERROR: unknown option_: '%s'\n", argv[i]);
                return 3;
            }

        } else {
            fprintf(stderr, "ERROR: unknown argument: '%s'\n", argv[i]);
            return 3;
        }
    }


    printf("Loaded %d words, %d added, %d removed\n",
           words_all.curr, words_add.curr, words_remove.curr);

    add_words_to(& words_all, &words_add);


    save_words_without(str_out, &words_all, &words_remove);

    return 0;
}
