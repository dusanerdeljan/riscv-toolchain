#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

#define TRUE  1
#define FALSE 0

#define RISCV_SIM_DEBUG 0

//8, 32 i 64-bitni tipovi podataka
typedef unsigned char uchar;
typedef int32_t word;
typedef uint32_t uword;
typedef int64_t quad;
typedef uint64_t uquad;

//tipovi naredbi
enum { NO_TYPE = 0, SIGNED_TYPE, UNSIGNED_TYPE } sign_type;

//vrste operanada
enum { OP_REGISTER, OP_IMMEDIATE, OP_REGISTER_OFFSET, OP_ADDRESS } operand_type;

//instrukcije
enum { INS_JAL, INS_RET, INS_J, INS_BGE, INS_BLE, INS_BGT, INS_BLT, INS_BEQ, INS_BNE, INS_ADD, INS_ADDI, INS_SUB, INS_MV, INS_LW, INS_SW, INS_LI, INS_NOP } ins_type;

#define RV32I_REG_NUM            32
#define SYMTAB_LENGTH            64

#define NO_ADDRESS               -123

#define FUNCTION_REGISTER        10
#define FRAME_POINTER            8
#define STACK_POINTER            2
#define GLOBAL_POINTER           3
#define RETURN_ADDRESS_REG       1

#define PRINT_SRCLINES           10
#define PRINT_STKLINES           10

#define STACK_SEGMENT_LENGTH     500
#define SECTION_DATA_LENGTH      20
#define SECTION_TEXT_LENGTH      2000 // :D

#define TEXT_SEGMENT_START       (0x10000)
#define STATIC_DATA_START        (0x10000000)
#define STACK_SEGMENT_START      (0xbffffff0)

//dužina pomoćnog bafera za ispis
#define CHAR_BUFFER_LENGTH       256

static char *abi_regs[] = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "fp", "s1", "a0", "a1", "a2", "a3",
        "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
        "t3", "t4", "t5", "t6"
};

extern char char_buffer[CHAR_BUFFER_LENGTH];
extern int yyerror(char *s);
extern void warning(char *s);

//kodovi grešaka
enum { NO_ERROR = 0, PARSE_ERROR, ARG_ERROR, SIM_ERROR, STEP_ERROR };

//pomoćni makroi za ispis
#define parsererror(args...) sprintf(char_buffer, args), yyerror(char_buffer), exit(PARSE_ERROR)
#define argerror(args...) cprintf("\n{RED}Argument error:{NRM} "), printf(args), printf("\n"), exit(ARG_ERROR)
#define simerror(args...) cprintf("\n{RED}Simulation error:{NRM} "), printf(args), printf("\n"), exit(SIM_ERROR)

#if RISCV_SIM_DEBUG
        #define debug(args...) printf(args), printf("\n")
#else
        #define debug(args...)
#endif

#endif

