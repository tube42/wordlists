
/*
 * for parsing and filtering a text file and creating a new in our own binary format
 * we also do some frequency analysis
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"


int config_min = 3;
int config_max = 100;
char config_name[64];
int config_allow_names = 1;
int config_allow_abbreviations = 0;
int config_char_ok[256];
int config_char_vowel[256];

int stats_too_small, stats_too_large;
/*******************************************************************************/

/* parsing utils */


int is_uppercase(char c)
{
    return (c & 32) == 0;
}
int is_ok(char *str) {

    while(*str) {
        char c = *str++;
        if( !config_char_ok[ 0xFF & c])  return 0;
    }
    return 1;
}

int is_name(char *str) {
    // first char uppercase

    int i, idx = -1;
    for(i = 0; str[i] != '\0'; i++) {
        if( is_uppercase(str[i]))
            idx = i;
    }

    return  (idx == 1);
}

int is_abbreviation(char *str) {
    // all chars uppercase
    while(*str) {
        char c = *str++;
        if( !is_uppercase(c))
            return 0;
    }
    return 1;
}

void make_lower_case(char *str) {
    while(*str)
        *str++ |= 32;
}


/*******************************************************************************/

void load_config(FILE *fp)
{
    int i, j;
    char line[1024 + 1];
    char *tmp;

    // init configs
    for(i = 0; i < 256; i++)
        config_char_ok[i] = config_char_vowel[i] = 0;


    // read each line
    while(read_line(fp, line, 1024 + 1)) {
        char *key, *data;

        tmp = trim(line);
        if(*tmp == '\0' || *tmp == '#') continue; // empty line or comment

        data = strchr(line, '=');
        if(!data) {
            fprintf(stderr, "BAD CONFIG: '%s'\n", line);
            exit(20);
        }

        *data = '\0';
        data = trim(data + 1);
        key = trim(line);


        if(!strcmp(key, "min")) config_min = atol(data);
        else if(!strcmp(key, "max")) config_max = atol(data);
        else if(!strcmp(key, "name")) strcpy(config_name, data);
        else if(!strcmp(key, "allow_names")) config_allow_names = atol(data);
        else if(!strcmp(key, "allow_abbreviations")) config_allow_abbreviations = atol(data);
        else if(!strcmp(key,"vowels"))
            for(i = 0; data[i] != '\0'; i++) config_char_ok[ 0xFF & data[i]] = config_char_vowel[ 0xFF & data[i]] = 1;
        else if(!strcmp(key,"consonants"))
            for(i = 0; data[i] != '\0'; i++) config_char_ok[ 0xFF & data[i]] = 1;
        else {
            fprintf(stderr, "UNKNWON CONFIGURATION: '%s=%s'\n", key, data);
            exit(20);
        }
    }

    // dump configuration:
    fprintf(stderr, "CONFIGURATION IS:\n");
    fprintf(stderr, " name                 = %s\n", config_name);
    fprintf(stderr, " min                  = %d\n", config_min);
    fprintf(stderr, " max                  = %d\n", config_max);
    fprintf(stderr, " allow_names          = %d\n", config_allow_names);
    fprintf(stderr, " allow_abbreviations  = %d\n", config_allow_abbreviations);

    fprintf(stderr, " letters              = ");
    for(i = 0; i < 256; i++) if(config_char_ok[i]) fprintf(stderr, "%c", i);
    fprintf(stderr, "\n");

    fprintf(stderr, " vowels               = ");
    for(i = 0; i < 256; i++) if(config_char_vowel[i]) fprintf(stderr, "%c", i);
    fprintf(stderr, "\n");

}


/*******************************************************************************/

void filter_and_load_words(FILE *in, buffer *mem)
{
    char buffer[1024 + 1];
    char *tmp;
    int len;
    int total = 0;

    stats_too_small = stats_too_large = 0;

    while( read_line(in, buffer, 1024+1)) {
        //        tmp = strchr(buffer, '/');
        //        if(tmp) *tmp = '\0';

        len = strlen(buffer);

        if(len < config_min) {
            stats_too_small++;
            continue;
        }

        if(len > config_max) {
            stats_too_large ++;
            continue;
        }

        if( ! config_allow_abbreviations && is_abbreviation(buffer))
            continue;

        if( !config_allow_names && is_name(buffer))
            continue;

        make_lower_case(buffer);

        if(!is_ok(buffer))
            continue;

        buffer_add( mem, strdup(buffer));
        total++;
    }

    fprintf(stderr, "Finished parsing. Kept %d words & dropped %d+%d words due to size\n",
            total, stats_too_small, stats_too_large);

}


/*******************************************************************************/


void write_mapping(FILE *out, char *name_map)
{
    mappings m;
    FILE *fp;
    char buffer[64];
    int i;


    memset(&m, 0, sizeof(m));


    if(name_map) {


        fp = fopen(name_map, "rb");
        if(fp) {

            for(;;) {
                int len;
                char *tmp;
                onemap *m2 = &m.maps[m.size];

                if(!fgets(buffer, 60, fp))
                    break;

                tmp = trim(buffer);
                len = strlen(tmp);
                if(len < 2) continue;

                m2->cnt = --len;
                m2->code = *tmp++;
                for(i = 0; i < len; i++)
                    m2->data[i] = *tmp++;
                m.size++;
            }

            fseek(fp, 0, SEEK_END);
            fwrite(&m, 1, sizeof(m), out);
            printf("MAPPING: added %d maps\n", m.size);

            fclose(fp);
        } else {
            fprintf(stderr, "Could not open map file '%s' for reading!\n", name_map);
        }
    }

}

