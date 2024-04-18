#include <stdio.h>

void
usage(char *name) {
    printf("Usage: %s [options] file\nOptions\n"
    "  -v\t\tVerbose output.\n  -g\t\tGenerate debug symbols for arfmipssim.\n  -o <file>\tPlace the output into <file>.\n", name);
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
    char *output = NULL;
    char *input = NULL;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            /* Argument */
            switch (argv[i][1]) {
                case 'v': verbose = 1; break;
                case 'g': debugsym = 1; break;
                case 'o': output = argv[++i]; break;
            }
        } else {
            if (input == NULL) input = argv[i];
            else {
                usage(*argv);
                return 1;
            }
         }
    }

    if (input == NULL) {
        usage(*argv);
        return 1;
    }

    printf("v: %d, g: %d, o: %s, i: %s\n", verbose, debugsym, output, input);

    return 0;
}
