; comment
        .data
value:  .word 0x12345678
value2: .word 0x87654321
        .asciiz "asdf"
        .align 2
        .byte 0x69
        .space 5
        .byte 0x42
        .text
entry:  insta
        instb
        instc
routine:instd
another:inste
