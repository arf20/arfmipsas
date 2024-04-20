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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>

#include "assembler.h"

/* Tunables */
#define BUFF_SIZE   256
#define SYMBOL_TABLE_INIT_SIZE  16  /* symbols */
#define SEGMENT_INIT_SIZE       256 /* bytes */

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

const char *
get_numeric_operand(const char *str, int *p) {
    char buff[BUFF_SIZE];
    int i = 0;
    int v = 0;
    while (isxdigit(*str) || *str == 'x' || *str == 'b') {
        if (i >= BUFF_SIZE) break;
        buff[i] = *str;
        str++;
        i++;
    }
    buff[i] = '\0';
    if (buff[1] == 'b') {
        /* hex */
        *p = strtol(buff + 2, NULL, 2);
    } else {
        /* hex (0x), oct (0) or dec */
        *p = strtol(buff, NULL, 0);
    }
    return str;
}

/* Symbol table helpers */
symbol_table_t *
symbol_table_new() {
    symbol_table_t *st = malloc(sizeof(symbol_table_t));
    st->table = malloc(SYMBOL_TABLE_INIT_SIZE * sizeof(symbol_t));
    st->size = 0;
    st->capacity = SYMBOL_TABLE_INIT_SIZE;
    return st;
}

void
symbol_table_push(symbol_table_t *st, symbol_t sym) {
    /* Grow table by double */
    if (st->size == st->capacity) {
        st->table = realloc(st->table, 2*st->capacity);
        st->capacity *= 2;
    }
    /* Insert at end */
    st->table[st->size++] = sym;
}

addr_t
next_data_addr(const char *dir, const char *p, addr_t curr_addr, int line,
    FILE *errf)
{
    int p1;

    if (strcmp(dir, "byte") == 0) {
        curr_addr += 1;
    } else if (strcmp(dir, "half") == 0) {
        curr_addr += 2;
    } else if (strcmp(dir, "word") == 0) {
        curr_addr += 4;
    } else if (strcmp(dir, "ascii") == 0) {
        if (*p != '\"') {
            fprintf(errf, "%d: warning: expected string literal\n",
                line);
            return curr_addr;
        }
        p++; /* skip " */
        while (*p != '\"') {
            curr_addr++;
            p++;
        }
    } else if (strcmp(dir, "asciiz") == 0) {
        if (*p != '\"') {
            fprintf(errf, "%d: warning: expected string literal\n",
                line);
            return curr_addr;
        }
        p++; /* skip " */
        while (*p != '\"') {
            curr_addr++;
            p++;
        }

        curr_addr++; /* NUL terminator */
    } else if (strcmp(dir, "align") == 0) {
        get_numeric_operand(p, &p1);
        switch (p1) {
            case 1: {
                if (curr_addr % 2)
                    curr_addr++;
            } break;
            case 2: {
                int r = curr_addr % 4;
                if (r)
                    curr_addr += 4 - r;
            } break;
            default: {
                fprintf(errf, "%d: warning: unknown alignment\n",
                    line);
            }
        }
    } else if (strcmp(dir, "space") == 0) {
        get_numeric_operand(p, &p1);
        curr_addr += p1;
    } else {
        /* Unknown directive */
        fprintf(errf, "%d: warning: unknown data directive %s\n",
            line, dir);
    }

    return curr_addr;
}

void
write_data(uint8_t *segdata, const char *dir, const char *p, addr_t addr,
int line, FILE *verf, FILE *errf)
{
    addr -= DATA_ORG;
    int p1;
    get_numeric_operand(p, &p1);

    if (strcmp(dir, "byte") == 0) {
        segdata[addr] = p1;
    } else if (strcmp(dir, "half") == 0) {
        *(int16_t*)&segdata[addr] = p1;
    } else if (strcmp(dir, "word") == 0) {
        *(int32_t*)&segdata[addr] = p1;
    } else if (strcmp(dir, "ascii") == 0) {
        if (*p != '\"') {
            return;
        }
        p++; /* skip " "*/
        while (*p != '\"') {
            segdata[addr] = *p;
            p++;
            addr++;
        }
    } else if (strcmp(dir, "asciiz") == 0) {
        if (*p != '\"') {
            return;
        }
        p++; /* skip " "*/
        while (*p != '\"') {
            segdata[addr] = *p;
            p++;
            addr++;
        }

        addr++; /* NUL terminator */
        segdata[addr] = '\0';
    }

    return;
}

