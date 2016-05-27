/*******************************************************
assembler for picoblaze microcontroller

  v1.0 - developement started 8/5/2002
       - predefined instructions are identical with picoblaze VHDL code
	   - this program parse the assembly code and generates

	   - .bin file, program word in hex format
	   - .fmt file, formated assembly file
	   - .mcs file, intel mcs-86 format file for programming
	   - .vhd file, rom vhdl module for simulation
	   - .log file, program report

*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

/* declare all instructions */
/* same as picoblaze VHDL code */

/* program control group */
const char *jump_id = "11010";
const char *call_id = "11011";
const char *return_id = "10010";

/* logical group */
const char *load_k_to_x_id = "00000";
const char *load_y_to_x_id = "01000";
const char *and_k_to_x_id = "00001";
const char *and_y_to_x_id = "01001";
const char *or_k_to_x_id = "00010";
const char *or_y_to_x_id = "01010";
const char *xor_k_to_x_id = "00011";
const char *xor_y_to_x_id = "01011";

/* arithmetic group */
const char *add_k_to_x_id = "00100";
const char *add_y_to_x_id = "01100";
const char *addcy_k_to_x_id = "00101";
const char *addcy_y_to_x_id = "01101";
const char *sub_k_to_x_id = "00110";
const char *sub_y_to_x_id = "01110";
const char *subcy_k_to_x_id = "00111";
const char *subcy_y_to_x_id = "01111";

/* Test command */
const char *test_sX_sY = "11101";
const char *test_sX_k = "10101";

/* shift and rotate */
const char *shift_rotate_id = "10100";
/* shift decoding
instruction(3) - shift left/right
bit2 bit1 bit0
1    1    0    - SR0/SL0; 6
1    1    1    - SR1/Sl1; 7
0    1    0    - SRX/SLX; 2
0    0    0    - SRA/SLA; 0
1    0    0    - RR /RL ; 4
*/
#define SR0_SL0 6
#define SR1_SL1 7
#define SRX_SLX 2
#define SRA_SLA 0
#define RR_RL 4
#define SHIFT_RIGHT 8
#define SHIFT_LEFT 0

/* flip */ /* added new instruction */
const char *flip_id = "11111";


/* input/output group */
const char *input_p_to_x_id = "10000";
const char *input_y_to_x_id = "11000";
const char *output_p_to_x_id = "10001";
const char *output_y_to_x_id = "11001";

/* interrupt group */

const char *interrupt_id = "11110";
const char *returni_id = "10110";

/* flag */
const char *zero_id = "00";
const char *not_zero_id = "01";
const char *carry_id = "10";
const char *not_carry_id = "11";

#define MAX_LINE_COUNT 1000 /* max 1000 lines allowed */
#define PROGRAM_COUNT 256	/* total program word */

/* increase instruction_count for added new instruction */
#define instruction_count 31/* total instruction set */

#define CONSTANT_COUNT 100	/* max 100 constant can be declared */
#define REG_COUNT 8			/* max 8 namereg can be declared */


char filename[200];
FILE *ifp;
FILE *ofp;
FILE *ffp;

char linebuf[200];
int line_count = 0;
int constant_count = 0;
int reg_count = 0;
unsigned program_word[PROGRAM_COUNT]; /* program word array */

typedef struct reg {
	char *name;
	int value;
}reg_t;

reg_t reg_set[REG_COUNT]; /* namereg array */

typedef struct constant {
	char *name;
	int value;
}constant_t;

constant_t constant_set[CONSTANT_COUNT]; /* constant array */

typedef struct opcode {
	unsigned int address;
	char *label;
	char *instruction;
	char *op1;
	char *op2;
	char *comment;
}opcode_t;

opcode op[MAX_LINE_COUNT]; /* operaton array to save info for each line */

