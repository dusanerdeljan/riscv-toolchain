#ifndef DEFS_H
#define DEFS_H

#define RISCV_DEBUG 0

#define bool int
#define TRUE  1
#define FALSE 0

#define SYMBOL_TABLE_LENGTH   100
#define NO_INDEX              -1
#define NO_ATR                 0
#define LAST_WORKING_REG      17
#define FUN_REG               26
#define CHAR_BUFFER_LENGTH   128
#define POST_INC_COUNT_MAP_LENGTH 64
extern char char_buffer[CHAR_BUFFER_LENGTH];

extern int out_lin;
//pomocni makroi za ispis
extern void warning(char *s);
extern int yyerror(char *s);
#define err(args...)  sprintf(char_buffer, args), yyerror(char_buffer)
#define warn(args...) sprintf(char_buffer, args), warning(char_buffer)
#define code(args...) ({fprintf(output, args); \
          if (++out_lin > 2000) err("Too many output lines"), exit(1); })

#if RISCV_DEBUG
    #define debug(args...) printf(args), printf("\n")
#else
    #define debug(args...) 
#endif

//tipovi podataka
enum types { NO_TYPE, INT, UINT, VOID };

//vrste simbola (moze ih biti maksimalno 32)
enum kinds { NO_KIND = 0x1, REG = 0x2, LIT = 0x4, 
             FUN = 0x8, VAR = 0x10, PAR = 0x20, GVAR = 0x40, PARA_ITER = 0x80};

//logicki operatori
enum logops { AND, OR };

//konstante arithmetickih operatora
enum arops { ADD, SUB, MUL, DIV, AROP_NUMBER };

//stringovi za generisanje aritmetickih naredbi
static char *ar_instructions[] = { "ADDS", "SUBS", "MULS", "DIVS",
                                   "ADDU", "SUBU", "MULU", "DIVU" };

//stringovi za JMP narebu
static char* jumps[]={"JLTS", "JGTS", "JLES", "JGES", "JEQ ", "JNE ",
                      "JLTU", "JGTU", "JLEU", "JGEU", "JEQ ", "JNE " };

static char* opp_jumps[]={"JGES", "JLES", "JGTS", "JLTS", "JNE ", "JEQ ",
                          "JGEU", "JLEU", "JGTU", "JLTU", "JNE ", "JEQ "};

//konstante relacionih operatora
enum relops { LT, GT, LE, GE, EQ, NE, RELOP_NUMBER };

//RISC-V branching pseudoinstructions
static char *riscv_branches[]= {"blt", "bgt", "ble", "bge", "beq ", "bne ",
                               "bltu", "bgtu", "bleu", "bgeu", "beq ", "bne " };

//RISC-V opposite branching pseudoinstructions
static char *riscv_opp_branches[]= {"bge", "ble", "bgt", "blt", "bne ", "beq ",
                                         "bgeu", "bleu", "bgtu", "bltu", "bne ", "beq "};

//RISC-V callee saved registers
static char *riscv_s_registers[] = {"s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"};

//RISC-V temp registers
static char *riscv_t_registers[] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};

//RISC-V argument registers (a0 is used for the return value as well)
static char *riscv_a_registers[] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};

//RISC-V integer arithmetic pseudoinstructions
static char *riscv_ar_instructions[] = {};

#endif
