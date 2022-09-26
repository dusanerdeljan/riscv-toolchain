%{

#define YYDEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h> //basename
#include <unistd.h> //isatty
#include "defs.h"
#include "riscv_simulator.h"

int yyparse(void);
int yylex(void);
int yyerror(char *s);
void warning(char *s);

extern int yylineno;
extern int max_steps;
char char_buffer[CHAR_BUFFER_LENGTH];
int error_count = 0;
int mainarg = 0;

%}

%union {
    long i;
    char* s;
}

%token _DATA
%token _TEXT
%token _WORD

%token _JAL
%token _J
%token _RET

%token <i> _BGE
%token <i> _BLE
%token <i> _BGT
%token <i> _BLT
%token <i> _BEQ
%token <i> _BNE

%token _ADD
%token _ADDI
%token _SUB
%token _MV

%token _LW
%token _SW
%token _LI

%token _NOP

%token <i> _NUMBER
%token <i> _REGISTER
%token <s> _LABEL_DEF
%token <s> _LABEL

%token _COMMA
%token _LPAREN
%token _RPAREN

%%

program
    : section_data section_text
    ;

section_data
    : _DATA variable_list
    ;

variable_list
    : /* empty */
    | variable_list variable
    ;

variable
    : _LABEL_DEF _WORD _NUMBER
    {
        insert_global($1);
        insert_data($3);
    }
    ;

section_text
    : _TEXT code_list
    ;

code_list
    :   code_line
    |   code_list code_line
    ;

code_line
    :   label_def
    |   asm_line
    ;

label_def
    : _LABEL_DEF
    {
        //debug("inserting label %s ...", $1);
        insert_source("%s:", $1);
        //debug("inserted label in source");
        insert_label($1);
    }
    ;

asm_line
    : jump_ins
    | branch_ins
    | load_store_ins
    | arithmetic_ins
    ;

jump_ins
    : jal_ins
    | j_ins
    | ret_ins
    ;

jal_ins
    : _JAL _LABEL
    {
        insert_source("\t\t\tjal %s", $2);
        insert_jump(INS_JAL, $2);
    }
    ;

j_ins
    : _J _LABEL
    {
        insert_source("\t\t\tj %s", $2);
        insert_jump(INS_J, $2);
    }
    ;

ret_ins
    : _RET
    {
        insert_source("\t\t\tret");
        insert_jump(INS_RET, NULL);
    }
    ;

branch_ins
    : bge_ins
    | ble_ins
    | bgt_ins
    | blt_ins
    | beq_ins
    | bne_ins
    ;

bge_ins
    : _BGE _REGISTER _COMMA _REGISTER _COMMA _LABEL
    {
        insert_source("\t\t\tbge%c %s, %s, %s", type_char($1), abi_regs[$2], abi_regs[$4], $6);
        insert_branch(INS_BGE, $1, $2, $4, $6);
    }
    ;

ble_ins
    : _BLE _REGISTER _COMMA _REGISTER _COMMA _LABEL
    {
        insert_source("\t\t\tble%c %s, %s, %s", type_char($1), abi_regs[$2], abi_regs[$4], $6);
        insert_branch(INS_BLE, $1, $2, $4, $6);
    }
    ;

bgt_ins
    : _BGT _REGISTER _COMMA _REGISTER _COMMA _LABEL
    {
        insert_source("\t\t\tbgt%c %s, %s, %s", type_char($1), abi_regs[$2], abi_regs[$4], $6);
        insert_branch(INS_BGT, $1, $2, $4, $6);
    }
    ;

blt_ins
    : _BLT _REGISTER _COMMA _REGISTER _COMMA _LABEL
    {
        insert_source("\t\t\tblt%c %s, %s, %s", type_char($1), abi_regs[$2], abi_regs[$4], $6);
        insert_branch(INS_BLT, $1, $2, $4, $6);
    }
    ;

beq_ins
    : _BEQ _REGISTER _COMMA _REGISTER _COMMA _LABEL
    {
        insert_source("\t\t\tbeq %s, %s, %s", abi_regs[$2], abi_regs[$4], $6);
        insert_branch(INS_BEQ, $1, $2, $4, $6);
    }
    ;

bne_ins
    : _BNE _REGISTER _COMMA _REGISTER _COMMA _LABEL
    {
        insert_source("\t\t\tbne %s, %s, %s", abi_regs[$2], abi_regs[$4], $6);
        insert_branch(INS_BNE, $1, $2, $4, $6);
    }
    ;

load_store_ins
    : load_ins
    | store_ins
    | load_imm_ins
    ;

load_ins
    : _LW _REGISTER _COMMA _NUMBER _LPAREN _REGISTER _RPAREN
    {
        insert_source("\t\t\tlw %s, %ld(%s)", abi_regs[$2], $4, abi_regs[$6]);
        insert_load_store(INS_LW, $2, $4, $6);
    }
    ;