/*******************************************************************************/

void write_stats(FILE *fp, buffer *b)
{
    int i, count;
    int stats[256];
    liststats buffer;

    /* clean stats: */
    for(i = 0; i < 256; i++) stats[i] = 0;


    /* get letter frequency */
    for(i = 0; i < b->curr; i++) {
        char *str = b->data[i];
        while(*str) {
            stats[ 0xFF & *str]++;
            str++;
        }
    }

    count = 0;
    for(i = 0; i < 256; i++)
        if(stats[i])
            count++;


    buffer.size = 0;
    for(i = 0; i < 256; i++) {
        if( stats[i]) {
            charstats *st = &(buffer.chars[ buffer.size++]);
            st->freq = stats[i];
            st->letter = i;
            st->points = 0;
            st->is_vowel = config_char_vowel[ 0xFF & i];
        }
    }

    /* sort it (bubble sort FTL) */
    {
        int swapped;
        do {
            swapped = 0;
            for(i = 1; i < buffer.size; i++) {
                if(buffer.chars[i-1].freq < buffer.chars[i].freq) {
                    charstats tmp = buffer.chars[i-1];
                    buffer.chars[i-1] = buffer.chars[i];
                    buffer.chars[i] = tmp;
                    swapped = 1;
                }
            }
        } while(swapped);
    }
    /* set points */
    {
        int p = 0, c;
        int j = buffer.size;

        for(i = 0; i < buffer.size;)  {
            j /= 4;
            if(j < 2) j = 2;
            p ++;

            for(c = 0; c < j && i < buffer.size; c++)
                buffer.chars[i++].points = p;

        }
    }

    /* write freq data to stream */
    write_int32(fp, buffer.size);
    for(i = 0; i < buffer.size; i++) {
        int tmp = (buffer.chars[i].points << 8) | (0xFF & (int)buffer.chars[i].letter);
        if(buffer.chars[i].is_vowel) tmp |= 0x10000;

        write_int32( fp, tmp);
        write_int32( fp, buffer.chars[i].freq);

    }
}


void write_words(FILE *fp, buffer *b, char *name_map)
{
    int total_size = 0;
    int i;

    fwrite ( "????", 5, 1, fp);
    total_size += 5;

    for(i = 0; i < b->curr; i++) {
        char *str = b->data[i];
        int len = strlen(str);
        /* note that we also include the trailing '\0' */
        fwrite(str, len+1, 1, fp);
        total_size += len + 1;
    }

    /* write configuration */
    fwrite( config_name, strlen(config_name) + 1, 1, fp);
    write_int32(fp, config_min);
    write_int32(fp, config_max);
    write_int32(fp, config_allow_names);
    write_int32(fp, config_allow_abbreviations);


    /* write stats */
    write_stats(fp, b);

    /* write the mapping */
    write_mapping(fp, name_map);

    /* now write file size: */
    fseek(fp, 0, SEEK_SET);


    /*
     * could do this fwrite( &total_size, 4, 1, out);
     * but we are taking no chances with endianness:
     */
    write_int32(fp, total_size);


}

void show_stats(buffer *b)
{
    int i;
    int len_cnt[128];

    for(i = 0; i < 128; i++) len_cnt[i] = 0;

    for(i = 0; i < b->curr; i++) {
        char *str = b->data[i];
        int len = strlen(str);

        len_cnt[len]++;
    }

    printf("Word statistics:\n");
    for(i = 0; i < 128; i++) {
        if(len_cnt[i])
            printf("  - words with %d letters: %d (%d%%)\n",
                   i, len_cnt[i], len_cnt[i] * 100 / b->curr);
    }

    printf("\n");
}
/*******************************************************************************/

int main(int argc, char **argv)
{
    int i, retv = 20;
    char *name_in = 0;
    char *name_out = 0;
    char *name_config = 0;
    char *name_map = 0;
    FILE *fp_in, *fp_out, *fp_config;

    /* parse parameters QUICK & DIRTY style... */
    for(i = 1; i < argc-1; i += 2) {
        if( !strcmp("-i", argv[i])) name_in = argv[i+1];
        else if( !strcmp("-o", argv[i])) name_out = argv[i+1];
        else if( !strcmp("-c", argv[i])) name_config = argv[i+1];
        else if( !strcmp("-m", argv[i])) name_map = argv[i+1];
        else {
            printf("Unknown parameter: %s\n", argv[i]);
            return 3;
        }
    }


    if(!name_in || !name_out || !name_config) {
        printf("Usage: %s -c <input.config> -i <input.txt> -o <output>\n", argv[0]);
        return 3;
    }

    fp_config = fopen(name_config, "r");
    if(fp_config) {
        load_config(fp_config);
        fclose(fp_config);

        fp_in = fopen(name_in, "r");
        if(fp_in) {
            fp_out = fopen(name_out, "wb");
            if(fp_out) {
                buffer b;
                buffer_init( &b);

                filter_and_load_words(fp_in, &b);
                sort_words( &b);


                write_words(fp_out, &b, name_map);

                /* debug */
                show_stats(&b);

                /* clean up: */
                buffer_free( &b, 1);
                retv = 0;

                fclose(fp_out);
            } else printf("Unable to open %s for writing\n", name_out);
            fclose(fp_in);
        } else printf("Unable to open dictionary %s for reading\n", name_in);
    } else printf("Unable to open config %s for reading\n", name_config);

    return retv;
}
