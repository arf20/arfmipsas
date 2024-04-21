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
entry:  add $t0 , $t1 , $t2 
        ori $s0 , $s1 , 0x1234 
        sw $t0, 0x1234 ( $t1 ) 
        lui $t0, 0x1234
