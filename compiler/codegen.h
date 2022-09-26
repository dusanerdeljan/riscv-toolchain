#ifndef CODEGEN_H
#define CODEGEN_H

#include "defs.h"

typedef struct _relop {
    int left_index;
    int operator;
    int right_index;
} s_relop;

// funkcije za zauzimanje, oslobadjanje registra
int  take_reg(void);
// oslobadja ako jeste indeks registra
void free_if_reg(int reg_index); 

// ispisuje simbol (u odgovarajucem obliku) 
// koji se nalazi na datom indeksu u tabeli simbola
// return taken register if any
int gen_sym_name(int index);

// generise CMP naredbu, parametri su indeksi operanada u TS-a 
void gen_cmp(int operand1_index, int operand2_index);

// generise MOV naredbu, parametri su indeksi operanada u TS-a 
void gen_mov(int input_index, int output_index);

// dobavlja koliko puta je neki operand uvecan u num exp
int get_increment_count(int idx);

// resetuje broj uvecavanja sa postinc
void reset_post_increment_count();

// primenjuje post inc na sve elemente koji su uvecani, osim na onaj ciji je indeks prosledjen (onaj kojem se dodeljuje vrednost u assign_stmt)
void apply_post_increment(int left_idx);

// oznacava da je na element sa prosledjenim indeksom primenjen postinc
void increment_post_inc_count(int idx);

// generise post increment naredbu
void gen_post_inc(int operand_index);

// generise CMP naredbu, drugi parametar je vrednost literala
void gen_literal_cmp(int operand_index, int literal);

// generates a function prologoue which sets up the stack pointer and saves all the required registers on the stack
void gen_function_prologue();

// generates a function epligoue which clears up the stack and restores the saved registers
void gen_function_epilogoue();

// generates arithmetic instructions, return taken register
int gen_arithmetic_instruction(int operation, int left_index, int right_index);

// loads operand in the first available register and returns taken register
int gen_load(int operand_index);

// stores value from register in memory
void gen_store(int reg, int memory);

// takes stapce on stack
void take_stack(int var_num);

// frees up the stack
void free_stack(int var_num);

// generates startup code which calls the main function and initializes the frame pointer
void gen_start();

// generates frame pointer initialization code
void gen_fp_init();

// substracts the stack pointer if the number of arguments is greater than 8 so that remaining args can go on stack
void prepare_args_stack(int fcall_idx);

// generates function call
void gen_func_call(int fun_idx, int fcall_idx);

// moves argument in the first available argument register
void add_arg(int operand_index);

// saves argument registers on stack if needed
void gen_fun_call_save_args_on_stack(int fun_idx, int fcall_idx);

// generates opposite branch
void gen_opposite_branch(s_relop *relop, int lab_num);

// generates true branch
void gen_true_branch(s_relop *relop, int lab_num);

// generates para check condition
void gen_para_check(int para_iter_index, int upper_bound_index, int para_num);

// generates required labels and branches for branch statement
void gen_branch_branches(int branch_var_index, int first_index, int second_index, int third_index, int branch_num);

#endif
