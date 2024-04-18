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
#include <sys/param.h>

#include "assembler.h"

#define BUFF_SIZE   256

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

const char *
copy_keyword(const char *src, char *dst, size_t sz) {
    char  *ptr = dst;
    while (isalpha(*src) && ptr - dst < sz) {
        *ptr = *src;
        ptr++;
        src++;
    }
    *ptr = '\0';
    return src;
}

int
pass(int passn, const char *input, segment_t **output, FILE *verf, FILE *errf) {
    if (passn == 0)
        fprintf(verf, "=== FIRST PASS ===\n");
    else
        fprintf(verf, "=== SECOND PASS ===\n");

    /* Deserialization vars */
    const char *t = NULL;
    size_t line = 1;
    char buff[BUFF_SIZE];
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
        else {
            /* Label or instruction or both */
            size_t ll = label_len(input);
            if (input[ll] == ':') {
                /* Label */
                strncpy(buff, input, ll);
                buff[ll] = '\0';
                fprintf(verf, "%d: Label: %s\n", line, buff);
                input = strip(input + ll + 1);
                
                if (*input == '\n') {
                    /* End of line */
                    line++;
                    input++;
                }
                /* Else, fall to insturction next iteration */
            }
            else {
                /* Instruction */
                t = strchr(input, '\n') + 1;

                if (*input == '.') {
                    /* Directive */
                    input++; /* skip period */
                    /* Get directive */
                    input = copy_keyword(input, buff, BUFF_SIZE);
                    fprintf(verf, "%d: Directive: %s\n", line, buff);

                    

                } else {
                    /* Instruction */
                    input = copy_keyword(input, buff, BUFF_SIZE);
                    fprintf(verf, "%d: Instruction: %s\n", line, buff);

                }

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
