#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "symtab.h"


extern FILE *output;
int free_reg_count = LAST_WORKING_REG + 1;
char invalid_value[] = "???";
int function_stack_change = 4 * (1 + 1 + 11 + 8);

int regs[LAST_WORKING_REG];

int arg_index = -1;


// FUNCTIONS

/*
Stack layout:
        ???              <- fp
----------------------- 
return address           
-----------------------
frame pointer
-----------------------
s1
-----------------------
...
-----------------------
s11
-----------------------
a0
-----------------------
....
-----------------------
a7
-----------------------
local_1
-----------------------
...
-----------------------
local_n                 <- sp
-----------------------
*/
void gen_function_prologue() {
  int index = 0;
  // Take space on stack
  take_stack(1 + 1 + 11 + 8);
  // Save return address
  code("\n\tsw\tra, %d(sp)", function_stack_change - (++index)*4);
  // Save frame pointer
  code("\n\tsw\tfp, %d(sp)", function_stack_change - (++index)*4);
  // Save saved registers
  int i;
  for (i = 1; i < 12; i++) {
    code("\n\tsw\ts%d, %d(sp)", i, function_stack_change - (++index)*4);
  }
  // Save argument registers
  for (i = 0; i < 8; i++) {
    code("\n\tsw\ta%d, %d(sp)", i, function_stack_change - (++index)*4);
  }
  // Move frame pointer on top of the stack frame
  code("\n\taddi fp, sp, %d", function_stack_change);
}

void gen_function_epilogoue() {
  // Restore return address for the caller
  code("\n\tlw\tra, -4(fp)");
  // Restore saved registers for the caller
  int i;
  for (i = 1; i < 12; i++) {
    code("\n\tlw\ts%d, -%d(fp)", i, 8 + i*4);
  }
  // Restore argument registers
  // for (i = 0; i < 8; i++) {
  //   code("\n\tlw\ta%d, -%d(fp)", i, 8 + 11*4 + i*4);
  // }
  // Restore the caller's stack pointer
  code("\n\tmv\tsp, fp");
  // Restores caller's frame pointer
  code("\n\tlw\tfp, -8(fp)");
  // Return the controll back to the caller
  code("\n\tret");
}

// REGISTERS

int take_reg(void) {
  if(free_reg_count == 0) {
    err("Compiler error! No free registers!");
    exit(EXIT_FAILURE);
  }
  int i;
  for (i = 0; i < LAST_WORKING_REG; i++) {
    if (regs[i] == 0) {
      regs[i] = 1;
      free_reg_count--;
      return i;
    }
  }
}

// Ako je u pitanju indeks registra, oslobodi registar
void free_if_reg(int reg_index) {
  if (reg_index >= 0 && reg_index <= LAST_WORKING_REG) {
    if (free_reg_count == LAST_WORKING_REG + 1) {
      err("compiler error: no more regs to free");
      exit(EXIT_FAILURE);
    }
    free_reg_count++;
    regs[reg_index] = 0;
    set_type(reg_index, NO_TYPE);
  }
}

// SYMBOL
int gen_sym_name(int index) {
  if(index > -1) {
    if(get_kind(index) == VAR || get_kind(index) == PARA_ITER) { // -n*4(%14)
      int offset = function_stack_change + (get_atr1(index)) * 4;
      code("-%d(fp)", offset);
    } else {
      if(get_kind(index) == PAR) {
        int parameter_index = get_atr1(index);
        int offset;
        if (parameter_index <= 8) {
          offset = -(1 + 1 + 11 + parameter_index) * 4;
        } else {
          offset = 4 * (parameter_index - 9);
        }
        code("%d(fp)", offset);
      } else if (get_kind(index) == GVAR) {
        code("%d(gp)", 4 * get_atr1(index));
      } else {
        if(get_kind(index) == LIT) {
          debug("error: this shouldn't have happened...");
        } else {  //function, reg
          code("%s", get_name(index));
        }
      }
    }
  }
  return -1;
}

// OTHER

void gen_cmp(int op1_index, int op2_index) {
  if(get_type(op1_index) == INT)
    code("\n\t\tCMPS \t");
  else
    code("\n\t\tCMPU \t");
  gen_sym_name(op1_index);
  code(",");
  gen_sym_name(op2_index);
  free_if_reg(op2_index);
  free_if_reg(op1_index);
}

