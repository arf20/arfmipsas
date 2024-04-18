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

#include "assembler.h"

void
usage(char *name) {
    fprintf(stderr, "Usage: %s [options] file\nOptions\n"
    "  -v\t\tVerbose output.\n  -g\t\tGenerate debug symbols for arfmipssim.\n  -o <file>\tPlace the output into <file>.\n", name);
}

char *
read_whole_file(const char *fn) {
    FILE *f = fopen(fn, "r");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    size_t ins = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buff = malloc(ins);
    fread(buff, ins, 1, f);

    fclose(f);

    return buff;
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

    /* Read input file */
    char *input = read_whole_file(infn);
    if (!input) {
        fprintf(stderr, "Error reading file: %s\n", strerror(errno));
        return 1;
    }

    /* Assemble input */
    segment_t *segments = NULL;
    assemble(input, &segments, stdout, stderr);

    return 0;
}
