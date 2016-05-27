#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Configuration variables for the softcore
static const int REG_FILE = 8;
static const int RAM_FILE = 64;
static const int STACK = 16;
static const int INS_ROM = 256;

struct func_mapping {
  uint16_t instruction;
  void (*handler)(uint16_t);
};


static const uint16_t instruction_rom[INS_ROM] = {0};

static uint8_t register_file[REG_FILE] = {0};
static uint8_t stack[STACK] = {0};

static uint8_t program_counter = 0;
static uint8_t stack_pointer = 0;

static uint8_t constant_argument = 0;
static uint8_t x_register_pointer = 0; // three bits
static uint8_t y_register_pointer = 0; // three bits

static uint8_t ram_file[RAM_FILE] = {0}; // unused
static uint8_t output_port = 0; // unused
static uint8_t input_port = 0; // unused

static bool carry_flag = false;

// unused
static bool zero_flag = false;
static bool read_strobe = false;
static bool write_strobe = false;

// unused
static bool interrupt_enable = false;
static bool active_interrupt = false;


/* Prototypes */
uint8_t stack_pop();
void stack_push(uint16_t instruction_pointer);
void dispatch_instruction(uint16_t instruction);
/* Following functions implement the opcodes
 * the picoblaze architecture */
void load_k_to_x(uint16_t instruction);
void load_y_to_x(uint16_t instruction);
void call(uint16_t instruction);
void ret(uint16_t instruction);
void jump(uint16_t instruction);
void and_k_to_x(uint16_t instruction);
void and_y_to_x(uint16_t instruction);
void or_k_to_x(uint16_t instruction);
void or_y_to_x(uint16_t instruction);
void xor_k_to_x(uint16_t instruction);
void xor_y_to_x(uint16_t instruction);
void add_k_to_x(uint16_t instruction);
void add_y_to_x(uint16_t instruction);
void addcy_k_to_x(uint16_t instruction);
void addcy_y_to_x(uint16_t instruction);
void sub_k_to_x(uint16_t instruction);
void sub_y_to_x(uint16_t instruction);
void subcy_k_to_x(uint16_t instruction);
void subcy_y_to_x(uint16_t instruction);
void shift_rotate(uint16_t instruction);
void input_p_to_x(uint16_t instruction);
void input_y_to_x(uint16_t instruction);
void output_p_to_x(uint16_t instruction);
void output_y_to_x(uint16_t instruction);
/* not implemented */
void interrupt(uint16_t instruction);
void returni(uint16_t instruction);
void zero(uint16_t instruction);
void not_zero(uint16_t instruction);
void carry(uint16_t instruction);
void not_carry(uint16_t instruction);

const struct func_mapping func_instruction_mapping[] = {
{0xD0, jump},
{0xD8, call},
{0x90, ret},
{0x00, load_k_to_x},
{0x40, load_y_to_x},
{0x80, and_k_to_x},
{0x48, and_y_to_x},
{0x10, or_k_to_x},
{0x50, or_y_to_x},
{0x18, xor_k_to_x},
{0x58, xor_y_to_x},
{0x20, add_k_to_x},
{0x60, add_y_to_x},
{0x28, addcy_k_to_x},
{0x68, addcy_y_to_x},
{0x30, sub_k_to_x},
{0x70, sub_y_to_x},
{0x38, subcy_k_to_x},
{0x78, subcy_y_to_x},
{0xA0, shift_rotate},
{0x80, input_p_to_x},
{0xC0, input_y_to_x},
{0x88, output_p_to_x},
{0xC8, output_y_to_x},
{0xF0, interrupt},
{0xB0, returni},
{0x00, zero},
{0x80, not_zero},
{0x10, carry},
{0x18, not_carry}};

uint8_t stack_pop() {
    return stack[stack_pointer--];
}

void stack_push(const uint16_t instruction_pointer) {
  printf("In function: stack_push\n");
  stack[stack_pointer++] = instruction_pointer;
  return;
}

void dispatch_instruction(const uint16_t instruction) {
  printf("In function: dispatch_instruction\n");
  size_t i;
  uint16_t opTmp;
  for (i = 0;
       i < sizeof(func_instruction_mapping) / sizeof(struct func_mapping);
       i++)
  {
    if (instruction == (opTmp = func_instruction_mapping[i].instruction)) {
      printf("Executing instruction: %x\n", opTmp);
      func_instruction_mapping[i].handler(instruction);
    } else {
        printf("Invalid instruction!: %x\n", opTmp);
    }
  }

  program_counter++;

  return;
}

void load_k_to_x(uint16_t instruction) {
  printf("In function: load_k_to_x\n");
  register_file[x_register_pointer] = constant_argument;
  return;
}

void load_y_to_x(uint16_t instruction) {
  printf("In function: load_y_to_x\n");
  register_file[x_register_pointer] = register_file[y_register_pointer];
  return;
}

void call(uint16_t instruction) {
  printf("In function: call\n");
  stack_push(program_counter);
  program_counter = constant_argument;
  return;
}

void ret(uint16_t instruction) {
  printf("In function: ret\n");
  program_counter = stack_pop();
}

