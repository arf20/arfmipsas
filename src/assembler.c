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

int
islabelchar(char c) {
    return isalnum(c) || c == '_';
}

size_t
label_len(const char *str) {
    size_t len = 0;
    while (islabelchar(*str)) {
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
symbol_table_lookup(symbol_table_t *st, const char *label) {
    for (int i = 0; i < st->size; i++)
        if (strcmp(st->table[i].label, label) == 0)
            return st->table[i].address;
    return 0; /* on not found */
}

int
count_data_operands(const char *oper) {
    int v, i = 0;
    while (isprint(*oper)) {
        oper = strip(oper);
        oper = get_numeric_operand(oper, &v);
        i++;
        oper = strip(oper);
        if (*oper != ',') break;
        oper++;
        oper = strip(oper);
    }
    return i;
}

addr_t
next_data_addr(const char *dir, const char *oper, addr_t curr_addr, int line,
    FILE *errf)
{
    int p1;

    if (strcmp(dir, "byte") == 0) {
        curr_addr += count_data_operands(oper);
    } else if (strcmp(dir, "half") == 0) {
        curr_addr += 2 * count_data_operands(oper);
    } else if (strcmp(dir, "word") == 0) {
        curr_addr += 4 * count_data_operands(oper);
    } else if (strcmp(dir, "ascii") == 0) {
        if (*oper != '\"') {
            fprintf(errf, "%d: warning: expected string literal\n",
                line);
            return curr_addr;
        }
        oper++; /* skip " */
        while (*oper != '\"') {
            curr_addr++;
            oper++;
        }
    } else if (strcmp(dir, "asciiz") == 0) {
        if (*oper != '\"') {
            fprintf(errf, "%d: warning: expected string literal\n",
                line);
            return curr_addr;
        }
        oper++; /* skip " */
        while (*oper != '\"') {
            curr_addr++;
            oper++;
        }

        curr_addr++; /* NUL terminator */
    } else if (strcmp(dir, "align") == 0) {
        get_numeric_operand(oper, &p1);
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
        get_numeric_operand(oper, &p1);
        curr_addr += p1;
    } else {
        /* Unknown directive */
        fprintf(errf, "%d: warning: unknown data directive %s\n",
            line, dir);
    }

    return curr_addr;
}

const char *
write_data_bytes(const char *oper, int8_t *ptr, FILE *verf) {
    int v, i = 0;
    while (isprint(*oper)) {
        oper = strip(oper);
        oper = get_numeric_operand(oper, &v);
        *ptr = v;
        fprintf(verf, "%d", *ptr);
        ptr++;
        i++;
        /* skip , */
        oper = strip(oper);
        if (*oper != ',') break;
        fprintf(verf, ", ");
        oper++;
        oper = strip(oper);
    }
    return oper;
}

const char *
write_data_halfs(const char *oper, int16_t *ptr, FILE *verf) {
    int v, i = 0;
    while (isprint(*oper)) {
        oper = strip(oper);
        oper = get_numeric_operand(oper, &v);
        *ptr = v;
        fprintf(verf, "%d", *ptr);
        ptr++;
        i++;
        /* skip , */
        oper = strip(oper);
        if (*oper != ',') break;
        fprintf(verf, ", ");
        oper++;
        oper = strip(oper);
    }
    return oper;
}

const char *
write_data_words(const char *oper, int32_t *ptr, FILE *verf) {
    int v, i = 0;
    while (isprint(*oper)) {
        oper = strip(oper);
        oper = get_numeric_operand(oper, &v);
        *ptr = v;
        fprintf(verf, "%d", *ptr);
        ptr++;
        i++;
        /* skip , */
        oper = strip(oper);
        if (*oper != ',') break;
        fprintf(verf, ", ");
        oper++;
        oper = strip(oper);
    }
    return oper;
}

void
write_data(uint8_t *segdata, const char *dir, const char *oper, addr_t addr,
int line, FILE *verf, FILE *errf)
{
    addr -= DATA_ORG;

    if (strcmp(dir, "byte") == 0) {
        write_data_bytes(oper, segdata + addr, verf);
    } else if (strcmp(dir, "half") == 0) {
        write_data_halfs(oper, (int16_t*)&segdata[addr], verf);
    } else if (strcmp(dir, "word") == 0) {
        write_data_words(oper, (int32_t*)&segdata[addr], verf);
    } else if (strcmp(dir, "ascii") == 0) {
        if (*oper != '\"') {
            return;
        }
        oper++; /* skip " */
        fprintf(verf, "\"");
        while (*oper != '\"') {
            segdata[addr] = *oper;
            fprintf(verf, "%c", *oper);
            oper++;
            addr++;
        }
        fprintf(verf, "\"");
    } else if (strcmp(dir, "asciiz") == 0) {
        if (*oper != '\"') {
            return;
        }
        oper++; /* skip " */
        fprintf(verf, "\"");
        while (*oper != '\"') {
            segdata[addr] = *oper;
            fprintf(verf, "%c", *oper);
            oper++;
            addr++;
        }
        fprintf(verf, "\"");

        addr++; /* NUL terminator */
        segdata[addr] = '\0';
    }

    return;
}

word_t
encode_r(uint8_t op, reg_t rs, reg_t rt, reg_t rd, uint8_t shamt, uint8_t func)
{
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
encode_i(uint8_t op, reg_t rs, reg_t rt, int16_t imm) {
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

const char *
skip_operand_separator(const char *oper, int line, FILE *verf, FILE *errf) {
    oper = strip(oper);
    if (*oper != ',') {
        fprintf(errf, "%d: warning: expected ,\n", line);
        return oper;
    }
    oper++;
    fprintf(verf, ", ");
    return strip(oper);
}

const char *
get_register_operand(const char *oper, reg_t *r, int line, FILE *errf) {
    if (*oper != '$') {
        fprintf(errf, "%d: warning: expected register\n", line);
        return oper;
    }
    oper++; /* skip $ */

    char buff[BUFF_SIZE];
    int i = 0;
    while (isalpha(*oper) && i < BUFF_SIZE - 1) {
        buff[i] = *oper;
        i++;
        oper++;
    }
    buff[i] = '\0';

    int unknown = 0;
    if (strlen(buff) == 4 && strcmp(buff, "zero") == 0)
        *r = 0;
    else if (strlen(buff) == 2) { 
        if (strcmp(buff, "at") == 0)
            *r = 1;
        else if (strcmp(buff, "gp") == 0)
            *r = 28;
        else if (strcmp(buff, "sp") == 0)
            *r = 29;
        else if (strcmp(buff, "fp") == 0)
            *r = 30;
        else if (strcmp(buff, "ra") == 0)
            *r = 31;
        else unknown = 1;
    } else if (strlen(buff) == 1) {
        int n = strtol(oper, NULL, 10);
        if (buff[0] == 'v' && n >= 0 && n <= 1)
            *r = 2 + n;
        else if (buff[0] == 'a' && n >= 0 && n <= 3)
            *r = 4 + n;
        else if (buff[0] == 't' && n >= 0 && n <= 9) {
            if (n < 8) *r = 8 + n;
            else *r = 24 + n - 8;
        }
        else if (buff[0] == 's' && n >= 0 && n <= 7)
            *r = 16 + n;
        else if (buff[0] == 'k' && n >= 0 && n <= 1)
            *r = 26 + n;
        else unknown = 1;
    } else unknown = 1;

    if (unknown) {
        fprintf(errf, "%d: warning: unknown register\n", line);
        *r = 0;
    }
    return oper + 1; /* all register pseudo numer < 10 */
}

const char *
parse_reg_operands(const char *oper, int n, reg_t *regs, int line, FILE *verf,
    FILE *errf)
{
    /* max 3 regs */
    for (int i = 0; i < n; i++) {
        oper = strip(oper);
        oper = get_register_operand(oper, regs, line, errf);
        fprintf(verf, "$%d", *regs);
        oper = strip(oper);
        if (i < n - 1) {
            if (*oper != ',')
                fprintf(errf, "%d: warning: expected ,\n", line);
            oper = skip_operand_separator(oper, line, verf, errf);
            regs++;
        }
    }
    return oper;
}

const char *
parse_immediate_operand(const char *oper, uint16_t *imm, int line, FILE *verf,
    FILE *errf)
{
    int t;
    oper = get_numeric_operand(oper, &t);
    *imm = t;
    fprintf(verf, "%d", *imm);
    return oper;
}

const char *
parse_base_displacement_operand(const char *oper, uint16_t *imm, reg_t *base,
    int line, FILE *verf, FILE *errf)
{
    /* get displacement */
    int dis;
    oper = get_numeric_operand(oper, &dis);
    *imm = dis;

    oper = strip(oper);
    
    if (*oper != '(') {
        fprintf(errf, "%d: warning: expected (\n", line);
        return oper;
    }
    oper++; /* skip ( */
    oper = strip(oper);

    /* get base register */
    oper = get_register_operand(oper, base, line, errf);

    oper = strip(oper);
    if (*oper != ')')
        fprintf(errf, "%d: warning: expected )\n", line);
    oper++;
    oper = strip(oper);

    fprintf(verf, "%d($%d)", *imm, *base);
    return oper; /* at \n presumably */
}

const char *
parse_label_operand(const char *oper, symbol_table_t *st, addr_t *addr,
    int line, FILE *verf, FILE *errf)
{
    char buff[BUFF_SIZE];
    int i = 0;
    while (islabelchar(*oper) && i < BUFF_SIZE) {
        buff[i] = *oper;
        i++;
        oper++;
    }
    buff[i] = '\0';
    *addr = symbol_table_lookup(st, buff);

    fprintf(verf, "0x%.8x", *addr);
    return strip(oper);
}

int16_t
calculate_relative_jump(addr_t from, addr_t to) {
    /* 0 relative is from + 4 */
    return (to - from - 4) / 4;
}

void
encode_instruction(segment_t *segs, addr_t addr, const char *ins,
    const char *oper,  int line, FILE *verf, FILE *errf)
{

    uint8_t *segdata = segs[SEG_TEXT].data;

    addr -= TEXT_ORG;
    reg_t regs[3]; /* register operands */
    uint16_t imm; /* immediate data */
    addr_t label_addr; /* jump addr */

    /* ALU instructions, R format
        fields: $a, $b, $c => rd, rs, rt */
    if (strcmp(ins, "and") == 0) {
        parse_reg_operands(oper, 3, regs, line, verf, errf);
        *(word_t*)&segdata[addr] = encode_r(0, regs[1], regs[2], regs[0], 0,
            0b100100);
    } else if (strcmp(ins, "or") == 0) {
        parse_reg_operands(oper, 3, regs, line, verf, errf);
        *(word_t*)&segdata[addr] = encode_r(0, regs[1], regs[2], regs[0], 0,
            0b100101);
    } else if (strcmp(ins, "add") == 0) {
        parse_reg_operands(oper, 3, regs, line, verf, errf);
        *(word_t*)&segdata[addr] = encode_r(0, regs[1], regs[2], regs[0], 0,
            0b100000);
    } else if (strcmp(ins, "sub") == 0) {
        parse_reg_operands(oper, 3, regs, line, verf, errf);
        *(word_t*)&segdata[addr] = encode_r(0, regs[1], regs[2], regs[0], 0,
            0b100010);
    } else if (strcmp(ins, "slt") == 0) {
        parse_reg_operands(oper, 3, regs, line, verf, errf);
        *(word_t*)&segdata[addr] = encode_r(0, regs[1], regs[2], regs[0], 0,
            0b101010);
    }
    /* ALU immediate instructions, I format
        fields: $a, $b, imm */
    else if (strcmp(ins, "ori") == 0) {
        oper = parse_reg_operands(oper, 2, regs, line, verf, errf);
        oper = skip_operand_separator(oper, line, verf, errf);
        oper = parse_immediate_operand(oper, &imm, line, verf, errf);
        *(word_t*)&segdata[addr] = encode_i(0b001101, regs[1], regs[0], imm);
    }
    /* Memory instructions, I format */
    else if (strcmp(ins, "lw") == 0) {
        /* $a, off($b) => rt, imm(rs) */
        oper = parse_reg_operands(oper, 1, regs, line, verf, errf);
        oper = skip_operand_separator(oper, line, verf, errf);
        oper = parse_base_displacement_operand(oper, &imm, regs + 1, line,
            verf, errf);
        *(word_t*)&segdata[addr] = encode_i(0b100011, regs[0], regs[1], imm);
    }
    else if (strcmp(ins, "sw") == 0) {
        /* $a, off($b) => rs, imm(rt) */
        oper = parse_reg_operands(oper, 1, regs, line, verf, errf);
        oper = skip_operand_separator(oper, line, verf, errf);
        oper = parse_base_displacement_operand(oper, &imm, regs + 1, line,
            verf, errf);
        *(word_t*)&segdata[addr] = encode_i(0b101011, regs[1], regs[0], imm);
    }
    /* Immediate constant 
        $a, val => rt, val */
    else if (strcmp(ins, "lui") == 0) {
        oper = parse_reg_operands(oper, 1, regs, line, verf, errf);
        oper = skip_operand_separator(oper, line, verf, errf);
        oper = parse_immediate_operand(oper, &imm, line, verf, errf);
        *(word_t*)&segdata[addr] = encode_i(0b000100, 0, regs[0], imm);
    }
    /* Conditional jump
        $a, $b, label => rs, rt, (label) */
    else if (strcmp(ins, "beq") == 0) {
        oper = parse_reg_operands(oper, 2, regs, line, verf, errf);
        oper = skip_operand_separator(oper, line, verf, errf);
        oper = parse_label_operand(oper, segs[SEG_TEXT].symbols, &label_addr,
            line, verf, errf);
        *(word_t*)&segdata[addr] = encode_i(0b000100, regs[0], regs[1],
            calculate_relative_jump(addr + TEXT_ORG, label_addr));
    }
    /* Unconditional jump 
        label => addr */
    else if (strcmp(ins, "j") == 0) {
        oper = parse_label_operand(oper, segs[SEG_TEXT].symbols, &label_addr,
            line, verf, errf);
        *(word_t*)&segdata[addr] = encode_j(0b000010, label_addr);
    }
    else {
        fprintf(errf, "%d:  ^^ warning: unknown instruction\n", line);
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
                    fprintf(verf, "%d:  -> label %s: 0x%.8x\n", line, label,
                        sym.address);
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

                if (t == (const char*)0x1) /* EOF */
                    break;
                
                if (*input == '.') {
                    /* Directive */
                    input++; /* skip period */
                    /* Get directive */
                    input = copy_keyword(input, buff, BUFF_SIZE);
                    input = strip(input);
                    fprintf(verf, "%d: directive: .%s ", line, buff);

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
                            fprintf(errf, "%d: warning: data directive in text "
                                "segment\n", line, buff);
                        }
                    }

                    fprintf(verf, "\n");
                } else {
                    /* Instruction */
                    input = copy_keyword(input, buff, BUFF_SIZE);
                    fprintf(verf, "%d: instruction: %s ", line, buff);
                    input = strip(input);
                    
                    if (passn == 0) {
                        if (curr_seg != SEG_TEXT) 
                            fprintf(errf, "%d: warning: instruction outside "
                                "text segment\n", line);
                        else
                            /* MIPS instructions are 4 bytes */
                            curr_addr[SEG_TEXT] += 4;  
                    } else {
                        encode_instruction(segs,
                            curr_addr[SEG_TEXT], buff, input, line, verf, errf);
                        if (curr_seg == SEG_TEXT) curr_addr[SEG_TEXT] += 4;;
                    }
                
                    fprintf(verf, "\n");
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