const char *instruction_set[] = {
	"JUMP",		/* 0 */
	"CALL",		/* 1 */
	"RETURN",	/* 2 */
	"LOAD",		/* 3 */
	"AND",		/* 4 */
	"OR",		/* 5 */
	"XOR",		/* 6 */
	"ADD",		/* 7 */
	"ADDCY",	/* 8 */
	"SUB",		/* 9 */
	"SUBCY",	/* 10 */
	"SR0",		/* 11 */
	"SR1",		/* 12 */
	"SRX",		/* 13 */
	"SRA",		/* 14 */
	"RR",		/* 15 */
	"SL0",		/* 16 */
	"SL1",		/* 17 */
	"SLX",		/* 18 */
	"SLA",		/* 19 */
	"RL",		/* 20 */
	"INPUT",	/* 21 */
	"OUTPUT",	/* 22 */
	"RETURNI",	/* 23 */
	"ENABLE",	/* 24 */
	"DISABLE",  /* 25 */
	"CONSTANT",	/* 26 */
	"NAMEREG",	/* 27 */
	"ADDRESS",	/* 28 */
	"FLIP",     /* 29 */
    "TEST"};	/* 30 */ /* added new instruction */

int error = 0;
/*====================================== */
void free_mem(void)
{
	int i;

	for(i = 0; i < line_count; i++){
		if(op[i].comment != NULL) free(op[i].comment);
		if(op[i].label != NULL) free(op[i].label);
		if(op[i].instruction != NULL) free(op[i].instruction);
		if(op[i].op1 != NULL) free(op[i].op1);
		if(op[i].op2 != NULL) free(op[i].op2);
	}
}

/*====================================== */
void init_program_word(void)
{
	int i;

	for(i = 0; i < PROGRAM_COUNT; i++)
		program_word[i] = 0;
}

/*====================================== */
void error_out(void)
{
	free_mem();
	exit(1);
}

/*====================================== */
/* convert hex string to int, return -1 if not valid */
int htoi(char *s)
{
	int i, l, n = 0;
	char *p;

	l = strlen(s);
	for(i = 0; i < l; i++){
		p = s+l-1-i;
		if(isdigit(*p) || (*p >= 'A' && *p <= 'F')){
			if(isdigit(*p)) n += (*p - '0') * (int) pow(16 , i);
			else n += (*p - 'A' + 10) * (int) pow(16 , i);
		} else return (-1);
	}
	return(n);
}

/*====================================== */
/* Only S0 - S7 are valid */
int register_number(char *s)
{
	if(*s != 'S') return( -1 );
	if(strlen(s) != 2) return( -1 );
	if((*(s+1) >= '0') && (*(s+1) <= '7'))
		return (*(s+1) - '0');
	else return( -1 );
}

/*====================================== */
void insert_instruction(const char *s, int p)
{
	int i, l;
	unsigned n = 0;

	l = strlen(s);
	for(i = 0; i < l; i++)
		if(*(s+i) == '1')
			n = n + (unsigned) pow(2, (l-i-1));

	program_word[p] = program_word[p] | (n << 11);
}

/*====================================== */
void insert_sXX(int c, int p)
{
	program_word[p] = program_word[p] | (unsigned) (c << 8);
}

/*====================================== */
void insert_sYY(int c, int p)
{
	program_word[p] = program_word[p] | (unsigned) (c << 5);
}

/*====================================== */
void insert_constant(int c, int p)
{
	program_word[p] = program_word[p] | (unsigned) (c);
}

/*====================================== */
void insert_flag(int c, int p)
{
	program_word[p] = program_word[p] | (unsigned) (c << 8);
}

/*====================================== */
int decode_flag(char *s)
{
	if(!strcmp(s, "Z")) return (4);
	else if (!strcmp(s, "NZ")) return (5);
	else if (!strcmp(s, "C")) return (6);
	else if (!strcmp(s, "NC")) return (7);
	else return (-1);
}

