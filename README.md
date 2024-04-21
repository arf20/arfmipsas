# arfmipsas

Assembler for UM (Computer Science) ETC base MIPS-based RISC CPU

## ISA

This basic CPU implements the following [11] instructions:

and, or, add, sub, slt, ori, lw, sw, lui, beq, j

## Build

As any CMake project. No dependencies.

```
cd build
cmake ..
make
```

## Run

```
Usage: ./arfmipsas [options] file
Options
  -v            Verbose output.
  -g            Generate debug symbols for arfmipssim.
  -o <file>     Place the output into <file>.
```

Example

```
./arfmipsas ../tests/test.asm
```

## Output


