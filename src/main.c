/*

    arfminesweeper: Cross-plataform multi-frontend game
    Copyright (C) 2023 arf20 (Ángel Ruiz Fernandez)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    main.c: Program entry point

*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "assembler.h"

void
usage(char *name) {
    fprintf(stderr, "Usage: %s [options] file\nOptions\n"
    "  -v\t\tVerbose output.\n  -g\t\tGenerate debug symbols for arfmipssim.\n  -o <file>\tPlace the output into <file>.\n", name);
}

char *
read_whole_file(FILE *f, size_t *sz) {
    fseek(f, 0, SEEK_END);
    *sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buff = malloc(*sz);
    fread(buff, *sz, 1, f);

    fclose(f);

    return buff;
}

void
print_symbols(segment_t *segs) {
    printf("=== SYMBOL TABLE ===\nsegment\n  label           address\n"
    "----------------------------\n");
    for (int i = 0; i < 2; i++) { /* 2 segments */
        switch (segs[i].id) {
            case SEG_DATA: printf(".data [%d]\n", segs[i].size); break;
            case SEG_TEXT: printf(".text [%d]\n", segs[i].size); break;
        }
        
        for (int j = 0; j < segs[i].symbols->size; j++) {
            symbol_t s = segs[i].symbols->table[j];
            printf("  %s:", s.label);
            printf("%.*s", 16 - strlen(s.label) - 1, "                ");
            printf("0x%.8x\n", s.address);
        }
    }
    printf("\n");
}

void
write_symbols(symbol_table_t *st, FILE *f) {
    for (int i = 0; i < st->size; i++) {
        fprintf(f, "%s:0x%.8x\n", st->table[i].label,
            st->table[i].address);
    }
}

void
dump_segments(segment_t *segs) {
    printf("=== SEGMENT DUMP ===\n");
    for (int i = 0; i < 2; i++) { /* 2 segments */
        addr_t org;
        switch (segs[i].id) {
            case SEG_DATA: {
                printf(".data", segs[i].size);
                org = DATA_ORG;
            } break;
            case SEG_TEXT: {
                printf(".text", segs[i].size); 
                org = TEXT_ORG;
            } break;
        }
        
        printf("    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");

        int j = 0;
        for (; j < segs[i].size; j++) {
            if (j % 16 == 0) {
                if (j != 0) {
                    printf("  |");
                    for (int k = j - 16; k < j; k++)
                        isprint(segs[i].data[k]) ? putchar(segs[i].data[k]) : putchar('.');
                    printf("|");
                }
                printf("\n%.8x ", org + j);
            }

            printf("%.2x ", segs[i].data[j]);
        }
        
        if (j % 16 != 0) {
            for (int k = 0; k < 16 - (j % 16); k++)
                printf("   ");
            printf("  |");
            for (int k = 16 * (j / 16); k < segs[i].size; k++)
                isprint(segs[i].data[k]) ? putchar(segs[i].data[k]) : putchar('.');
            printf("|");
        }

        printf("\n");
    }
    
}

int
main(int argc, char **argv) {
    if (argc < 2) {
        usage(*argv);
        return 1;
    }

    /* Command line options */
    int verbose = 0;
    int debugsym = 0;
    char *outfn = NULL;
    char *infn = NULL;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            /* Argument */
            switch (argv[i][1]) {
                case 'v': verbose = 1; break;
                case 'g': debugsym = 1; break;
                case 'o': outfn = argv[++i]; break;
            }
        } else {
            if (infn == NULL) infn = argv[i];
            else {
                usage(*argv);
                return 1;
            }
         }
    }

    if (infn == NULL) {
        usage(*argv);
        return 1;
    }

    if (!outfn) {
        outfn = malloc(2);
        strcpy(outfn, "a");
    }

    FILE *verf = NULL;
    if (verbose) verf = stdout;
    else verf = fopen("/dev/null", "w");

    /* Read input file */
    FILE *inf = fopen(infn, "r");
    if (!inf) {
        free(outfn);
        fprintf(stderr, "Error reading file: %s\n", strerror(errno));
        return 1;
    }

    size_t inlen;
    char *input = read_whole_file(inf, &inlen);

    /* Assemble input */
    segment_t *segments = NULL;
    int r = assemble(input, inlen, &segments, verf, stderr);
    if (r < 0) {
        fprintf(stderr, "Error assembling\n");
        return 1;
    }

    /* Verbose */
    if (verbose) {
        print_symbols(segments);
        dump_segments(segments);
    }

    char buff[256];

    strcpy(buff, outfn);
    strcat(buff, ".data");
    FILE *outdf = fopen(buff, "wb");
    fwrite(segments[SEG_DATA].data, segments[SEG_DATA].size, 1, outdf);
    fclose(outdf);

    strcpy(buff, outfn);
    strcat(buff, ".text");
    FILE *outtf = fopen(buff, "wb");
    fwrite(segments[SEG_DATA].data, segments[SEG_DATA].size, 1, outtf);
    fclose(outtf);

    if (debugsym) {
        strcpy(buff, outfn);
        strcat(buff, ".sym");
        FILE *outsf = fopen(buff, "wb");
        write_symbols(segments[SEG_DATA].symbols, outsf);
        write_symbols(segments[SEG_TEXT].symbols, outsf);
        fclose(outsf);
    }


    /* Deinit */
    free(input);
    free(outfn);

    segment_destroy(&segments[SEG_DATA]);
    segment_destroy(&segments[SEG_TEXT]);

    free(segments);

    return 0;
}
