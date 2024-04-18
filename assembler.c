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

    assembler.c: Assembler routines

*/

#include <string.h>
#include <ctype.h>

#include "assembler.h"

const char *
strip(const char *str) {
    while (*str == '\t' || *str == ' ') str++;
    return str;
}

size_t
label_len(const char *str) {
    size_t len = 0;
    while (isalnum(*str) || *str == '_') {
        len++;
        str++;
    }
    return len;
}

size_t
instruction_len(const char *str) {
    size_t len = 0;
    while (isalnum(*str)) {
        len++;
        str++;
    }
    return len;
}

int
pass(int passn, const char *input, segment_t **output, FILE *verf, FILE *errf) {
    /* Deserialization vars */
    char *t = NULL;
    size_t line = 1;
    char buff[256];
    size_t len = 0;

    /* State */
    segid_t curr_seg = SEG_TEXT; /* .text by default */

    while (*input) {
        input = strip(input);
        if (*input == '\n') {
            fprintf(verf, "%d: Empty line\n", line);
            input++;
            line++;
        }
        else if (*input == '#' || *input == ';') {
            /* Comment */
            input = strchr(input, '\n') + 1;
            line++;
        }
        else if (*input == '.') {
            /* Macro */
            t = strchr(input, '\n') + 1;
            len = t - input - 1;
            strncpy(buff, input, len);
            buff[len] = '\0';
            fprintf(verf, "%d: Macro: %s\n", line, buff);

            input = t;
            line++;
        }
        else {
            /* Label or instruction or both */
            const char *tok = input;
            size_t ll = label_len(tok);
            input = strip(input + ll);
            if (*input == ':') {
                /* Label */
                strncpy(buff, tok, ll);
                buff[ll] = '\0';
                fprintf(verf, "%d: Label: %s\n", line, buff);
                input = strip(input + 1);
                
                if (*input == '\n') {
                    /* End of line */
                    line++;
                    input++;
                }
                else {
                    /* Instruction after label */
                    t = strchr(input, '\n') + 1;
                    len = t - input - 1;
                    strncpy(buff, input, len);
                    buff[len] = '\0';
                    fprintf(verf, "%d: Instruction: %s\n", line, buff);

                    input = t;
                    line++;
                }
            }
            else {
                /* Instruction */
                t = strchr(input, '\n') + 1;
                len = t - input - 1;
                strncpy(buff, input, len);
                buff[len] = '\0';
                fprintf(verf, "%d: Instruction: %s\n", line, tok);

                input = t;
                line++;
            }
        }
    }
}

int
assemble(const char *input, segment_t **output, FILE *verf, FILE *errf) {
    /* Two passes */
    for (int i = 0; i < 2; i++) {
        int err = pass(i, input, output, verf, errf);
        if (err < 0)
            return err;
    }

    return 0;
}