word_t
encode_r(uint8_t op, reg_t rs, reg_t rt, reg_t rd, uint8_t shamt, uint8_t func) {
    word_t i = 0;
    i |= (op &    0b111111)   << 26;
    i |= (rs &    0b11111)    << 21;
    i |= (rt &    0b11111)    << 16;
    i |= (rd &    0b11111)    << 11;
    i |= (shamt & 0b11111)    << 6;
    i |= (func &  0b111111)   << 0;
    return i;
}

word_t
encode_i(uint8_t op, reg_t rs, reg_t rt, uint16_t imm) {
    word_t i = 0;
    i |= (op &    0b111111)   << 26;
    i |= (rs &    0b11111)    << 21;
    i |= (rt &    0b11111)    << 16;
    i |= (imm &   0xffff)     << 0;
    return i;
}

word_t
encode_j(uint8_t op, addr_t addr) {
    word_t i = 0;
    i |= (op &    0b111111)   << 26;
    i |= (addr & 0b00001111111111111111111111111100) >> 2; /* j = addr[27-2] */
    return i;
}

reg_t
get_register_operand(const char *oper, int line, FILE *errf) {
    if (*oper != '$') {
        fprintf(errf, "%d: warning: expected register\n", line);
        return 0;
    }
    oper++; /* skip $ */

    char buff[256];
    int i = 0;
    while (isalpha(*oper)) {
        buff[i] = *oper;
        i++;
        oper++;
    }
    buff[i] = '\0';

    if (strlen(buff) == 4 && strcmp(buff, "zero") == 0)
        return 0;
    else if (strlen(buff) == 2) { 
        if (strcmp(buff, "at") == 0)
            return 1;
        else if (strcmp(buff, "gp") == 0)
            return 28;
        else if (strcmp(buff, "sp") == 0)
            return 29;
        else if (strcmp(buff, "fp") == 0)
            return 30;
        else if (strcmp(buff, "ra") == 0)
            return 31;
    } else if (strlen(buff) == 1) {
        int n = strtol(oper, NULL, 10);
        if (buff[0] == 'v' && n >= 0 && n <= 1)
            return 2 + n;
        else if (buff[0] == 'a' && n >= 0 && n <= 3)
            return 4 + n;
        else if (buff[0] == 't' && n >= 0 && n <= 9)
            if (n < 8) return 8 + n;
            else return 24 + n - 8;
        else if (buff[0] == 's' && n >= 0 && n <= 7)
            return 16 + n;
        else if (buff[0] == 'k' && n >= 0 && n <= 1)
            return 26 + n;
    }

    fprintf(errf, "%d: warning: unknown register\n", line);
        return 0;
}

void
encode_instruction(uint8_t *segdata, addr_t addr, const char *ins,
    const char *oper,  int line, FILE *verf, FILE *errf)
{
    addr -= TEXT_ORG;
    if (strcmp(ins, "add") == 0) {

        /*segdata[addr] = encode_r(0, )*/
    }
}


