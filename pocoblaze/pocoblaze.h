#include <stdint.h>

#ifndef PB_PROTO
#define PB_PROTO

/* Prototypes */
uint8_t stack_pop();
void stack_push(uint16_t instruction_pointer);
void dispatch_instruction(uint16_t instruction);
//
void load_k_to_x();
void load_y_to_x();
void ret();
void jump();
void jump_z();
void jump_nz();
void jump_c();
void jump_nc();
void call();
void call_c();
void call_nc();
void call_z();
void call_nz();
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
void fetch();
void store();
void fetch_reg_from_sY();
void fetch_reg_from_k();
void store_sX_at_sY();
void store_sX_at_k();
// not implemented
void interrupt();
void returni();
void zero();
void not_zero();
void carry();
void not_carry();

#endif