void gen_mov(int input_index, int output_index) {
  if (input_index == output_index) {
    return;
  }
  int input_reg = gen_load(input_index);
  int output_reg = gen_load(output_index);
  code("\n\tmv  ");
  gen_sym_name(output_reg);
  code(", ");
  gen_sym_name(input_reg);

  //ako se smešta u registar, treba preneti tip 
  if(output_reg >= 0 && output_reg <= LAST_WORKING_REG)
    set_type(output_reg, get_type(input_index));
  gen_store(output_reg, output_index);
  free_if_reg(input_reg);
}

int post_inc_count_map[POST_INC_COUNT_MAP_LENGTH];

int get_increment_count(int idx) {
  return post_inc_count_map[idx];
}

void reset_post_increment_count() {
  int i;
  for (i=0;i<POST_INC_COUNT_MAP_LENGTH;i++) {
    post_inc_count_map[i] = 0;
  }
}

void apply_post_increment(int left_idx) {
  int i;
  for (i=0;i<POST_INC_COUNT_MAP_LENGTH;i++) {
    int count = post_inc_count_map[i];
    if (count != 0 && i != left_idx) {
      int j;
      for (j=0;j<count;j++) {
        gen_post_inc(i);
      }
    }
  }
  reset_post_increment_count();
}

void increment_post_inc_count(int idx) {
  post_inc_count_map[idx] += 1;
}

void gen_post_inc(int operand_index) {
  int input_reg = gen_load(operand_index);
  code("\n\taddi %s, %s, 1", get_name(input_reg), get_name(input_reg));
  gen_mov(input_reg, operand_index);
}

void gen_literal_cmp(int operand_index, int literal) {
  if (get_type(operand_index) == INT) {
    code("\n\t\tCMPS \t");
  } else {
    code("\n\t\tCMPU \t");
  }
  gen_sym_name(operand_index);
  code(",$%d", literal);
  free_if_reg(operand_index);
}

int gen_arithmetic_instruction(int operation, int left_index, int right_index) {
  int result_reg = take_reg();
  int left_reg = gen_load(left_index);
  int right_reg = gen_load(right_index);
  if (operation == ADD) {
    code("\n\tadd\t");
  } else {
    code("\n\tsub\t");
  }
  gen_sym_name(result_reg);
  code(", ");
  gen_sym_name(left_reg);
  code(", ");
  gen_sym_name(right_reg);
  free_if_reg(left_reg);
  free_if_reg(right_reg);
  return result_reg;
}

int gen_load(int operand_index) {
  if(operand_index > -1) {
    if(get_kind(operand_index) == VAR || get_kind(operand_index) == PARA_ITER) {
      int reg = take_reg();
      int offset = function_stack_change + (get_atr1(operand_index)) * 4;
      code("\n\tlw\t%s, -%d(fp)", get_name(reg), offset);
      set_type(reg, get_type(operand_index));
      return reg;
    } else if (get_kind(operand_index) == PAR) {
      int reg = take_reg();
      int par_num = get_atr1(operand_index);
      int offset;
      if (par_num <= 8) {
        offset = -(1 + 1 + 11 + par_num) * 4;
      } else {
        offset = 4 * (par_num - 9);
      }
      code("\n\tlw\t%s, %d(fp)", get_name(reg), offset);
      set_type(reg, get_type(operand_index));
      return reg;
    } else if (get_kind(operand_index) == LIT) {
      int reg = take_reg();
      code("\n\tli\t%s, %d", get_name(reg), atoi(get_name(operand_index)));
      set_type(reg, get_type(operand_index));
      return reg;
    } else if (get_kind(operand_index) == GVAR) {
      int reg = take_reg();
      code("\n\tlw\t%s, %d(gp)", get_name(reg), 4 * get_atr1(operand_index));
      set_type(reg, get_type(operand_index));
      return reg;
    }
  }
  return operand_index;
}

void gen_store(int register_index, int memory_index) {
  if (get_kind(memory_index) != VAR && get_kind(memory_index) != PARA_ITER && get_kind(memory_index) != PAR && get_kind(memory_index) != GVAR) {
    return;
  }
  code("\n\tsw\t");
  gen_sym_name(register_index);
  code(", ");
  gen_sym_name(memory_index);
  free_if_reg(register_index);
}

void take_stack(int var_num) {
  if (var_num > 0) {
    code("\n\taddi sp, sp, -%d", 4 * var_num);
  }
}

void free_stack(int var_num) {
  if (var_num > 0) {
    code("\n\taddi sp, sp, %d", 4 * var_num);
  }
}

