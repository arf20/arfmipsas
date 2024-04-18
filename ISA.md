# MIPS I ISA subset for ETC's CPU

## Formats

### R

```
+---------+-------+-------+-------+-------+---------+
| op      | rs    | rt    | rd    | shamt | func    |
+---------+-------+-------+-------+-------+---------+
```

 - op[6]: Opcode
 - rs[5]: First source register
 - rt[5]: Second source register
 - rd[5]: Destination register
 - shamt[5]: Shift amount
 - func[6]: Function only 3 bits used

### I

```
+---------+-------+-------+-------------------------+
| op      | rs    | rt    | imm                     |
+---------+-------+-------+-------------------------+
```

 - op[6]: Opcode
 - rs[5]: Source register
 - rt[5]: Second source or destination Register
 - imm[16]: Immediate half-word

### I

```
+---------+-----------------------------------------+
| op      | j                                       |
+---------+-----------------------------------------+
```

 - op[6]: Opcode
 - j[26]: Jump address [27-2]

## Instructions

### Arithmetic Logic register to register

| instruction     | operation     | format | op     | func   |
|-----------------|---------------|--------|--------|--------|
| and $a, $b, $c  | $a = $b & $c  | R      | 000000 | 100100 |
| or  $a, $b, $c  | $a = $b \| $c | R      | 000000 | 100101 |
| add $a, $b, $c  | $a = $b + $c  | R      | 000000 | 100000 |
| sub $a, $b, $c  | $a = $b - $c  | R      | 000000 | 100010 |
| slt $a, $b, $c  | $a = $b - $c  | R      | 000000 | 101010 |

 - rs = $b
 - rt = $c
 - rd = $a

### Arithmetic Logic Immediate

| instruction     | operation        | format | op     |
|-----------------|------------------|--------|--------|
| ori $a, $b, val | $a = $b | val    | I      | 001101 |

 - rs = $b
 - rt = $a
 - imm = val

### Memory

| instruction     | operation        | format | op     |
|-----------------|------------------|--------|--------|
| lw  $a, off($b) | $a = M[$b + off] | I      | 100011 |
| sw  $a, off($b) | M[$b + off] = $a | I      | 101011 |

 - imm = off
 - rs = $b
 - rt = $a

### Constant

| instruction     | operation        | format | op     |
|-----------------|------------------|--------|--------|
| lui $a, val     | $a = M[$b + off] | I      | 100011 |

 - imm = val

### Conditional jump

| instruction       | operation        | format | op     |
|-------------------|------------------|--------|--------|
| beq $a, $b, label | PC = label       | I      | 000100 |

 - rs = $a
 - rt = $b
 - imm = (label - (PC + 4)) / 4 (relative jump)

### Inconditional jump

| instruction     | operation        | format | op     |
|-----------------|------------------|--------|--------|
| j label         | PC = label       | J      | 000010 | 

 - j = label[27-2] (absolute jump)


## Glossary

| instruction | status |
|-------------|--------|
| add         | TODO   |
| sub         | TODO   |
| slt         | TODO   |
| and         | TODO   |
| or          | TODO   |
| ori         | TODO   |
| lui         | TODO   |
| lw          | TODO   |
| sw          | TODO   |
| beq         | TODO   |
| j           | TODO   |
