#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "pocoblaze.h"
/* Instruction format of picoc:
 *
 * - 16-Bit fixed width
 * - two cases
 *   - monadic form:
 *     [00000 000 00000000]
 *      (1)   (2) (3)
 *      ---------------------
 *      (1) = instruction (5 bits)
 *      (2) = register address (3 bit: s0 - s7)
 *      (3) = immediate constant (8 bits)
 *
 *   - dyadic form:
 *     [00000 000 000 00000]
 *      (1)   (2) (3) (4)
 *      -----------------
 *      (1) = instruction (5 bits)
 *      (2) = first register address (sX)
 *      (3) = second register address (sY)
 *      (4) = unused (5 bits)
 *
 * - This closely resembles the
 *   behaviour of the original picoblaze
 *   implementation in VHDL
 * - Note that some monadic instructions will use
 *   the dyadic form storing the register address
 *   in (3) and using (2) as switch (an example would be
 *   the shift and rotate instruction) */

/* Compile with -DSIMULATOR to enable stats */
// Configuration variables for the softcore
#define REG_FILE   8
#define RAM_FILE   64
#define STACK      16
#define INS_ROM    256

#define INS_MASK 0xf800
#define SX_MASK 0x0700
#define SY_MASK 0x00E0

#define SIMULATOR

