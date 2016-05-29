; Multiplier Routine (8-bit x 8-bit = 16-bit product)
; ==================================================
; Shift and add algorithm
;

        jump main

mult_8x8:
        NAMEREG s0, multiplicand ; preserved
        NAMEREG s1, multiplier ; preserved
        NAMEREG s2, bit_mask ; modified
        NAMEREG s3, result_msb ; most-significant byte (MSB) of result (modified)
        NAMEREG s4, result_lsb ; least-significant byte (LSB) of result (modified)

        LOAD bit_mask, 01 ; start with least-significant bit (lsb)
        LOAD result_msb, 00 ; clear product MSB
        LOAD result_lsb, 00 ; clear product LSB (not required)

; loop through all bits in multiplier
mult_loop:
        TEST multiplier, bit_mask ; check if bit is set
        JUMP Z, no_add ; if bit is not set, skip addition
        ADD result_msb, multiplicand ; addition only occurs in MSB

no_add: AND s0, s0           ; clear CARRY
        SRA result_msb ; shift MSB right, CARRY into bit 7,
                                ; lsb into CARRY
        SRA result_lsb ; shift LSB right,
                                ; lsb from result_msb into bit 7
        SL0 bit_mask ; shift bit_mask left to examine
                                ; next bit in multiplier
        JUMP NZ, mult_loop ; if all bit examined, then bit_mask = 0,
                                ; loop if not 0
        RETURN ; multiplier result now available in
                                ; result_msb and result_lsb

main:   load s0, 39
        load s1, ff
        call mult_8x8
        add result_msb, 01
        sub result_lsb, 0A