void gen_start() {
  code("\n_start:");
  gen_fp_init();
  code("\n\tjal main");
  code("\n\tnop");
}

void gen_fp_init() {
  // code("\n.text");
  // code("\n_start:");
  code("\n\tmv\tfp, sp");  // initialize a frame pointer
  // code("\n\tj\tmain");     // jump to main
}

void prepare_args_stack(int fcall_idx) {
  take_stack(get_atr1(fcall_idx) - 8);
}

void gen_func_call(int fun_idx, int fcall_idx) {
  code("\n\tjal\t%s", get_name(fcall_idx));
  free_stack(get_atr1(fcall_idx) - 9);
  arg_index = -1;
  int caller_args = get_atr1(fun_idx);
  int calle_args = get_atr1(fcall_idx);
  // Basically no argument register will be overwritten
  if (calle_args == 0 || caller_args == 0) {
    return;
  }
  int args_to_save = calle_args < caller_args ? calle_args : caller_args; // min of those two
  // Here it is easier to refference based on the stack pointer, since this will be stored after the local variables
  // int i;
  // for (i = 0; i < args_to_save; i++) {
  //   code("\n\tlw\ta%d, %d(sp)", i, i*4);
  // }
  // free_stack(args_to_save);
}

void add_arg(int operand_index) {
  ++arg_index;
  if (arg_index >= 8) {
    int temp_reg = gen_load(operand_index);
    code("\n\tsw %s, %d(sp)", get_name(temp_reg), 4*(arg_index - 8));
  } else {
    gen_mov(operand_index, LAST_WORKING_REG + 1 + arg_index);
  }
}

void gen_fun_call_save_args_on_stack(int fun_idx, int fcall_idx) {
  int caller_args = get_atr1(fun_idx);
  int calle_args = get_atr1(fcall_idx);
  // Basically no argument register will be overwritten
  if (calle_args == 0 || caller_args == 0) {
    return;
  }
  int args_to_save = calle_args < caller_args ? calle_args : caller_args; // min of those two
  take_stack(args_to_save);
  // Here it is easier to refference based on the stack pointer, since this will be stored after the local variables
  int i;
  for (i = 0; i < args_to_save; i++) {
    code("\n\tsw\ta%d, %d(sp)", i, i*4);
  }
}

void gen_opposite_branch(s_relop *relop, int lab_num) {
  int left_index = gen_load(relop->left_index);
  int right_index = gen_load(relop->right_index);
  code("\n\t%s\t%s, %s, .false%d", riscv_opp_branches[relop->operator], get_name(left_index), get_name(right_index), lab_num);
  free_if_reg(left_index);
  free_if_reg(right_index);
}

void gen_true_branch(s_relop *relop, int lab_num) {
  int left_index = gen_load(relop->left_index);
  int right_index = gen_load(relop->right_index);
  code("\n\t%s\t%s, %s, .true%d", riscv_branches[relop->operator], get_name(left_index), get_name(right_index), lab_num);
  free_if_reg(left_index);
  free_if_reg(right_index);
}

void gen_para_check(int para_iter_index, int upper_bound_index, int para_num) {
  int left_index = gen_load(para_iter_index);
  int right_index = gen_load(upper_bound_index);
  if (get_type(para_iter_index) == INT) {
    code("\n\tbgt\t%s, %s, .para%d_exit", get_name(left_index), get_name(right_index), para_num);
  } else {
    code("\n\tbgtu\t%s, %s, .para%d_exit", get_name(left_index), get_name(right_index), para_num);
  }
  free_if_reg(left_index);
  free_if_reg(right_index);
}

void gen_branch_branches(int branch_var_index, int first_index, int second_index, int third_index, int branch_num) {
  code("\n.branch%d:", branch_num);
  int branch_reg = gen_load(branch_var_index);
  int temp_reg = gen_load(first_index);
  code("\n\tbeq\t%s, %s, .branch%d_first", get_name(branch_reg), get_name(temp_reg),  branch_num);
  free_if_reg(temp_reg);
  temp_reg = gen_load(second_index);
  code("\n\tbeq\t%s, %s, .branch%d_second", get_name(branch_reg), get_name(temp_reg),  branch_num);
  free_if_reg(temp_reg);
  temp_reg = gen_load(third_index);
  code("\n\tbeq\t%s, %s, .branch%d_third", get_name(branch_reg), get_name(temp_reg),  branch_num);
  free_if_reg(temp_reg);
  free_if_reg(branch_reg);
  code("\n\tj \t.branch%d_otherwise", branch_num);
}