/*====================================== */
int find_constant(char *s)
{
	int i;

	for(i = 0; i < constant_count; i++)
		if(!strcmp(s, constant_set[i].name))
			return(constant_set[i].value);
	return(-1);
}

/*====================================== */
int find_label(char *s)
{
	int i;

	for(i = 0; i < line_count; i++)
		if(op[i].label != NULL)
			if(!strcmp(s, op[i].label))
				return(op[i].address);
	return(-1);
}
/*====================================== */
int find_namereg(char *s)
{
	int i;

	for(i = 0; i < reg_count; i++)
		if(!strcmp(s, reg_set[i].name))
			return(reg_set[i].value);
	return(-1);
}

static void strupr(char *string){
    while (*string){
        *string = toupper(*string); 
        string++;
    }
}

/*====================================== */
int parse_linebuf(void)
{
	char *ptr;
	char seps[]   = " :;,\t\n";
    char *token;

	/* get comment */
	if( (ptr = strchr(linebuf, ';')) != NULL ){
		op[line_count].comment = strdup(ptr);
		*ptr = '\0';
		op[line_count].comment[strlen(op[line_count].comment)-1] = '\0';
	}

	/* get label */
	if( (ptr = strchr(linebuf, ':')) != NULL ){
		token = strtok( linebuf, seps );
		op[line_count].label = strdup(token);
		strupr(op[line_count].label);
	}

	/* get instruction */
	if (ptr == NULL)
		token = strtok( linebuf, seps );
	else token = strtok( NULL, seps);
	if (token != NULL){
		op[line_count].instruction = strdup(token);
		strupr(op[line_count].instruction);
	} else return (0);

	/* get op1 */
	token = strtok( NULL, seps);
	if (token != NULL){
		op[line_count].op1 = strdup(token);
		strupr(op[line_count].op1);
	} else return (0);

	/* get op2 */
	token = strtok( NULL, seps);
	if (token != NULL){
		op[line_count].op2 = strdup(token);
		strupr(op[line_count].op2);
	} else return (0);

	/* make sure nothing left */
	token = strtok( NULL, seps);
	if (token != NULL){
		printf("\nToo many operands in line %d\n", line_count+1);
		fprintf(ofp,"\nToo many operands in line %d\n", line_count+1);
		error++;
	}
	return (0);
}