void jump(uint16_t instruction) {
  printf("In function: jump\n");
  program_counter = constant_argument;
}

void and_k_to_x(uint16_t instruction) {
  printf("In function: and_k_to_x\n");
  register_file[x_register_pointer] &= constant_argument;

  return;
}

void and_y_to_x(uint16_t instruction) {
  printf("In function: and_y_to_x\n");
  register_file[x_register_pointer] &= register_file[y_register_pointer];

  return;
}

void or_k_to_x(uint16_t instruction) {
  printf("In function: or_k_to_x\n");
  register_file[x_register_pointer] &= constant_argument;

  return;
}

void or_y_to_x(uint16_t instruction) {
  printf("In function: or_y_to_x\n");
  register_file[x_register_pointer] |= register_file[y_register_pointer];

  return;
}

void xor_k_to_x(uint16_t instruction) {
  printf("In function: xor_k_to_x\n");
  register_file[x_register_pointer] ^= constant_argument;

  return;
}

void xor_y_to_x(uint16_t instruction) {
  printf("In function: xor_y_to_x\n");
  register_file[x_register_pointer] ^= register_file[y_register_pointer];

  return;
}

void add_k_to_x(uint16_t instruction) {
  printf("In function: add_k_to_x\n");
  register_file[x_register_pointer] += constant_argument;

  return;
}

void add_y_to_x(uint16_t instruction) {
  printf("In function: add_y_to_x\n");
  register_file[x_register_pointer] += register_file[y_register_pointer];

  return;
}

void addcy_k_to_x(uint16_t instruction) {
  printf("In function: addcy_k_to_x\n");
  register_file[x_register_pointer] +=
      constant_argument + (carry_flag == true ? 1 : 0);

  return;
}

void addcy_y_to_x(uint16_t instruction) {
  printf("In function: addcy_y_to_x\n");
  register_file[x_register_pointer] +=
      register_file[y_register_pointer] + (carry_flag == true ? 1 : 0);

  return;
}

void sub_k_to_x(uint16_t instruction) {
  printf("In function: sub_k_to_x\n");
  register_file[x_register_pointer] -= constant_argument;

  return;
}

void sub_y_to_x(uint16_t instruction) {
  printf("In function: sub_y_to_x\n");
  register_file[x_register_pointer] -= register_file[y_register_pointer];

  return;
}

void subcy_k_to_x(uint16_t instruction) {
  printf("In function: subcy_k_to_x\n");
  register_file[x_register_pointer] -=
      constant_argument - (carry_flag == true ? 1 : 0);

  return;
}

void subcy_y_to_x(uint16_t instruction) {
  printf("In function: subcy_y_to_x\n");
  register_file[x_register_pointer] -=
      register_file[y_register_pointer] - (carry_flag == true ? 1 : 0);

  return;
}

/* Codeausschnitt aus picoblaze_behavioral.vhd:
   -- shift decoding bits
   shift_right     <= instruction(3);
   shift_in_bit    <= instruction(0);
   shift_code1     <= instruction(2);
   shift_code0     <= instruction(1);
   logical_code1   <= instruction(12);
   logical_code0   <= instruction(11);

   My interpretation:
   ------------------
   shift_code1 high => shift
   shift_code1 low => rotate
   shift_right high => right shift or rotate
   shift_right low => left shift or rotate
   shift_in_bit => if (shift_code1 = high) then
                     use shift_in_bit as fill bit. */
void shift_rotate(uint16_t instruction) {
  printf("In function: shift_rotate\n");
  uint16_t in_bit = instruction & 0x01;
  uint8_t register_data = register_file[x_register_pointer];

  if ((instruction & 0x01) == 0x01) {    // do a shift
      if ((instruction & 0x08) == 0x08) {  // shift right
      register_data >>= 1;
      register_data |= ((uint8_t)in_bit) << 7;
    } else {  // shift left
      register_data <<= 1;
      register_data |= (uint8_t)in_bit;
    }
  } else {                             // rotate
      if ((instruction & 0x08) == 0x08) {  // rotate right
      in_bit = register_data & 0x01;
      register_data >>= 1;
      register_data |= ((uint8_t)in_bit) << 7;
    } else {  // rotate left
      in_bit = register_data & 0x80;
      register_data <<= 1;
      register_data |= ((uint8_t)in_bit) >> 7;
    }
  }

  register_file[x_register_pointer] = register_data;
  return;
}

void input_p_to_x(uint16_t instruction) {
  printf("In function: input_p_to_x\n");

  return;
}

void input_y_to_x(uint16_t instruction) {
  printf("In function: input_y_to_x\n");

  return;
}

void output_p_to_x(uint16_t instruction) {
  printf("In function: output_p_to_x\n");

  return;
}

void output_y_to_x(uint16_t instruction) {
  printf("In function: output_y_to_x\n");

  return;
}

int main(void) {
  uint16_t instruction;

  /* simple fetch and execute cycle*/
  while (1) {
    instruction = instruction_rom[program_counter];
    dispatch_instruction(instruction);
  }

  return 0;
}
