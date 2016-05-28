#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Compile with -DSIMULATOR to enable stats */
// Configuration variables for the softcore
#define REG_FILE   8
#define RAM_FILE   64
#define STACK      16
#define INS_ROM    256

#define INS_MASK 0xf800
#define SX_MASK 0x0700
#define SY_MASK 0x00E0

// Setup of the processors register comonents
uint16_t instruction_rom[INS_ROM] = {
    0xD00D, 0x0201, 0x0300, 0x0400, 0xE940, 0xD407, 0x6300, 0x4800, 0xA308, 0xA408,
    0xA206, 0xD504, 0x9000, 0x0033, 0x01FF, 0xD801, 0x2301, 0x340A, 0x0000, 0x0000,
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
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

uint8_t register_file[REG_FILE] = { 0 };
uint8_t stack[STACK] = { 0 };

uint16_t program_counter = 0;
uint8_t stack_pointer   = 0;

uint8_t constant_argument  = 0;
uint8_t x_register_pointer = 0; // three bits
uint8_t y_register_pointer = 0; // three bits

bool carry_flag = false;

// unused
uint8_t ram_file[RAM_FILE] = { 0 };
uint8_t output_port = 0;
uint8_t input_port  = 0;

// unused
bool zero_flag    = false;
bool read_strobe  = false;
bool write_strobe = false;

// unused
bool interrupt_enable = false;
bool active_interrupt = false;

/* Prototypes */
uint8_t stack_pop();
void stack_push(uint16_t instruction_pointer);
void dispatch_instruction(uint16_t instruction);

/* Following functions implement the opcodes
 * the picoblaze architecture */
void load_k_to_x();
void load_y_to_x();
void call();
void ret();
void jump();
void jump_z();
void jump_nz();
void test_k_to_sx();
void test_sy_to_sx();
void and_k_to_x();
void and_y_to_x();
void or_k_to_x();
void or_y_to_x();
void xor_k_to_x();
void xor_y_to_x();
void add_k_to_x();
void add_y_to_x();
void addcy_k_to_x();
void addcy_y_to_x();
void sub_k_to_x();
void sub_y_to_x();
void subcy_k_to_x();
void subcy_y_to_x();
void shift_rotate(uint16_t instruction);
void input_p_to_x();
void input_y_to_x();
void output_p_to_x();
void output_y_to_x();

/* not implemented */
void interrupt();
void returni();
void zero();
void not_zero();
void carry();
void not_carry();

uint8_t stack_pop() {
    printf("In function: stack_pop\n");
    return stack[--stack_pointer];
}

void stack_push(const uint16_t instruction_pointer) {
    printf("In function: stack_push\n");
    stack[stack_pointer++] = instruction_pointer;
}

void dispatch_instruction(const uint16_t instruction) {
    printf("In function: dispatch_instruction | zero: %i | carry: %i |sp : %i\n",
           zero_flag, carry_flag,  stack_pointer);

    x_register_pointer = (instruction & SX_MASK) >> 8;
    y_register_pointer = (instruction & SY_MASK) >> 5;
    constant_argument = instruction & 0x00FF;

    printf("x_register_pointer: %i\n", x_register_pointer);
    printf("y_register_pointer: %i\n", y_register_pointer);
    printf("constant_argument: %i\n", constant_argument);

    switch (instruction & INS_MASK) {
    case 0xD000:
        switch (x_register_pointer){
        case 5:
            jump_nz(instruction);
            break;
        case 4:
            jump_z(instruction);
            break;
        default: jump(instruction);
            break;
        }
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0xD800:
        call();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x9000:
        ret();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x0000:
        load_k_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x4000:
        load_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x8000:
        and_k_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0xA800:
        test_k_to_sx();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0xE800:
        test_sy_to_sx();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x4800:
        and_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x1000:
        or_k_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x5000:
        or_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x1800:
        xor_k_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x5800:
        xor_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x2000:
        add_k_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x6000:
        add_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x2800:
        addcy_k_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x6800:
        addcy_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x3000:
        sub_k_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x7000:
        sub_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x3800:
        subcy_k_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x7800:
        subcy_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0xA000:
        shift_rotate(instruction);
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x8100:
        input_p_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0xC000:
        input_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x8800:
        output_p_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0xC800:
        output_y_to_x();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0xF000:
        interrupt();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0xB000:
        returni();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x0100:
        zero();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x8200:
        not_zero();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x1300:
        carry();
        printf("Executing instruction %x!\n", instruction);
        break;

    case 0x1400:
        not_carry();
        printf("Executing instruction %x!\n", instruction);
        break;
    }

    int i;
    for(i=0; i<REG_FILE; i++)
        printf("---- content in register %i: %02x\n", i, register_file[i]);

    printf("next instruciton!\n\n");
    program_counter++;
}

// working!
void load_k_to_x() {
    printf("In function: load_k_to_x\n");
    register_file[x_register_pointer] = constant_argument;
}

// working!
void load_y_to_x() {
    printf("In function: load_y_to_x\n");
    register_file[x_register_pointer] = register_file[y_register_pointer];
}

void jump_z(){
    printf("In function: jump_z\n");

    if (zero_flag)
        jump();
}

void jump_nz(){
    printf("In function: jump_nz\n");

    if (! zero_flag)
        jump();
}

// working!
void call() {
    printf("In function: call | pushed counter : %i\n", program_counter);
    stack_push(program_counter);
    // subtract one because dispatch_instruction
    // always increments PC
    program_counter = constant_argument - 1;
}

// working!
void ret() {
    program_counter = stack_pop();
    printf("In function: ret | poped counter : %i\n", program_counter);
}

// working!
void jump() {
    printf("In function: jump\n");
    // subtract one because dispatch_instruction
    // always increments PC
    program_counter = constant_argument - 1;
}

// working
void and_k_to_x() {
    printf("In function: and_k_to_x\n");
    register_file[x_register_pointer] &= constant_argument;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working
void and_y_to_x() {
    printf("In function: and_y_to_x\n");
    register_file[x_register_pointer] &= register_file[y_register_pointer];

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working
void or_k_to_x() {
    printf("In function: or_k_to_x\n");
    register_file[x_register_pointer] |= constant_argument;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working
void or_y_to_x() {
    printf("In function: or_y_to_x\n");
    register_file[x_register_pointer] |= register_file[y_register_pointer];

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working!
void xor_k_to_x() {
    printf("In function: xor_k_to_x\n");
    register_file[x_register_pointer] ^= constant_argument;

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working
void xor_y_to_x() {
    printf("In function: xor_y_to_x\n");
    register_file[x_register_pointer] ^= register_file[y_register_pointer];

    if (register_file[x_register_pointer] == 0)
        zero_flag = true;

    carry_flag = false;
}

// working!
void add_k_to_x() {
    printf("In function: add_k_to_x\n");
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
    printf("In function: add_y_to_x\n");
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
    printf("In function: addcy_k_to_x\n");
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
    printf("In function: addcy_y_to_x\n");
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
    printf("In function: sub_k_to_x\n");
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
    printf("In function: sub_y_to_x\n");
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
    printf("In function: subcy_k_to_x\n");
    int16_t temp = register_file[x_register_pointer];

    register_file[x_register_pointer] -= constant_argument;
    register_file[x_register_pointer] -= (carry_flag == true ? 1 : 0);

    printf("temp: %hu const: %hhu carry: %i\n", temp, constant_argument, carry_flag);

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
    printf("In function: subcy_y_to_x\n");
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

// partially working! RL, RR, SL0, SL1, SRA, SRX
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
    printf("In function: shift_rotate\n");
    uint8_t operation = (instruction & 0xFF);
    uint8_t register_data = register_file[x_register_pointer];

    uint8_t leftmost_bit = register_data & 0x80;  // s7[7]
    uint8_t rightmost_bit = register_data & 0x01; // s7[0]

    printf("leftmost: %x rightmost %x\n", leftmost_bit, rightmost_bit);

    switch (operation) {
    case RL_MASK:
        printf("RL\n");
        register_file[x_register_pointer] <<= 1;
        register_file[x_register_pointer] &= ~1; // clear lsb bit
        register_file[x_register_pointer] |= (leftmost_bit >> 7);
        carry_flag = leftmost_bit;
        break;

    case RR_MASK:
        printf("RR\n");
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] &= ~0x80; // clear msb
        register_file[x_register_pointer] |= rightmost_bit << 7;
        carry_flag = rightmost_bit;
        break;

    case SL0_MASK:
        printf("SL0\n");
        register_file[x_register_pointer] <<= 1;
        register_file[x_register_pointer] &= ~0x01; // clear lsb

        carry_flag = leftmost_bit;
        break;
    case SL1_MASK:
        printf("SL1\n");
        register_file[x_register_pointer] <<= 1;
        register_file[x_register_pointer] |= 0x01; // set msb

        carry_flag = leftmost_bit;
        zero_flag = false;
        break;

    case SLA_MASK:
        printf("SLA\n");
        register_file[x_register_pointer] <<= 1;
        if (carry_flag)
            register_file[x_register_pointer] |= 0x01; // change rightmost bit to 1
        else
            register_file[x_register_pointer] &= 0xFE; // change rightmost bit to 0
        carry_flag = leftmost_bit;
        break;
    case SLX_MASK:
        printf("SLX\n");
        register_file[x_register_pointer] <<= 1;
        // TODO error!
        register_file[x_register_pointer] &= 0x7F; // change leftmost bit to 0
        carry_flag = leftmost_bit;
        break;

    case SR0_MASK:
        printf("SR0\n");
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] &= 0x7F; // change leftmost bit to 0
        carry_flag = rightmost_bit;
    case SR1_MASK:
        printf("SR1\n");
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] |= 0x80; // change leftmost bit to 1
        zero_flag = false;
        carry_flag = rightmost_bit;
        break;


    case SRA_MASK:
        printf("SRA\n");
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] |= (((uint8_t) carry_flag) << 7);
        carry_flag = rightmost_bit;

        break;
    case SRX_MASK:
        printf("SRX\n");
        register_file[x_register_pointer] >>= 1;
        register_file[x_register_pointer] |= leftmost_bit;
        carry_flag = rightmost_bit;

        break;

    }

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

void test_sy_to_sx(){
    printf("In function: test_sy_to_sx\n");

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

/* unimplemented */
void input_p_to_x() {
    printf("In function: input_p_to_x\n");
}


void input_y_to_x() {
    printf("In function: input_y_to_x\n");
}


void output_p_to_x() {
    printf("In function: output_p_to_x\n");
}

void output_y_to_x() {
    printf("In function: output_y_to_x\n");
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
        printf("  stack content at pos %2i: %x\n", i, stack[i]);
    }

    printf("\nCurrent state of register file\n");
    for (i = 0; i < REG_FILE; i++) {
        printf("  content in register %i: %x\n", i, register_file[i]);
    }

    printf("\nProgram counter points at instruction %i\n", program_counter);
    printf("Stack pointer points at %i\n", stack_pointer);

    printf("Constant argument has value %i\n", constant_argument);
    printf("X register pointer has value %i\n", x_register_pointer);
    printf("Y register pointer has value %i\n", y_register_pointer);
    printf("Carry flag has value %i\n", carry_flag);
    printf("Zero flag has value %i\n", zero_flag);
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
    printf("Number of registers: [%i] \n", REG_FILE);
    printf("Number of ram slots: [%i] \n", RAM_FILE);
    printf("Number of stack frames : [%i] \n", STACK);
    printf("Number of stored instructions: [%i] \n", INS_ROM);
    make_report();
# endif

    return 0;
}
