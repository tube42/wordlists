/*
 * map from UTF-8 to our ASCII format
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"


void do_convert(FILE *fpin, FILE *fpout, mappings *mp)
{
	int c, size, code;
	char buffer[4];
	mapping *m;

	while(!feof(fpin)) {
		c = fgetc(fpin);
		if(c == -1)
			break;

		size = utf8_size_byte(c);
		if(size == 1) {
			fputc(c, fpout);
		} else {
			// read the entire unicode buffer
			buffer[0] = c;
			if(fread(buffer + 1, 1, size - 1, fpin) !=  size - 1) {
				fprintf(stderr, "Unexpeted EOF in the middle of unicode char\n");
				exit(20);
			}

			if(utf8_read(buffer, size, &code) != size) {
				fprintf(stderr, "Error reading unicode char\n");
				exit(20);
			}

			// write its mapping:
			m = mappings_find_code(mp, code);
			if(m) {
				fputc(m->to, fpout);
			} else {
				// no mapping found, just write it as-is
				fwrite(buffer, 1, size, fpout);
			}
		}
	}
}

/* ----------------------------------------------------------------------- */
int main(int argc, char **argv)
{
	int i;
	FILE *fpin, *fpout, *fpmap;
	mappings mp;

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

	mappings_read(fpmap, &mp);
	fclose(fpmap);
   // DEBUG
	printf("LOADED MAPPING:\n");
	for(i = 0; i < mp.count; i++) {
	printf("\t%d: %c -> 0x%x (%c)\n",
			i, mp.items[i].to, mp.items[i].from, mp.items[i].from);
	}


	do_convert(fpin, fpout, &mp);
	fclose(fpin);
	fclose(fpout);


	return 0;

}