/*====================================== */
/* syntax test and assign addresses */
void test_instructions(void)
{
	int i, j, k;
	int address = 0;

	for(i = 0; i < line_count; i++){
		if(op[i].instruction != NULL){
			for(j = 0; j < instruction_count; j++)
				if(!strcasecmp(op[i].instruction, instruction_set[j]))
					break;
			if(j >= instruction_count){
				printf("Unknown instruction - %s found on line %d\n",op[i].instruction, i+1);
				fprintf(ofp,"Unknown instruction - %s found on line %d\n",op[i].instruction, i+1);
				error++;
			}
			switch (j)
			{
				case 0: /* JUMP */
				case 1: /* CALL */
					if(op[i].op2 != NULL){
						if(decode_flag(op[i].op1) == -1){
							printf("ERROR - Invalid operand %s on line %d\n", op[i].op1, i+1);
							fprintf(ofp,"ERROR - Invalid operand %s on line %d\n", op[i].op1, i+1);
							error++;
						}
					} else if(op[i].op1 == NULL){
						printf("ERROR - Missing operand for %s on line %d\n",op[i].instruction, i+1);
						fprintf(ofp,"ERROR - Missing operand for %s on line %d\n",op[i].instruction, i+1);
						error++;
					}
					break;
				case 2: /* RETURN */
					if(op[i].op2 != NULL){
						printf("ERROR - Too many Operands for %s\n on line %d", op[i].instruction, i+1);
						fprintf(ofp,"ERROR - Too many Operands for %s\n on line %d", op[i].instruction, i+1);
						error++;
					} else if (op[i].op1 != NULL){
						if(decode_flag(op[i].op1) == -1){
							printf("ERROR - Invalid operand %s on line %d\n", op[i].op1, i+1);
							fprintf(ofp,"ERROR - Invalid operand %s on line %d\n", op[i].op1, i+1);
							error++;
						}
					}
					break;
				case 3: /* LOAD */
				case 4: /* AND */
				case 5: /* OR */
				case 6: /* XOR */
				case 7: /* ADD */
				case 8: /* ADDCY */
				case 9: /* SUB */
				case 10: /* SUBCY */
				case 30: /* TEST */ /* added new instruction, same syntax with shift/rotate */
					if((op[i].op1 == NULL) || (op[i].op2 == NULL)){
						printf("ERROR - Missing operand for %s on line %d\n",op[i].instruction, i+1);
						fprintf(ofp,"ERROR - Missing operand for %s on line %d\n",op[i].instruction, i+1);
						error++;
					}
					break;
				case 11: /* SR0 */
				case 12: /* SR1 */
				case 13: /* SRX */
				case 14: /* SRA */
				case 15: /* RR */
				case 16: /* SL0 */
				case 17: /* SL1 */
				case 18: /* SLX */
				case 19: /* SLA */
				case 20: /* RL */
				case 29: /* FLIP */ /* added new instruction, same syntax with shift/rotate */
					if(op[i].op2 != NULL){
						printf("ERROR - Too many Operands for %s on line %d\n", op[i].instruction, i+1);
						fprintf(ofp,"ERROR - Too many Operands for %s on line %d\n", op[i].instruction, i+1);
						error++;
					} else if(op[i].op1 == NULL){
						printf("ERROR - Missing operand for %s on line %d\n", op[i].instruction, i+1);
						fprintf(ofp,"ERROR - Missing operand for %s on line %d\n", op[i].instruction, i+1);
						error++;
					}
					break;
				case 21: /* INPUT */
				case 22: /* OUTPUT */
					if((op[i].op1 == NULL) || (op[i].op2 == NULL)){
						printf("ERROR - Missing operand for %s on line %d\n",op[i].instruction, i+1);
						fprintf(ofp,"ERROR - Missing operand for %s on line %d\n",op[i].instruction, i+1);
						error++;
					}					break;
				case 23: /* RETURNI */
					if(op[i].op2 != NULL){
						printf("ERROR - Too many Operands for RETURNI on line %d\n", i+1);
						fprintf(ofp,"ERROR - Too many Operands for RETURNI on line %d\n", i+1);
						error++;
					} else if(op[i].op1 == NULL){
						printf("ERROR - Missing operand for RETURNI on line %d\n", i+1);
						fprintf(ofp,"ERROR - Missing operand for RETURNI on line %d\n", i+1);
						error++;
					} else if(strcmp(op[i].op1,"ENABLE") && strcmp(op[i].op1,"DISABLE")){
						printf("ERROR - Invalid operand on line %d, only ENABLE/DISABLE allowed\n", i+1);
						fprintf(ofp,"ERROR - Invalid operand on line %d, only ENABLE/DISABLE allowed\n", i+1);
						error++;
					}
					break;
				case 24: /* ENABLE */
				case 25: /* DISABLE */
					if(op[i].op2 != NULL){
						printf("ERROR - Too many Operands for ENABLE/DISABLE on line %d\n", i+1);
						fprintf(ofp,"ERROR - Too many Operands for ENABLE/DISABLE on line %d\n", i+1);
						error++;
					} else if(op[i].op1 == NULL){
						printf("ERROR - Missing operand for %s on line %d\n", op[i].instruction, i+1);
						fprintf(ofp,"ERROR - Missing operand for %s on line %d\n", op[i].instruction, i+1);
						error++;
					} else if(strcmp(op[i].op1,"INTERRUPT")){
						printf("ERROR - Invalid operand on line %d, only INTERRUPT allowed\n", i+1);
						fprintf(ofp,"ERROR - Invalid operand on line %d, only INTERRUPT allowed\n", i+1);
						error++;
					}
					break;
				case 26: /* CONSTANT */
					if((op[i].op1 == NULL) || (op[i].op2 == NULL)){
						printf("ERROR - Missing operand for CONSTANT on line %d\n", i+1);
						fprintf(ofp,"ERROR - Missing operand for CONSTANT on line %d\n", i+1);
						error++;
					} else if(htoi(op[i].op1) != -1) {
						printf("ERROR - Invalid operand %s for CONSTANT on line %d\n", op[i].op1, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s for CONSTANT on line %d\n", op[i].op1, i+1);
						error++;
					} else if(htoi(op[i].op2) == -1){
						printf("ERROR - Invalid operand %s for CONSTANT on line %d\n", op[i].op2, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s for CONSTANT on line %d\n", op[i].op2, i+1);
						error++;
					} else {
						if(constant_count >= CONSTANT_COUNT){
							printf("ERROR - Too many CONSTANT declared\n");
							fprintf(ofp,"ERROR - Too many CONSTANT declared\n");
							error++;
						}
						for(k = 0; k < constant_count; k++)
							if(!strcmp(constant_set[k].name, op[i].op1)){
								printf("ERROR - Duplicate CONSTANT name %s found\n", op[i].op1);
								fprintf(ofp,"ERROR - Duplicate CONSTANT name %s found\n", op[i].op1);
								error++;
							}
						constant_set[constant_count].name = op[i].op1;
						constant_set[constant_count].value = htoi(op[i].op2);
						if(constant_set[constant_count].value >= PROGRAM_COUNT){
							printf("ERROR - Invalid operand %s for CONSTANT on line %d\n", op[i].op2, i+1);
							fprintf(ofp,"ERROR - Invalid operand %s for CONSTANT on line %d\n", op[i].op2, i+1);
							error++;
						}
						constant_count++;
					}
					break;
				case 27: /* NAMEREG */
					if((op[i].op1 == NULL) || (op[i].op2 == NULL)){
						printf("ERROR - Missing operand for NAMEREG on line %d\n", i+1);
						fprintf(ofp,"ERROR - Missing operand for NAMEREG on line %d\n", i+1);
						error++;
					} else if(htoi(op[i].op2) != -1){
						printf("ERROR - Invalid operand %s for NAMEREG on line %d\n", op[i].op2, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s for NAMEREG on line %d\n", op[i].op2, i+1);
						error++;
					} else if(register_number(op[i].op1) == -1){
						printf("ERROR - Invalid operand %s for NAMEREG on line %d\n", op[i].op1, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s for NAMEREG on line %d\n", op[i].op1, i+1);
						error++;
					} else {
						if(reg_count >= REG_COUNT){
							printf("ERROR - Too many NAMEREG declared\n");
							fprintf(ofp,"ERROR - Too many NAMEREG declared\n");
							error++;
						}
						for(k = 0; k < reg_count; k++)
							if(!strcmp(reg_set[k].name, op[i].op2)){
								printf("ERROR - Duplicate NAMEREG name %s found\n", op[i].op2);
								fprintf(ofp,"ERROR - Duplicate NAMEREG name %s found\n", op[i].op2);
								error++;
							}
						reg_set[reg_count].name = op[i].op2;
						reg_set[reg_count].value = register_number(op[i].op1);
						reg_count++;
					}
					break;
				case 28: /* ADDRESS */
					//assign op1 to address
					if(op[i].op2 != NULL){
						printf("ERROR - Too many Operands for ADDRESS directive on line %d\n", i+1);
						fprintf(ofp,"ERROR - Too many Operands for ADDRESS directive on line %d\n", i+1);
						error++;
					} else if(op[i].op1 == NULL){
						printf("ERROR - Missing operand for ADDRESS directive on line %d\n", i+1);
						fprintf(ofp,"ERROR - Missing operand for ADDRESS directive on line %d\n", i+1);
						error++;
					} else {
						address = htoi(op[i].op1);
						if((address == -1) || (address >= PROGRAM_COUNT)){
							printf("ERROR - Invalid ADDRESS directive on line %d\n", i+1);
							fprintf(ofp,"ERROR - Invalid ADDRESS directive on line %d\n", i+1);
							error++;
						}
					}
					break;
			}
			op[i].address = address;
			/* add (j > 28) for FLIP instruction, - added new instruction */
			if((j < 26) || (j > 28) || (j == 30)) address++;
		} else op[i].address = address; /* This is a comment line*/
	}
}

/*====================================== */
/* parse instructions and write program word */
void write_program_word(void)
{
	int i, j, reg_n;
	const char *kptr, *sptr;

	for(i = 0; i < line_count; i++){
		if(op[i].instruction != NULL){
			for(j = 0; j < instruction_count; j++)
				if(!strcasecmp(op[i].instruction, instruction_set[j]))
					break;
			switch (j)
			{
				case 0: /* JUMP */
				case 1: /* CALL */
					if(j == 0)
						kptr = jump_id;
					else
						kptr = call_id;
					insert_instruction(kptr, op[i].address);
					if(op[i].op2 == NULL){
						if((reg_n = find_label(op[i].op1)) != -1){
							insert_constant(reg_n, op[i].address);
						} else if((reg_n = find_constant(op[i].op1)) != -1){
							insert_constant(reg_n, op[i].address);
						} else if(((reg_n = htoi(op[i].op1)) != -1) && (reg_n < PROGRAM_COUNT)){
							insert_constant(reg_n, op[i].address);
						} else {
							printf("ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
							fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
							error++;
						}
					} else {
						if(op[i].op1 != NULL){
							reg_n = decode_flag(op[i].op1);
							insert_flag(reg_n, op[i].address);
						}
						if((reg_n = find_label(op[i].op2)) != -1){
							insert_constant(reg_n, op[i].address);
						} else if((reg_n = find_constant(op[i].op2)) != -1){
							insert_constant(reg_n, op[i].address);
						} else if(((reg_n = htoi(op[i].op2)) != -1) && (reg_n < PROGRAM_COUNT)){
							insert_constant(reg_n, op[i].address);
						} else {
							printf("ERROR - Invalid operand %s on line %d\n",op[i].op2, i+1);
							fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op2, i+1);
							error++;
						}
					}
					break;
				case 2: /* RETURN */
					insert_instruction(return_id,op[i].address);
					if(op[i].op1 != NULL){
						reg_n = decode_flag(op[i].op1);
						insert_flag(reg_n, op[i].address);
					}
					break;
				case 3: /* LOAD */
				case 4: /* AND */
				case 5: /* OR */
				case 6: /* XOR */
				case 7: /* ADD */
				case 8: /* ADDCY */
				case 9: /* SUB */
				case 10: /* SUBCY */
                case 30: /* TEST */
					if(j == 3){ kptr = load_k_to_x_id; sptr = load_y_to_x_id;}
					if(j == 4){ kptr = and_k_to_x_id; sptr = and_y_to_x_id;}
					if(j == 5){ kptr = or_k_to_x_id; sptr = or_y_to_x_id;}
					if(j == 6){ kptr = xor_k_to_x_id; sptr = xor_y_to_x_id;}
					if(j == 7){ kptr = add_k_to_x_id; sptr = add_y_to_x_id;}
					if(j == 8){ kptr = addcy_k_to_x_id; sptr = addcy_y_to_x_id;}
					if(j == 9){ kptr = sub_k_to_x_id; sptr = sub_y_to_x_id;}
					if(j == 10){ kptr = subcy_k_to_x_id; sptr = subcy_y_to_x_id;}
                    /* Edit: TEST */
					if(j == 30){ kptr = test_sX_k; sptr = test_sX_sY;}

					if((reg_n = find_namereg(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else if((reg_n = register_number(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else {
						printf("ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						error++;
					}
					if((reg_n = find_constant(op[i].op2)) != -1){
						insert_constant(reg_n, op[i].address);
						insert_instruction(kptr,op[i].address);
					} else if((reg_n = find_namereg(op[i].op2)) != -1){
						insert_sYY(reg_n, op[i].address);
						insert_instruction(sptr,op[i].address);
					} else if((reg_n = register_number(op[i].op2)) != -1){
						insert_sYY(reg_n, op[i].address);
						insert_instruction(sptr,op[i].address);
					} else if(((reg_n = htoi(op[i].op2)) != -1) && (reg_n < PROGRAM_COUNT)){
						insert_constant(reg_n, op[i].address);
						insert_instruction(kptr,op[i].address);
					} else {
						printf("ERROR - Invalid operand %s on line %d\n",op[i].op2, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op2, i+1);
						error++;
					}
					break;
				case 11: /* SR0 */
				case 12: /* SR1 */
				case 13: /* SRX */
				case 14: /* SRA */
				case 15: /* RR */
				case 16: /* SL0 */
				case 17: /* SL1 */
				case 18: /* SLX */
				case 19: /* SLA */
				case 20: /* RL */
					insert_instruction(shift_rotate_id,op[i].address);
					if((reg_n = find_namereg(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else if((reg_n = register_number(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else {
						printf("ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						error++;
					}
					if((j >= 11) && (j <= 15)) reg_n = SHIFT_RIGHT;
					else reg_n = SHIFT_LEFT;
					if((j == 11) || (j == 16)) reg_n = reg_n + SR0_SL0;
					else if((j == 12) || (j == 17)) reg_n = reg_n + SR1_SL1;
					else if((j == 13) || (j == 18)) reg_n = reg_n + SRX_SLX;
					else if((j == 14) || (j == 19)) reg_n = reg_n + SRA_SLA;
					else if((j == 15) || (j == 20)) reg_n = reg_n + RR_RL;
					insert_constant(reg_n, op[i].address);
					break;
				case 21: /* INPUT */
					if((reg_n = find_namereg(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else if((reg_n = register_number(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else {
						printf("ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						error++;
					}
					if((reg_n = find_constant(op[i].op2)) != -1){
						insert_constant(reg_n, op[i].address);
						insert_instruction(input_p_to_x_id,op[i].address);
					} else if((reg_n = find_namereg(op[i].op2)) != -1){
						insert_sYY(reg_n, op[i].address);
						insert_instruction(input_y_to_x_id,op[i].address);
					} else if((reg_n = register_number(op[i].op2)) != -1){
						insert_sYY(reg_n, op[i].address);
						insert_instruction(input_y_to_x_id,op[i].address);
					} else if(((reg_n = htoi(op[i].op2)) != -1) && (reg_n < PROGRAM_COUNT)){
						insert_constant(reg_n, op[i].address);
						insert_instruction(input_p_to_x_id,op[i].address);
					} else {
						printf("ERROR - Invalid operand %s on line %d\n",op[i].op2, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op2, i+1);
						error++;
					}
					break;
				case 22: /* OUTPUT */
					if((reg_n = find_namereg(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else if((reg_n = register_number(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else {
						printf("ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						error++;
					}
					if((reg_n = find_constant(op[i].op2)) != -1){
						insert_constant(reg_n, op[i].address);
						insert_instruction(output_p_to_x_id,op[i].address);
					} else if((reg_n = find_namereg(op[i].op2)) != -1){
						insert_sYY(reg_n, op[i].address);
						insert_instruction(output_y_to_x_id,op[i].address);
					} else if((reg_n = register_number(op[i].op2)) != -1){
						insert_sYY(reg_n, op[i].address);
						insert_instruction(output_y_to_x_id,op[i].address);
					} else if(((reg_n = htoi(op[i].op2)) != -1) && (reg_n < PROGRAM_COUNT)){
						insert_constant(reg_n, op[i].address);
						insert_instruction(output_p_to_x_id,op[i].address);
					} else {
						printf("ERROR - Invalid operand %s on line %d\n",op[i].op2, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op2, i+1);
						error++;
					}
					break;
				case 23: /* RETURNI */
					insert_instruction(returni_id, op[i].address);
					if(!strcmp(op[i].op1, "ENABLE"))
						program_word[op[i].address]++;
					break;
				case 24: /* ENABLE */
				case 25: /* DISABLE */
					insert_instruction(interrupt_id, op[i].address);
					if(j == 24) /* ENABLE */
						program_word[op[i].address]++;
					break;
				case 26: /* CONSTANT */
				case 27: /* NAMEREG */
				case 28: /* ADDRESS */
					break;
				case 29: /* FLIP */ /* added new instruction */
					insert_instruction(flip_id, op[i].address);
					if((reg_n = find_namereg(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else if((reg_n = register_number(op[i].op1)) != -1)
						insert_sXX(reg_n, op[i].address);
					else {
						printf("ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						fprintf(ofp,"ERROR - Invalid operand %s on line %d\n",op[i].op1, i+1);
						error++;
					}
					break;
			}
		}
	}
}
/*====================================== */
/* write program word in hex format */
void write_hex(void)
{
    int i;

	for(i = 0; i < PROGRAM_COUNT; i++){
		fprintf(stdout, "%3d : %04X\n", i, program_word[i]);
	}

}

/*====================================== */
/* write program word in hex format */
void write_c(void)
{
    int i, k;

	for(i = 0; i < PROGRAM_COUNT/10; i++){
        for (k=0; k<10; k++)
		    fprintf(stdout, "0x%04X, ", program_word[i*10+k]);
        fprintf(stdout, "\n");
    }

    for (k=0; k<5; k++)
	    fprintf(stdout, "0x%04X, ", program_word[250+k]);
    fprintf(stdout, "\n");
}

/*====================================== */
/* write vhdl module for simulation */
void write_bin(void)
{
	int i, j;

	for(i = 0; i < PROGRAM_COUNT-1; i++){
        fprintf(stdout, "%3d : ", i);
		for(j = 15; j >= 0; j--)
			fprintf(stdout, "%d", (program_word[i]>>j) & 1); //print binary
		fprintf(stdout, "\n");
	}

    fprintf(stdout, "%3d : ", i);
	for(j = 15; j >= 0; j--)
		fprintf(stdout, "%d", (program_word[i]>>j) & 1); //print binary
	fprintf(stdout, "\n");

}

/*====================================== */
int main(int argc, char **argv)
{
	char *ptr;

    if(argc != 3){
		printf("\nCommand line syntax:\n\nasm file.asm (b | h | c)\n");
		exit(1);
	}

	strcpy(filename, argv[1]);
	ptr = strstr(filename, ".asm");
	if (ptr == NULL){
		printf("\nInvalid file type, use .asm extension\n");
		exit(1);
	}
	*ptr = '\0';
	strcat(filename,".log");

    ifp = fopen(argv[1], "r");
    if (ifp == NULL){
        printf("\nCan not open input file\n");
		exit(1);
	}

	while ( fgets(linebuf, 128, ifp) != NULL ) {
		if(line_count >= MAX_LINE_COUNT){
			printf("\nInput exceed maximum line number\n");
			error_out();
		}
		parse_linebuf();
		line_count++;
	}

	init_program_word();
	test_instructions();
	write_program_word();


    /* EDIT. FM. MSEM */
    if (*argv[2] == 'h') write_hex();
    if (*argv[2] == 'b') write_bin();
    if (*argv[2] == 'c') write_c();

	free_mem();
	fclose(ifp);
	return(0);
}

