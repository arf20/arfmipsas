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

*/

#ifndef _ASSEMBLER_H
#define _ASSEMBLER_H

#include <stdio.h>
#include <stdint.h>

/* Macros */

#define DATA_ORG    0x10010000
#define TEXT_ORG    0x00400000

/* Types */

typedef uint32_t addr_t;

typedef enum { SEG_DATA, SEG_TEXT } segid_t;

typedef struct {
    addr_t address;
    char *label;
} symbol_t;

typedef struct {
    symbol_t *table;
    size_t size;
    size_t capacity;
} symbol_table_t;

typedef struct {
    segid_t id;
    uint8_t *data;
    size_t size;
    size_t capacity;
    symbol_table_t *symbols;
} segment_t;

/* Routines */

int assemble(const char *input, segment_t **output, FILE *verf, FILE *errf);

#endif /* _ASSEMBLER_H */