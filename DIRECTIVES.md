# Directives

## Segments

 - .data
 - .text

## Data
 - .byte    v.. 1 byte
 - .half    v.. 2 byte
 - .word    v.. 4 byte
 - .ascii   "v" Unterminated string
 - .asciiz  "v" NUL-terminated C-string

## Data alignment
 - .align n     Align next data item to (0 byte, 1 half, 2 word)
 - .space n     Reserve specified amount of bytes
