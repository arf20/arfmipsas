; comment
        .data
value:  .word 0x12345678, 2  , 3, 1
value2: .word 0x87654321, 0
        .asciiz "asdf"
        .align 2
        .byte 0x69   , 0x70, 12
        .space 5
        .byte 0x42, 10, 9
        .text
entry:  add $t0 , $t1 , $t2 
        ori $s0 , $s1 , 0x1234 
loop:   sw $t0, 0x1234 ( $t1 ) 
        lui $t0, 0x1234
        beq $t0, $t1, entry
        j loop