// Setup of the processors register comonents
uint16_t instruction_rom[INS_ROM] = {
0xD005, 0x00BE, 0x01EF, 0x0202, 0x9000, 0xD801, 0xF800, 0xF901, 0xE040, 0x2201,
0xE040, 0x2201, 0xE040, 0x9F40, 0x3203, 0x9E40, 0xBD00, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

uint8_t register_file[REG_FILE] = { 0 };
uint8_t ram_file[RAM_FILE] = { 0 };
uint8_t stack[STACK] = { 0 };

uint16_t program_counter = 0;
uint8_t stack_pointer   = 0;

uint8_t constant_argument  = 0;
uint8_t x_register_pointer = 0; // three bits
uint8_t y_register_pointer = 0; // three bits

bool carry_flag = false;

// unused
uint8_t output_port = 0;
uint8_t input_port  = 0;

// unused
bool zero_flag    = false;
bool read_strobe  = false;
bool write_strobe = false;

// unused
bool interrupt_enable = false;
bool active_interrupt = false;


/* functions for internal use */
uint8_t stack_pop() {
#ifdef DEBUG
    printf("stack_pop\n");
#endif
    return stack[--stack_pointer];
}

void stack_push(const uint16_t instruction_pointer) {
#ifdef DEBUG
    printf("stack_push\n");
#endif
    stack[stack_pointer++] = instruction_pointer;
}

void dispatch_instruction(const uint16_t instruction) {
#ifdef DEBUG
    printf("dispatch_instruction | zero: %i | carry: %i |sp : %i\n",
           zero_flag, carry_flag,  stack_pointer);
#endif

    x_register_pointer = (instruction & SX_MASK) >> 8;
    y_register_pointer = (instruction & SY_MASK) >> 5;
    constant_argument = instruction & 0x00FF;

#ifdef DEBUG
    printf("x_register_pointer: %i\n", x_register_pointer);
    printf("y_register_pointer: %i\n", y_register_pointer);
    printf("constant_argument: %i\n", constant_argument);
#endif

    #define NOT_ZERO 5
    #define ZERO 4
    #define CARRY 6
    #define NOT_CARRY 7

    switch (instruction & INS_MASK) {
       case 0xD000:
           switch (x_register_pointer){
               case NOT_ZERO: jump_nz(); break;
               case ZERO: jump_z(); break;
               case CARRY: jump_c(); break;
               case NOT_CARRY: jump_nc(); break;
               default: jump(); break;
           }
           break;
       case 0xD800:
           switch (x_register_pointer){
               case NOT_ZERO: call_nz(); break;
               case ZERO: call_z(); break;
               case CARRY: call_c(); break;
               case NOT_CARRY: call_nc(); break;
               default: call();
           }
           break;
       case 0x9000: ret(); break;
       case 0x0000: load_k_to_x(); break;
       case 0x4000: load_y_to_x(); break;
       case 0x8000: and_k_to_x(); break;
       case 0xA800: test_k_to_sx(); break;
       case 0xB800: fetch_reg_from_k(); break;
       case 0x9800: fetch_reg_from_sY(); break;
       case 0xF800: store_sX_at_k(); break;
       case 0xE000: store_sX_at_sY(); break;
       case 0xE800: test_sy_to_sx(); break;
       case 0x1000: or_k_to_x(); break;
       case 0x5000: or_y_to_x(); break;
       case 0x1800: xor_k_to_x(); break;
       case 0x5800: xor_y_to_x(); break;
       case 0x2000: add_k_to_x(); break;
       case 0x6000: add_y_to_x(); break;
       case 0x2800: addcy_k_to_x(); break;
       case 0x6800: addcy_y_to_x(); break;
       case 0x3000: sub_k_to_x(); break;
       case 0x7000: sub_y_to_x(); break;
       case 0x3800: subcy_k_to_x(); break;
       case 0x7800: subcy_y_to_x(); break;
       case 0xA000: shift_rotate(instruction); break;
       case 0x8100: input_p_to_x(); break;
       case 0xC000: input_y_to_x(); break;
       case 0x8800: output_p_to_x(); break;
       case 0xC800: output_y_to_x(); break;
       case 0xF000: interrupt(); break;
       case 0xB000: returni(); break;
       case 0x0100: zero(); break;
       case 0x8200: not_zero(); break;
       case 0x1300: carry(); break;
       case 0x1400: not_carry(); break;
    }

#ifdef DEBUG
    int i;
    for(i=0; i<REG_FILE; i++)
        printf("  Cntent in register %i: %02x\n", i, register_file[i]);
    printf("next instruciton!\n\n");
#endif

    program_counter++;
}

/* Funktions for opcodes */
// working!
void load_k_to_x() {
#ifdef DEBUG
    printf("load_k_to_x\n");
#endif
    register_file[x_register_pointer] = constant_argument;
}

// working!
void load_y_to_x() {
#ifdef DEBUG
    printf("load_y_to_x\n");
#endif
    register_file[x_register_pointer] = register_file[y_register_pointer];
}

void fetch_reg_from_sY(){
#ifdef DEBUG
    printf("fetch\n");
#endif

    register_file[x_register_pointer] =
        ram_file[register_file[y_register_pointer]];
}

void fetch_reg_from_k(){
#ifdef DEBUG
    printf("fetch\n");
#endif

    register_file[x_register_pointer] =
        ram_file[constant_argument];
}

void store_sX_at_sY(){
#ifdef DEBUG
    printf("store\n");
#endif

    ram_file[register_file[y_register_pointer]]
        = register_file[x_register_pointer];
}

void store_sX_at_k(){
#ifdef DEBUG
    printf("store\n");
#endif

    ram_file[constant_argument] =
        register_file[x_register_pointer];
}

// working!
void jump_z(){
#ifdef DEBUG
    printf("jump_z\n");
#endif

    if (zero_flag)
        jump();
}

// working!
void jump_nz(){
#ifdef DEBUG
    printf("jump_nz\n");
#endif

    if (! zero_flag)
        jump();
}

void jump_c() {
#ifdef DEBUG
    printf("jump_c\n");
#endif
    if (carry_flag)
        jump();
}

void jump_nc() {
#ifdef DEBUG
    printf("jump_nc\n");
#endif
    if (!carry_flag)
        jump();
}

void call_c() {
#ifdef DEBUG
    printf("call_c\n");
#endif
    if (carry_flag)
        call();
}

void call_nc() {
#ifdef DEBUG
    printf("call_nc\n");
#endif
    if (!carry_flag)
        call();
}

void call_z() {
#ifdef DEBUG
    printf("call_z\n");
#endif
    if (zero_flag)
        call();
}

void call_nz() {
#ifdef DEBUG
    printf("call_nz\n");
#endif
    if (!zero_flag)
        call();
}

// working!
void call() {
#ifdef DEBUG
    printf("call | pushed counter : %i\n", program_counter);
#endif
    stack_push(program_counter);
    // subtract one because dispatch_instruction
    // always increments PC
    program_counter = constant_argument - 1;
}

// working!
void ret() {
    program_counter = stack_pop();
#ifdef DEBUG
    printf("ret | poped counter : %i\n", program_counter);
#endif
}

// working!
void jump() {
#ifdef DEBUG
    printf("jump\n");
#endif
    // subtract one because dispatch_instruction
    // always increments PC
    program_counter = constant_argument - 1;
}

// working
void and_k_to_x() {
#ifdef DEBUG
    printf("and_k_to_x\n");
#endif
    register_file[x_register_pointer] &= constant_argument;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working
void and_y_to_x() {
#ifdef DEBUG
    printf("and_y_to_x\n");
#endif
    register_file[x_register_pointer] &= register_file[y_register_pointer];

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working
void or_k_to_x() {
#ifdef DEBUG
    printf("or_k_to_x\n");
#endif
    register_file[x_register_pointer] |= constant_argument;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working
void or_y_to_x() {
#ifdef DEBUG
    printf("or_y_to_x\n");
#endif
    register_file[x_register_pointer] |= register_file[y_register_pointer];

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working!
void xor_k_to_x() {
#ifdef DEBUG
    printf("xor_k_to_x\n");
#endif
    register_file[x_register_pointer] ^= constant_argument;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working
void xor_y_to_x() {
#ifdef DEBUG
    printf("xor_y_to_x\n");
#endif
    register_file[x_register_pointer] ^= register_file[y_register_pointer];

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working!
void add_k_to_x() {
#ifdef DEBUG
    printf("add_k_to_x\n");
#endif
    if ((uint16_t) register_file[x_register_pointer]
        + (uint16_t) constant_argument > 255 )
        carry_flag = true;
    else
        carry_flag = false;

    register_file[x_register_pointer] += constant_argument;


    if (register_file[x_register_pointer] == 0)
        zero_flag = true;
    else
        zero_flag = false;
}

// working!
void add_y_to_x() {
#ifdef DEBUG
    printf("add_y_to_x\n");
#endif
    if ((uint16_t) register_file[x_register_pointer]
        + (uint16_t) register_file[y_register_pointer] > 255 )
        carry_flag = true;
    else
        carry_flag = false;

    register_file[x_register_pointer] += register_file[y_register_pointer];


    if (register_file[x_register_pointer] == 0)
        zero_flag = true;
    else
        zero_flag = false;
}

// working!
void addcy_k_to_x() {
#ifdef DEBUG
    printf("addcy_k_to_x\n");
#endif
    uint16_t temp = register_file[x_register_pointer];

    register_file[x_register_pointer] +=
        constant_argument + (carry_flag == true ? 1 : 0);

    if ((uint16_t) temp
        + (uint16_t) constant_argument
        + (uint16_t) (carry_flag == true ? 1 : 0) > 255 )
        carry_flag = true;
    else
        carry_flag = false;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;
    else
        zero_flag = false;
}

// working!
void addcy_y_to_x() {
#ifdef DEBUG
    printf("addcy_y_to_x\n");
#endif
    uint16_t temp = register_file[x_register_pointer];

    register_file[x_register_pointer] +=
        register_file[y_register_pointer] + (carry_flag == true ? 1 : 0);

    if ((uint16_t) temp
        + (uint16_t) register_file[y_register_pointer]
        + (uint16_t) (carry_flag == true ? 1 : 0) > 255 )
        carry_flag = true;
    else
        carry_flag = false;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;
    else
        zero_flag = false;
}

// working!
void sub_k_to_x() {
#ifdef DEBUG
    printf("sub_k_to_x\n");
#endif
    if ((int16_t) register_file[x_register_pointer]
        - (int16_t) constant_argument < 0)
        carry_flag = true;
    else
        carry_flag = false;

    register_file[x_register_pointer] -= constant_argument;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;
    else
        zero_flag = false;
}

// working!
void sub_y_to_x() {
#ifdef DEBUG
    printf("sub_y_to_x\n");
#endif
    if ((int16_t) register_file[x_register_pointer]
        - (int16_t) register_file[y_register_pointer] < 0)
        carry_flag = true;
    else
        carry_flag = false;

    register_file[x_register_pointer] -= register_file[y_register_pointer];

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;
    else
        zero_flag = false;
}

// working!
void subcy_k_to_x() {
#ifdef DEBUG
    printf("subcy_k_to_x\n");
#endif
    int16_t temp = register_file[x_register_pointer];

    register_file[x_register_pointer] -= constant_argument;
    register_file[x_register_pointer] -= (carry_flag == true ? 1 : 0);

#ifdef DEBUG
    printf("temp: %hu const: %hhu carry: %i\n", temp, constant_argument, carry_flag);
#endif

    if ((int16_t) temp
        - (int16_t) constant_argument
        - (int16_t) (carry_flag == true ? 1 : 0) < 0)
        carry_flag = true;
    else
        carry_flag = false;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;
    else
        zero_flag = false;
}

// working?
void subcy_y_to_x() {
#ifdef DEBUG
    printf("subcy_y_to_x\n");
#endif
    int16_t temp = register_file[x_register_pointer];

    register_file[x_register_pointer] -= register_file[y_register_pointer];
    register_file[x_register_pointer] -= (carry_flag == true ? 1 : 0);

    if ((int16_t) temp
        - (int16_t) constant_argument
        - (int16_t) (carry_flag == true ? 1 : 0) < 0)
        carry_flag = true;
    else
        carry_flag = false;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;
    else
        zero_flag = false;
}

//Bit pattern for shift and rotate
#define RL_MASK 0x04
#define RR_MASK 0x0C

#define SL0_MASK 0x06
#define SL1_MASK 0x07

#define SLA_MASK 0x00
#define SLX_MASK 0x02

#define SR0_MASK 0x0E
#define SR1_MASK 0x0F

#define SRA_MASK 0x08
#define SRX_MASK 0x0A

// Partially working! Tested: RL, RR, SL0, SL1, SRA, SRX
/* Shift and rotate assembler mnemonics!
 * RL s7   ; s7 <- {s7[6:0],s7[7]}, CARRY <- s7[7]
 * RR s7   ; s7 <- {s7[0],s7[7:1]}, CARRY <- s7[0]
 * SL0 s7  ; s7 <- {s7[6:0],0}, CARRY <- s7[7]
 * SL1 s7  ; s7 <- {s7[6:0],1}, CARRY <- s7[7], ZERO <- 0
 * SLA s7  ; s7 <- {s7[6:0],CARRY}, CARRY <- s7[7]
 * SLX s7  ; s7 <- {s7[6:0],s7[0]}, CARRY <- s7[7]
 * SR0 s7  ; s7 <- {0,s7[7:1]}, CARRY <- s7[0]
 * SR1 s7  ; s7 <- {1,s7[7:1]}, CARRY <- s7[0], ZERO <- 0
 * SRA s7  ; s7 <- {CARRY, s7[7:1]}, CARRY <- s7[0]
 * SRX s7  ; s7 <- {s7[7], s7[7:1]}, CARRY <- s7[0] */
void shift_rotate(uint16_t instruction) {
#ifdef DEBUG
    printf("shift_rotate\n");
#endif
    uint8_t operation = (instruction & 0xFF);
    uint8_t register_data = register_file[x_register_pointer];

    uint8_t leftmost_bit = register_data & 0x80;  // s7[7]
    uint8_t rightmost_bit = register_data & 0x01; // s7[0]

#ifdef DEBUG
    printf("leftmost: %x rightmost %x\n", leftmost_bit, rightmost_bit);
#endif

    switch (operation) {
    case RL_MASK:
#ifdef DEBUG
        printf("RL\n");
#endif
        register_file[x_register_pointer] <<= 1;
        register_file[x_register_pointer] &= ~1; // clear lsb bit
        register_file[x_register_pointer] |= (leftmost_bit >> 7);
        carry_flag = leftmost_bit;
        break;

    case RR_MASK:
#ifdef DEBUG
        printf("RR\n");
#endif
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] &= ~0x80; // clear msb
        register_file[x_register_pointer] |= rightmost_bit << 7;
        carry_flag = rightmost_bit;
        break;

    case SL0_MASK:
#ifdef DEBUG
        printf("SL0\n");
#endif
        register_file[x_register_pointer] <<= 1;
        register_file[x_register_pointer] &= ~0x01; // clear lsb

        carry_flag = leftmost_bit;
        break;
    case SL1_MASK:
#ifdef DEBUG
        printf("SL1\n");
#endif
        register_file[x_register_pointer] <<= 1;
        register_file[x_register_pointer] |= 0x01; // set msb
        carry_flag = leftmost_bit;
        zero_flag = false;
        break;

    case SLA_MASK:
#ifdef DEBUG
        printf("SLA\n");
#endif
        register_file[x_register_pointer] <<= 1;
        if (carry_flag)
            register_file[x_register_pointer] |= 0x01; // change rightmost bit to 1
        else
            register_file[x_register_pointer] &= 0xFE; // change rightmost bit to 0
        carry_flag = leftmost_bit;
        break;
    case SLX_MASK:
#ifdef DEBUG
        printf("SLX\n");
#endif
        register_file[x_register_pointer] <<= 1;
        // TODO error!
        register_file[x_register_pointer] &= 0x7F; // change leftmost bit to 0
        carry_flag = leftmost_bit;
        break;

    case SR0_MASK:
#ifdef DEBUG
        printf("SR0\n");
#endif
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] &= 0x7F; // change leftmost bit to 0
        carry_flag = rightmost_bit;
    case SR1_MASK:
#ifdef DEBUG
        printf("SR1\n");
#endif
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] |= 0x80; // change leftmost bit to 1
        zero_flag = false;
        carry_flag = rightmost_bit;
        break;


    case SRA_MASK:
#ifdef DEBUG
        printf("SRA\n");
#endif
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] |= (((uint8_t) carry_flag) << 7);
        carry_flag = rightmost_bit;

        break;
    case SRX_MASK:
#ifdef DEBUG
        printf("SRX\n");
#endif
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] |= leftmost_bit;
        carry_flag = rightmost_bit;

        break;

    }

    /* instruction code summary in reference_manual.pdf
       does not specify this but detailed instrucion description
       in Appendic C does! */
    if (register_file[x_register_pointer] == 0)
        zero_flag = true;
    else
        zero_flag = false;
}

/* WARNING! CARGO CULT PROGRAMMING:
 * http://stackoverflow.com/questions/109023/
 * how-to-count-the-number-of-set-bits-in-a-32-bit-integer */
uint8_t NumberOfSetBits(uint32_t i)
{
     // Java: use >>> instead of >>
     // C or C++: use uint32_t
     i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

/* test s7, ff  ; if (s7 AND ff) = 0 then ZERO <- 1,
 *              ;   CARRY <- odd parity of (s7 AND ff) */
void test_k_to_sx(){
#ifdef DEBUG
    printf("test_k_to_sx\n");
#endif

    if ((register_file[x_register_pointer]
         & constant_argument) == 0) {
        zero_flag = true;
    }

    carry_flag = (NumberOfSetBits(
                      (register_file[x_register_pointer]
                       & constant_argument)) % 2 == 0)
        ? 1
        : 0;
}

void test_sy_to_sx(){
#ifdef DEBUG
    printf("test_sy_to_sx\n");
#endif

    if ((register_file[x_register_pointer]
         & register_file[y_register_pointer]) == 0) {
        zero_flag = true;
    }

    carry_flag = (NumberOfSetBits(
                      (register_file[x_register_pointer]
                       & register_file[y_register_pointer])) % 2 == 0)
        ? 1
        : 0;
}

/* unimplemented = no operation */
void input_p_to_x() {
#ifdef DEBUG
    printf("input_p_to_x\n");
#endif
}


void input_y_to_x() {
#ifdef DEBUG
    printf("input_y_to_x\n");
#endif
}


void output_p_to_x() {
#ifdef DEBUG
    printf("output_p_to_x\n");
#endif
}

void output_y_to_x() {
#ifdef DEBUG
    printf("output_y_to_x\n");
#endif
}

void interrupt() {         }
void returni() {         }
void zero() {         }
void not_zero() {         }
void carry() {         }
void not_carry() {         }

#ifdef SIMULATOR
void make_report(void) {
    int i;

    printf("Current state of stack\n");

    for (i = 0; i < STACK; i++) {
        printf("  stack content at pos %2d: %x\n", i, stack[i]);
    }

    printf("\nCurrent state of register file\n");
    for (i = 0; i < REG_FILE; i++) {
        printf("  content in register %d: %x\n", i, register_file[i]);
    }

#ifdef FANCY_RAM_REPORT
    int j;
    /* Fancy ram_file table <3 */
    printf("\nCurrent state of ram file\n");
      printf("Cols:  0     1     2     3     4     5     6     7      \nRows  ");
    for (j = 0; j < 8; j++) {
        printf("%s                                                 \n    %d ",
               (j == 0 ? "" : "      ") , j);
        for (i = 0; i < 8; i++) {
            printf(" 0x%02x %s", ram_file[j*8+i], (i == 7 ? "\n" : ""));
        }
    }
#else
    printf("\nCurrent state of ram file\n");
    for (i = 0; i < RAM_FILE; i++) {
        printf("  content in ram file at position %d: %x\n", i, ram_file[i]);
    }
#endif

    printf("\nProgram counter points at instruction %d\n", program_counter);
    printf("Stack pointer points at %d\n", stack_pointer);

    printf("Constant argument has value %d\n", constant_argument);
    printf("X register pointer has value %d\n", x_register_pointer);
    printf("Y register pointer has value %d\n", y_register_pointer);
    printf("Carry flag has value %d\n", carry_flag);
    printf("Zero flag has value %d\n", zero_flag);
}
#endif

int main(void) {
    uint16_t instruction;

    printf("Picoc starting execution...\n");

    /* simple fetch and execute cycle*/
    while (program_counter < INS_ROM &&
           instruction_rom[program_counter] != 0x0000) {
        instruction = instruction_rom[program_counter];

        dispatch_instruction(instruction);
    }

    printf("Picoc ended execution...\n");

# ifdef SIMULATOR
    printf("Number of registers: [%d] \n", REG_FILE);
    printf("Number of ram slots: [%d] \n", RAM_FILE);
    printf("Number of stack frames : [%d] \n", STACK);
    printf("Number of stored instructions: [%d] \n", INS_ROM);
    make_report();
# endif

    return 0;
}
