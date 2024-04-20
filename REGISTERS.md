# Registers

5 bit address - 32 registers

| register | name    | comment |
|----------|---------|---------|
| $0       | $zero   | Zero    |
| $1       | $at     | Reserved for pseudoinstructions |
| $2-3     | $v0-1   | Results |
| $4-7     | $a0-3   | Arguments |
| $8-15    | $t0-7   | Routine temporal |
| $16-23   | $s0-7   | Routine conserved |
| $24-25   | $t8-9   | Routine temporal (ii) |
| $26-27   | $k0-1   | Reserved for kernel |
| $28      | $gp     | Global pointer |
| $29      | $sp     | Stack pointer |
| $30      | $fp     | Frame pointer |
| $31      | $ra     | Return address |