int
pass(int passn, const char *input, segment_t *segs, FILE *verf, FILE *errf) {
    /* Deserialization vars */
    const char *t = NULL;
    size_t line = 1;
    char buff[BUFF_SIZE];
    size_t len = 0;

    /* First pass state */
    segid_t curr_seg = SEG_TEXT; /* .text by default */
    addr_t curr_addr[2] = { DATA_ORG, TEXT_ORG };
    
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
                char *label = strndup(input, ll);
                input = strip(input + ll + 1);

                if (passn == 0) {
                    /* Symbol calculation first pass only */
                    symbol_t sym;
                    sym.label = label;
                    sym.address = curr_addr[curr_seg];
                    symbol_table_push(segs[curr_seg].symbols, sym);
                    fprintf(verf, "%d: label %s: 0x%.8x\n", line, label, sym.address);
                }

                if (*input == '\n') {
                    /* End of line */
                    line++;
                    input++;
                }
                /* Else, fall to instruction on next iteration */
            }
            else {
                /* Instruction */
                t = strchr(input, '\n') + 1;

                if (*input == '.') {
                    /* Directive */
                    input++; /* skip period */
                    /* Get directive */
                    input = copy_keyword(input, buff, BUFF_SIZE);
                    input = strip(input);
                    fprintf(verf, "%d: directive: .%s\n", line, buff);

                    /* Segment directives */
                    if (strcmp(buff, "data") == 0) {
                        curr_seg = SEG_DATA;
                    } else if (strcmp(buff, "text") == 0) {
                        curr_seg = SEG_TEXT;
                    } else {
                        /* Data directives */
                        if (curr_seg == SEG_DATA) {
                            if (passn == 1)
                                write_data(segs[0].data, buff, input,
                                    curr_addr[SEG_DATA], line, verf, errf);
                            
                            curr_addr[SEG_DATA] = next_data_addr(buff, input,
                                curr_addr[SEG_DATA], line, errf);
                        }
                        else {
                            fprintf(errf, "%d: warning: data directive in text segment\n",
                                    line, buff);
                        }
                    }
                } else {
                    /* Instruction */
                    input = copy_keyword(input, buff, BUFF_SIZE);
                    fprintf(verf, "%d: instruction: %s\n", line, buff);
                    
                    if (passn == 0) {
                        if (curr_seg != SEG_TEXT) 
                            fprintf(errf, "%d: warning: instruction outside text segment\n", line);
                        else
                            curr_addr[SEG_TEXT] += 4;   /* MIPS instructions are 4 bytes */
                    } else {
                        encode_instruction(curr_addr[SEG_TEXT], buff, input, 
                            line, verf, errf);
                        if (curr_seg == SEG_TEXT) curr_addr[SEG_TEXT] += 4;;
                    }
                
                }

                input = t;
                line++;
            }
        }
    }

    if (passn == 0) {
        /* Calculate segment size and allocate segments at first pass */
        segs[0].size = curr_addr[0] - DATA_ORG;
        segs[0].data = malloc(segs[0].size);
        memset(segs[0].data, 0, segs[0].size);
        segs[1].size = curr_addr[1] - TEXT_ORG;
        segs[1].data = malloc(segs[1].size);
        memset(segs[1].data, 0, segs[1].size);
    }

    return 0;
}

int
assemble(const char *input, segment_t **output, FILE *verf, FILE *errf) {
    /* Init segments */
    segment_t *segs = malloc(2 * sizeof(segment_t));
    for (segid_t i = SEG_DATA; i < SEG_TEXT + 1; i++) {
        segs[i].id = i;
        segs[i].data = malloc(SEGMENT_INIT_SIZE);
        segs[i].size = 0;
        segs[i].capacity = SEGMENT_INIT_SIZE;
        segs[i].symbols = symbol_table_new();
    }

    /* Two passes */
    for (int i = 0; i < 2; i++) {
        if (i == 0)
            fprintf(verf, "=== FIRST PASS ===\n");
        else
            fprintf(verf, "=== SECOND PASS ===\n");
        
        int err = pass(i, input, segs, verf, errf);
        if (err < 0) {
            return err;
        }
            
        fprintf(verf, "\n");
    }

    *output = segs;

    return 0;
}