store_ins
    : _SW _REGISTER _COMMA _NUMBER _LPAREN _REGISTER _RPAREN
    {
        insert_source("\t\t\tsw %s, %ld(%s)", abi_regs[$2], $4, abi_regs[$6]);
        insert_load_store(INS_SW, $2, $4, $6);
    }
    ;

load_imm_ins
    : _LI _REGISTER _COMMA _NUMBER
    {
        insert_source("\t\t\tli %s, %ld", abi_regs[$2], $4);
        insert_load_store(INS_LI, $2, $4, 88);
    }
    ;

arithmetic_ins
    : add_ins
    | addi_ins
    | sub_ins
    | mv_ins
    | _NOP
    {
        insert_source("\t\t\tnop");
        insert_nop();
    }
    ;

add_ins
    : _ADD _REGISTER _COMMA _REGISTER _COMMA _REGISTER
    {
        insert_source("\t\t\tadd %s, %s, %s", abi_regs[$2], abi_regs[$4], abi_regs[$6]);
        insert_arithmetic(INS_ADD, $2, $4, $6);
    }
    ;

addi_ins
    : _ADDI _REGISTER _COMMA _REGISTER _COMMA _NUMBER
    {
        insert_source("\t\t\taddi %s, %s, %ld", abi_regs[$2], abi_regs[$4], $6);
        insert_arithmetic_immediate(INS_ADDI, $2, $4, $6);
    }
    ;

sub_ins
    : _SUB _REGISTER _COMMA _REGISTER _COMMA _REGISTER
    {
        insert_source("\t\t\tsub %s, %s, %s", abi_regs[$2], abi_regs[$4], abi_regs[$6]);
        insert_arithmetic(INS_SUB, $2, $4, $6);
    }
    ;

mv_ins
    : _MV _REGISTER _COMMA _REGISTER
    {
        insert_source("\t\t\tmv %s, %s", abi_regs[$2], abi_regs[$4]);
        insert_arithmetic_immediate(INS_MV, $2, $4, 0);
    }
    ;

%%

int yyerror(char *s) {
    fprintf(stderr, "\nSimulator: ASM parsing error in line %d: %s\n", yylineno, s);
    error_count++;
    return 0;
}

int main(int argc, char *argv[]) {
    int run_complete = FALSE;
    while (1) {
        char c = getopt(argc, argv, ":hrs:");
        if (c == -1) break;
        switch(c) {
            case 'h' : {
                    cprintf("\n{BLU}RISC-V RV32I Simulator{NRM} v0.1");
                    cprintf("\n\nUsage: {BLU}%s{NRM} [options] {BLU}< asm_file{NRM}", basename(strdup(argv[0])));
                    cprintf("\nIf started without options, simulator will run asm code");
                    cprintf("\nstep by step. Possible options are:");
                    cprintf("\n{GRN}-h{NRM}     - this help");
                    cprintf("\n{GRN}-r{NRM}     - complete run of the program, only exit code (%%%d) output",FUNCTION_REGISTER);
                    cprintf("\n{GRN}-s NUM{NRM} - maximal number of execution steps for complete run");
                    cprintf("\n         (simulator will return code %d if this number is reached)\n\n",STEP_ERROR);
                    exit(0);
                    break; }
            case 'r' : {
                    run_complete = TRUE;
                    break; }
            case 's' : {
                    max_steps = atoi(optarg);
                    break; }
            case '?' : {
                    argerror("Unknown option %c",optopt);
                    break; }
            case ':' : {
                    argerror("Argument missing for option %c",optopt);
                    break; }
            default : {
                    argerror("Unknown getopt return code 0x%X",c);
                    break; }
        }
    }

    //proveri da li postoji ulazni fajl
    if (isatty(fileno(stdin))) {
        argerror("No input file was specified.");
    }

    init_simulator();
    yyparse();

    //preusmeravanje terminala na stdin
    freopen("/dev/tty", "rw", stdin);

    if (error_count) {
        if (!run_complete)
            cprintf("\n{RED}There were error(s) in ASM source.{NRM}", error_count);
        printf("\n");
        exit(PARSE_ERROR);
    } else {
        if (run_complete) {
            word ret_val = run_simulator();
            if (max_steps != 0) {
                printf("%d", ret_val);
            } else
                cprintf("\n{RED}Program terminated.{NRM}");
        } else {
            run_interactive();
        }
    }
    printf("\n");
    if (error_count)
        return PARSE_ERROR;
    else if (max_steps != 0)
        return NO_ERROR;
    else
        //izvršeno max_steps koraka, a program se nije završio
        return STEP_ERROR;
    return 0;
}