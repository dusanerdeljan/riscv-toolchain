#ifndef RISCV_SIMULATOR_H
#define RISCV_SIMULATOR_H

#include "defs.h"

/*******************
* Structures
*******************/

typedef struct _operand {
    uchar operand_type;
    uchar register_index;
    word data;
} s_operand;

typedef struct _instruction {
    uchar instruction_type;
    uchar sign_type;
    s_operand destination;
    s_operand source1;
    s_operand source2;
} s_instruction;

typedef struct _processor {
    word pc;
    word regs[RV32I_REG_NUM];
    uchar done;
} s_processor;

typedef struct _symbol {
    char *name;
    int offset;
    uchar defined;
} s_symbol;

typedef struct _source {
    char *text;
    int address;
} s_source;

/*******************
* Macros
*******************/

#define GENERATE_BRANCH(compare) {\
    if (ins->sign_type == SIGNED_TYPE) {\
        word rs1_val = *get_reg(ins->source1.register_index);\
        word rs2_val = *get_reg(ins->source2.register_index);\
        if (rs1_val compare rs2_val) {\
            processor.pc = get_label_address(ins->destination.data);\
        } else {\
            processor.pc++;\
        }\
    } else {\
        uword rs1_val = (uword) *get_reg(ins->source1.register_index);\
        uword rs2_val = (uword) *get_reg(ins->source2.register_index);\
        if (rs1_val compare rs2_val) {\
            processor.pc = get_label_address(ins->destination.data);\
        } else {\
            processor.pc++;\
        }\
    }\
}\

// Copy-pase from hipsim :D
//pomoćni makroi za parser
extern char source_buffer[];
//makro za ubacivanje linije koda u source za ispis
#define insert_source(args...) sprintf(source_buffer, args), insert_source_f(source_buffer)

/*******************
* Functions
*******************/

// read the value stored in the register
word *get_reg(uchar reg);

// read the data from memory (global or stack segment) based on the input register
word *get_memory(uchar reg, word offset);

// get label address
int get_label_address(word label_index);

// ensures that the label is crated, returns index of the label in the symbol table
int ensure_label(char *name);

// insert label definition
void insert_label(char *name);

// insert unconditional jump instruction in section text
void insert_jump(uchar ins_type, char *name);

// insert branch instruction in section text
void insert_branch(uchar ins_type, uchar sign_type, uchar rs1, uchar rs2, char *name);

// insers load & store instruction in section text
void insert_load_store(uchar ins_type, uchar rd, word offset, uchar rs);

// insert arithmetic instruction in section text
void insert_arithmetic(uchar ins_type, uchar rd, uchar rs1, uchar rs2);

// insert arithmetic immediate instruction in section text
void insert_arithmetic_immediate(uchar ins_type, uchar rd, uchar rs1, word immediate);

// inserts nop instruction
void insert_nop();

// insert global in section data
void insert_data(word data);

// insert global symbol 
void insert_global(char *name);

// create register operand
s_operand create_reg_operand(uchar reg);

// create immediate operand
s_operand create_imm_operand(word imm);

// create register offset operand
s_operand create_reg_offset_operand(word offset, uchar reg);

// create address operand
s_operand create_address_operand(char *name);

// insert instruction in section text
void insert_instruction(s_instruction *ins);

// initializes simulator
void init_simulator();

// executes one instruction
void step();

// runs simulation and returns exit code from main
word run_simulator();

// check if there are any labels which are not defined
void check_undefined_labels();

// return u extension for unsigned instructions or no extension for the signed ones
char type_char(int sign_type);

// insert source code
void insert_source_f(char *source);

// runs simulator in the interactive mode
word run_interactive();

// prints register values
void print_registers();

// prints global segment
void print_global_segment();

// prints stack segment
void print_stack_segment();

// prints code segment
void print_code_segment();

//pomoćne funkcije
int getch(void);
int string_replace(char** String, char* ReplaceWhat, char* ReplaceTo);
int cprintf(const char *format, ...);

#endif