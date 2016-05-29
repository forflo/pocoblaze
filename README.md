# Pocoblaze
## What is Pocoblaze

Pocoblaze is three things

- A (surprisingly well functioning) educational play project
- A binary compatible emulator for the picoblaze (Xilinx)
  instruction set.
- A legup synthesizable hardware design. That is, a C-Program
  that only uses language primitives that can be transformed
  into an semantically equivalent Verilog design, which in turn
  is fully synthesizable.

Summary: **DON'T USE IN PRODUCTION**

## Features

- Configurable instruction ROM size. In practice, however,
  this size is limited to 256 instructions (see instruction format).
- Configurable stack size
- Configurable amount of registers, though the
  assembler only supports 8 (s0 - s7)

## Current status of development

- I implemented (and tested) the most instructions from the
  picoblaze instruction set (visit pocoblaze/references/reference_manual.pdf).
- Instructions that aren't supportet yet are
  - returni
  - interrupt management instructions
  - fetch
  - store (because the assembler doesn't support the latter two)
- As examble what you **can** do, look at that:
  ```assembly
  ; Multiplier Routine (8-bit x 8-bit = 16-bit product)
  ; ==================================================
  ; Shift and add algorithm
  ;,,,,,,,,,,,,,,,,,,,,,,,,,

  jump main

  mult_8x8:
      NAMEREG s0, multiplicand ; preserved
      NAMEREG s1, multiplier   ; preserved
      NAMEREG s2, bit_mask     ; modified
      NAMEREG s3, result_msb   ; most-significant byte (MSB) of result (modified)
      NAMEREG s4, result_lsb   ; least-significant byte (LSB) of result (modified)

      LOAD bit_mask, 01        ; start with least-significant bit (lsb)
      LOAD result_msb, 00      ; clear product MSB
      LOAD result_lsb, 00      ; clear product LSB (not required)

  ; loop through all bits in multiplier
  mult_loop:
      TEST multiplier, bit_mask    ; check if bit is set
      JUMP Z, no_add               ; if bit is not set, skip addition
      ADD result_msb, multiplicand ; addition only occurs in MSB

  no_add:
      AND s0, s0           ; clear CARRY
      SRA result_msb       ; shift MSB right, CARRY into bit 7,
                           ; lsb into CARRY
      SRA result_lsb       ; shift LSB right,
                           ; lsb from result_msb into bit 7
      SL0 bit_mask         ; shift bit_mask left to examine
                           ; next bit in multiplier
      JUMP NZ, mult_loop   ; if all bit examined, then bit_mask = 0,
                           ; loop if not 0
      RETURN               ; multiplier result now available in
                           ; result_msb and result_lsb

  main:
      load s0, 33
      load s1, ff
      call mult_8x8
      add result_msb, 01
      sub result_lsb, 0A
  ```
  You can find this listing on p. 31 in pocoblaze/references/reference_manual.pdf.
  This version, however, contains a bug! It seems that someone wrote that
  code before they added additional odd parity semantics to the TEST instruction.

  Needless to say, I implemented the proper additional TEST feature.
  My attempt of writing this in C resultet in utter cargo cult programming
  that I'm pretty proud of. What a reason to show off using ugly C Code:

  ```C
  /* WARNING! CARGO CULT PROGRAMMING:
   * http://stackoverflow.com/questions/109023/
   * how-to-count-the-number-of-set-bits-in-a-32-bit-integer */
  uint8_t NumberOfSetBits(uint32_t i){
      // Java: use >>> instead of >>
      // C or C++: use uint32_t
      i = i - ((i >> 1) & 0x55555555);
      i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
      return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
  }

  /* test s7, ff  ; if (s7 AND ff) = 0 then ZERO <- 1,
   *              ;   CARRY <- odd parity of (s7 AND ff) */
  void test_k_to_sx(){
      printf("In function: test_k_to_sx\n");

      if ((register_file[x_register_pointer]
          & constant_argument) == 0) {
          zero_flag = true;
      }

      // parity generator disabled because it seems
      // to misbehave in conjunction with the
      carry_flag = (NumberOfSetBits(
                     (register_file[x_register_pointer]
                       & constant_argument)) % 2 == 0)
          ? 1
          : 0;
  }
  ```

## Instruction format of Pocoblaze

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

## Assembler
In directory pocoblaze/assembler you can find a modified version
of the original Picoblaze assembler. The code is beyond ugly
but it works quite well. I added support for the TEST instruction
as described by the 2004 revision of the picoblaze manual. A
copy of this manual resides in pocoblaze/references.

WARNING: ASM.CPP IS NOT MY SOFTWARE
it's copyrighted by Xilinx!

## How to build

1. Check out this repo
2. Cd into repo/pocoblaze
3. Build using

        $ CFLAGS="-DSIMULATOR -DFANCY_RAM_REPORT" make

4. Cd into repo/assembler
5. Build assembler using

        $ make

In order to generate Verilog using LegUp you have to
download the LegUp Appliance. You can do that
[here](http://legup.eecg.utoronto.ca/getstarted.php).

Simply place repo/pocoplaze inside ../<leguppath>/examples/,
change into the directory and enter

    $ make -f LegupMakefile

for Verilog synthesis and.

    $ make -f LegupMakefile v

for Verilog simulation.
