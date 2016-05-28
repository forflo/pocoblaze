# Picoc
## What is picoc

Picoc is three things

- A (surprisingly well functioning) educational play project
- A binary compatible emulator for the picoblaze (Xilinx)
  instruction set.
- A legup synthesizable hardware design. That is, a C-Program
  that only uses language primitives that can be transformed
  into an semantically equivalent Verilog design, which in turn
  is fully synthesizable.

Summary: **DON'T USE IN PRODUCTION**

## Instruction format of picoc

- 16-Bit fixed width.
- two cases
  - monadic form:
```
    [00000 000 00000000]
     (1)   (2) (3)
     ---------------------
     (1) = instruction (5 bits)
     (2) = register address (3 bit: s0 - s7)
     (3) = immediate constant (8 bits)
```
  - dyadic form:
```
    [00000 000 000 00000]
     (1)   (2) (3) (4)
     -----------------
     (1) = instruction (5 bits)
     (2) = first register address (sX)
     (3) = second register address (sY)
     (4) = unused (5 bits)
```

- This closely resembles the
  behaviour of the original picoblaze
  implementation in VHDL
- Note that some monadic instructions will use
  the dyadic form storing the register address
  in (3) and using (2) as switch (an example would be
  the shift and rotate instruction)

## Features

- Configurable instruction ROM size. In practice, however,
  this size is limited to 256 instructions (see instruction format).
- Configurable stack size
- Configurable amount of registers, though the
  assembler only supports 8 (s0 - s7)

## Assembler
In directory picoc/assembler you can find a modified version
of the original Picoblaze assembler. The code is beyond ugly
but it works quite well. I added support for the TEST instruction
as described by the 2004 revision of the picoblaze manual. A
copy of this manual resides in picoc/references.

WARNING: ASM.CPP IS NOT MY SOFTWARE
it's copyrighted by Xilinx!
