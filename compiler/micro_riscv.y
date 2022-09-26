%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "defs.h"
  #include "symtab.h"
  #include "codegen.h"
  #include "func_param_map.h"

  int yyparse(void);
  int yylex(void);
  int yyerror(char *s);
  void warning(char *s);

  extern int yylineno;
  int out_lin = 0;
  char char_buffer[CHAR_BUFFER_LENGTH];
  int error_count = 0;
  int warning_count = 0;
  int var_num = 0;
  int fun_idx = -1;
  int fcall_idx = -1;
  int lab_num = -1;
  int gvar_num = -1;

  unsigned current_variable_type = NO_TYPE;
  unsigned has_return_statement = 0;
  unsigned para_type = NO_TYPE;
  int num_parameters = 0;
  int num_args = 0;
  int block_idx = 1;
  int is_para = 0;
  int para_num = -1;
  int branch_num = -1;

  int initialized_fp = 0;
  int initialized_text_section = 0;

  s_relop current_relop;

  FILE *output;

  void clear_relop(s_relop *r) {
    r->left_index = NO_INDEX;
    r->operator = NO_INDEX;
    r->right_index = NO_INDEX;
  }
%}

%union {
  int i;
  char *s;
}

%token <i> _TYPE
%token _IF
%token _ELSE
%token _RETURN
%token <s> _ID
%token <s> _INT_NUMBER
%token <s> _UINT_NUMBER
%token _LPAREN
%token _RPAREN
%token _LBRACKET
%token _RBRACKET
%token _ASSIGN
%token _SEMICOLON
%token <i> _AROP
%token <i> _RELOP

%token _SELECT
%token _FROM
%token _WHERE
%token _COMMA

%token _POST_INC

%token _PARA
%token _COLON

%token _LSQBRACKET
%token _RSQBRACKET
%token _BRANCH
%token _ARROW
%token _FIRST
%token _SECOND
%token _THIRD
%token _OTHERWISE

%token _WHILE

%token _QUESTION

%token <i> _LOGOP

%type <i> num_exp exp literal
%type <i> function_call rel_exp if_part
%type <i> post_increment argument_list argument arguments para_iter branch_var ternary_exp ternary_exp_op condition label_inc_lparen

%nonassoc ONLY_IF
%nonassoc _ELSE

%%

program
  : 
    {
      code(".data");
    } 
  global_list
  function_list
    {  
      if(lookup_symbol("main", FUN) == NO_INDEX)
        err("undefined reference to 'main'");
    }
  ;


global_list
	: /*empty*/
	| global_list global_var
	;

global_var
	: _TYPE _ID _SEMICOLON
	{
		int idx = lookup_symbol($2, GVAR); 
		if (idx != NO_INDEX) 
		{
				err("redefinition of '%s'", $2);
		} else {
			insert_symbol($2, GVAR, $1, ++gvar_num, NO_ATR);
			code("\n%s:\n\t.word\t0", $2);
		}
	}
	;


function_list
  : function
  | function_list function
  ;

function
  : _TYPE _ID
    {
      if (initialized_text_section == 0) {
        code("\n.text");
        gen_start();
        initialized_text_section = 1;
        initialized_fp = 1;
      }
      fun_idx = lookup_symbol($2, FUN);
      if(fun_idx == NO_INDEX) {
        fun_idx = insert_symbol($2, FUN, $1, NO_ATR, NO_ATR);
        declare_function(fun_idx);

        code("\n%s:", $2);
        gen_function_prologue();
      }
      else 
        err("redefinition of function '%s'", $2);
    }
    _LPAREN parameter_list { set_atr1(fun_idx, num_parameters); } _RPAREN body
    {
      if ((has_return_statement == 0) && (get_type(fun_idx) != VOID)) {
        warn("function '%s' with return type different than void has no return statement", get_name(fun_idx));
      }
      clear_symbols(fun_idx + 1);
      code("\n.%s_exit:", $2);
      free_stack(var_num);
      gen_function_epilogoue();
      var_num = 0;
      has_return_statement = 0;
      num_parameters = 0;
    }
  ;

parameter_list
  :
  | parameters
  ;

parameters
  : parameter
  | parameters _COMMA parameter

parameter
  :  _TYPE _ID
    {
      if ($1 == VOID) {
        err("void can't be used as a variable type");
      }
      // Check if parameter with the same name has already been declared
      if(lookup_symbol($2, PAR) == NO_INDEX) {
        insert_symbol($2, PAR, $1, ++num_parameters, fun_idx);
        debug("num_parameters: %d", num_parameters);
        add_param(fun_idx, num_parameters, $1);
      }
      else {
        err("duplicate parameter names '%s'", $2);
      }
    }
  ;

body
  : _LBRACKET variable_list
    {
      take_stack(var_num);
      code("\n.%s_body:", get_name(fun_idx));
    }
    statement_list _RBRACKET
  ;

variable_list
  : /* empty */
  | variable_list variables
  ;

variables
  : _TYPE
    {
      if ($1 == VOID) {
        err("void can't be used as a variable type");
      }
      current_variable_type = $1;
    } variable_names _SEMICOLON { current_variable_type = NO_TYPE; }
  ;

variable_names
  : variable_name
  | variable_names _COMMA variable_name
  ;

variable_name
  : _ID 
    {
      if (lookup_symbol($1, PAR | PARA_ITER) != NO_INDEX) {
        err("redefinition of '%s'", $1);
      } else {
        if(lookup_symbol($1, VAR) == NO_INDEX) {
          insert_symbol($1, VAR, current_variable_type, ++var_num, block_idx);
        } else if (get_atr2(lookup_symbol($1, VAR)) != block_idx) {
          insert_symbol($1, VAR, current_variable_type, ++var_num, block_idx);
        } else {
          err("redefinition of '%s'", $1);
        }
      }
    }
  ;

statement_list
  : /* empty */
  | statement_list statement
  ;

statement
  : compound_statement
  | assignment_statement
  | if_statement
  | return_statement
  | select_stmt
  | function_call_stmt
  | post_inc_statement
  | para_statement
  | branch_statement
  | while_statement
  ;

select_stmt
  : _SELECT variable_names _FROM select_from_var _WHERE _LPAREN rel_exp _RPAREN _SEMICOLON
  ;

select_from_var
  : _ID
    { 
      if (lookup_symbol($1, VAR|PAR|GVAR) == NO_INDEX) 
        err("select from var '%s' is not declared", $1);
    }
  ;

function_call_stmt
  : function_call _SEMICOLON
  ;

post_inc_statement
  : post_increment _SEMICOLON { apply_post_increment(-1); }
  ;

compound_statement
  : _LBRACKET
    {
      if (is_para) {
        is_para = 0;
      } else {
        block_idx++;
      }
      $<i>$ = get_last_element(); 
    }
    variable_list
    {
      // Make room on stack for the local variables if the new variables have been declared
      int last_idx = get_last_element();
      int num_block_vars = last_idx - $<i>2;
      $<i>$ = num_block_vars;
      take_stack(num_block_vars);
    }
    statement_list
    _RBRACKET
    {
      block_idx--;
      clear_symbols($<i>2 + 1);
      free_stack($<i>4);
      var_num -= $<i>4;
    }
  ;

assignment_statement
  : _ID _ASSIGN num_exp _SEMICOLON
      {
        int idx = lookup_symbol($1, VAR|PAR|GVAR|PARA_ITER);
        if(idx == NO_INDEX)
          err("invalid lvalue '%s' in assignment", $1);
        else
          if(get_type(idx) != get_type($3))
            err("incompatible types in assignment");
        gen_mov($3, idx);
        apply_post_increment(idx);
      }
  ;

num_exp
  : exp
  | num_exp _AROP exp
    {
      if(get_type($1) != get_type($3))
        err("invalid operands: arithmetic operation");
      else {
        int t1 = get_type($1);    
        $$ = gen_arithmetic_instruction($2, $1, $3);
        set_type($$, t1);
      }
    }
  ;

exp
  : literal
  | _ID
    {
      $$ = lookup_symbol($1, VAR|PAR|GVAR|PARA_ITER);
      if($$ == NO_INDEX)
        err("'%s' undeclared", $1);
    }
  | post_increment 
    {
      $$ = $1;
    }
  | function_call
    {
      $$ = take_reg();
      debug("generating move\n");
      gen_mov(FUN_REG, $$);
    }
  | label_inc_lparen num_exp { apply_post_increment(-1); } _RPAREN
    { 
      $$ = $2;
    }
  | ternary_exp
    {
      $$ = $1;
    }
  ;

post_increment
  : _ID _POST_INC
    {
      $$ = lookup_symbol($1, VAR|PAR|GVAR|PARA_ITER);
      if($$ == NO_INDEX)
        err("'%s' undeclared", $1);
      else
        increment_post_inc_count($$);
    }
  ;

literal
  : _INT_NUMBER
      { $$ = insert_literal($1, INT); }

  | _UINT_NUMBER
      { $$ = insert_literal($1, UINT); }
  ;

function_call
  : _ID 
    {
      fcall_idx = lookup_symbol($1, FUN);
      if(fcall_idx == NO_INDEX) {
        err("'%s' is not a function", $1);
      } else {
        //gen_fun_call_save_args_on_stack(fun_idx, fcall_idx);
      }
      prepare_args_stack(fcall_idx);
    }
    _LPAREN argument_list _RPAREN
    {
      if(get_atr1(fcall_idx) != $4) {
        err("wrong number of args to function '%s': exptected %d got %d", get_name(fcall_idx), get_atr1(fcall_idx), $4);
      } else {
        gen_func_call(fun_idx, fcall_idx);
        set_type(FUN_REG, get_type(fcall_idx));
        num_args = 0;
        $$ = FUN_REG;
      }
    }
  ;

argument_list
  : { $$ = 0; }
  | arguments { $$ = $1; }
  ;

arguments:
  argument
    {
      if(get_param_type(fcall_idx, ++num_args) != $1) {
        err("incompatible type for argument #%d in '%s'", 1, get_name(fcall_idx));
      }
      $$ = 1;
    }
  | arguments _COMMA argument
    {
      if(get_param_type(fcall_idx, ++num_args) != $3) {
        err("incompatible type for argument #%d in '%s'", $1, get_name(fcall_idx));
      }
      $$ = $1 + 1;
    }
  ;

argument
  : num_exp
    { 
      $$ = get_type($1);
      add_arg($1);
      apply_post_increment(-1); 
    }
  ;

if_statement
  : if_part %prec ONLY_IF
      { code("\n.exit%d:", $1); }

  | if_part _ELSE statement
      { code("\n.exit%d:", $1); }
  ;

if_part
  : _IF _LPAREN
      {
        $<i>$ = ++lab_num;
        code("\n.if%d:", lab_num);
      }
    condition
      {
        //code("\n\t\t%s\t.false%d", opp_jumps[$4], $<i>3); 
        gen_opposite_branch(&current_relop, $<i>3);
        code("\n.true%d:", $<i>3);
      }
    _RPAREN statement
      {
        code("\n\tj\t.exit%d", $<i>3);
        code("\n.false%d:", $<i>3);
        $$ = $<i>3;
      }
  ;

rel_exp
  : num_exp { apply_post_increment(-1); } _RELOP num_exp { apply_post_increment(-1); }
      {
        if(get_type($1) != get_type($4)) {
          err("invalid operands: relational operator");
        } else {
          $$ = NO_INDEX;
          current_relop.left_index = $1;
          current_relop.operator = $3 + ((get_type($1) - 1) * RELOP_NUMBER);
          current_relop.right_index = $4;
          //gen_cmp($1, $4);
        }
      }
  ;



return_statement
  : _RETURN _SEMICOLON
    {
      if (get_type(fun_idx) != VOID)
        warn("function '%s' is expected to return a value", get_name(fun_idx));
      has_return_statement = 1;
      code("\n\tj \t.%s_exit", get_name(fun_idx));
    }
  | _RETURN num_exp _SEMICOLON
      {
        if(get_type(fun_idx) != get_type($2)) {
          err("incompatible types in return");
        } else {
          gen_mov($2, FUN_REG);
          has_return_statement = 1;
          code("\n\tj \t.%s_exit", get_name(fun_idx));
          apply_post_increment(-1);
        }        
      }
  ;

para_iter
  : _TYPE 
      { 
        para_type = $1; 
        if (para_type == VOID) {
          err("para iterator can't be void");
        }
      } 
      _ID 
      {
        // TODO: Ako postoji parametar sa tim imenom
        if (lookup_symbol($3, PAR | PARA_ITER) != NO_INDEX) {
          err("variable with name '%s' already declared", $3);
          $$ = NO_INDEX;
        } else {
          if(lookup_symbol($3, VAR) == NO_INDEX) {
            $$ = insert_symbol($3, PARA_ITER, $1, ++var_num, block_idx);
          } else if (get_atr2(lookup_symbol($3, VAR)) != block_idx) {
            $$ = insert_symbol($3, PARA_ITER, $1, ++var_num, block_idx);
          } else {
            err("variable with name '%s' already declared", $3);
            $$ = NO_INDEX;
          }
        }
      }
  ;

para_statement
  : _PARA
    {
      is_para = 1;
      $<i>$ = block_idx++;
    } 
    _LPAREN 
    para_iter
    _ASSIGN
    literal
      {
        if (get_type($6) != para_type)
          err("lower bound type must be the same as iterator type");
      }
    _COLON 
    literal
      {
        if (get_type($9) != para_type)
          err("higher bound type must be the same as iterator type");
        if (atoi(get_name($6)) > atoi(get_name($9))) {
          err("lower bound must be lower than the higher bound");
        }
      }
    _RPAREN 
    {
      // Generate para label
      code("\n.para%d_init:", ++para_num);
      $<i>$ = para_num;
      take_stack(1); // Make space for para iterator on stack
      gen_mov($6, $4);
      code("\n.para%d_check:", para_num);
      gen_para_check($4, $9, para_num);
      code("\n.para%d_body:", para_num);
      para_type = NO_TYPE;
    }
    statement
      {
        if ($4 != NO_INDEX) {
          code("\n.para%d_step:", $<i>12);
          gen_post_inc($4);
          code("\n\tj \t.para%d_check", $<i>12);
          code("\n.para%d_exit:", $<i>12);
          free_stack(1); // Remove para iterator from stack
          clear_symbols($4);
        }
        is_para = 0;
        block_idx = $<i>2;
      }
  ;

branch_var
  : _ID
    {
      if (lookup_symbol($1, VAR|GVAR|PAR|PARA_ITER) == NO_INDEX) {
        err("branch var '%s' is not declared", $1);
      } else {
        $$ = lookup_symbol($1, VAR|PAR|GVAR|PARA_ITER);
      }
    }
  ;

branch_statement
  : _BRANCH { $<i>$ = ++branch_num; } _LSQBRACKET branch_var _SEMICOLON 
    literal
      {
        if (get_type($6) != get_type($4))
          err("first constant must be the same type as branch var");
      }
    _COMMA literal 
      {
        if (get_type($9) != get_type($4))
          err("second constant must be the same type as branch var");
      }
    _COMMA literal 
      {
        if (get_type($12) != get_type($4))
          err("third constant must be the same type as branch var");
      }
    _RSQBRACKET
      {
        if (atoi(get_name($6)) == atoi(get_name($9)))
          err("first and second case have the same value");
        if (atoi(get_name($6)) == atoi(get_name($12)))
          err("first and third case have the same value");
        if (atoi(get_name($9)) == atoi(get_name($12)))
          err("second and third case have the same value");

        gen_branch_branches($4, $6, $9, $12, $<i>2);
      }
    _FIRST _ARROW
    {
      code("\n.branch%d_first:", $<i>2);
    }
    statement
    _SECOND _ARROW 
    {
      code("\n\tj \t.branch%d_exit", $<i>2);
      code("\n.branch%d_second:", $<i>2);
    }
    statement
    _THIRD _ARROW 
    {
       code("\n\tj \t.branch%d_exit", $<i>2);
      code("\n.branch%d_third:", $<i>2);
    }
    statement
    _OTHERWISE _ARROW 
    {
       code("\n\tj \t.branch%d_exit", $<i>2);
      code("\n.branch%d_otherwise:", $<i>2);
    }
    statement
    {
      code("\n.branch%d_exit:", $<i>2);
    }
  ;

while_statement
  : _WHILE 
    { 
      $<i>$ = ++lab_num;
      code("\n.while%d:", lab_num);
    }
    _LPAREN
    condition 
    {
      gen_opposite_branch(&current_relop, $<i>2);
    }
    _RPAREN 
    {
      code("\n.true%d:", $<i>2);
    }
    statement
    {
      code("\n\tj \t.while%d", $<i>2);
      code("\n.false%d:", $<i>2);
    }
  ;

ternary_exp_op
  : literal
  | _ID
    {
      $$ = lookup_symbol($1, VAR|PAR|GVAR|PARA_ITER);
      if ($$ == NO_INDEX) {
        err("'%s' undeclared", $1);
      }
    }
  ;

ternary_exp
  : label_inc_lparen
    condition
    {
      gen_opposite_branch(&current_relop, $1); 
    }
    _RPAREN _QUESTION ternary_exp_op _COLON ternary_exp_op
    {
      $$ = take_reg();
      code("\n.true%d:", $1);
      gen_mov($6, $$);
      code("\n\tj \t.exit%d", $1);
      code("\n.false%d:", $1);
      gen_mov($8, $$);
      if (get_type($6) != get_type($8)) {
        err("ternary op expressions must be of the same type.");
      }
      code("\n.exit%d:", $1);
    }
  ;

condition
  : rel_exp { $$ = NO_INDEX; }
  | condition 
    _LOGOP
    {
      if ($2 == AND) {
        gen_opposite_branch(&current_relop, lab_num);
      } else {
        gen_true_branch(&current_relop, lab_num);
      }
    }
    rel_exp { $$ = NO_INDEX;}
  ;

label_inc_lparen
  : _LPAREN { $$ = ++lab_num; }

%%

int yyerror(char *s) {
  fprintf(stderr, "\nline %d: ERROR: %s", yylineno, s);
  error_count++;
  return 0;
}

void warning(char *s) {
  fprintf(stderr, "\nline %d: WARNING: %s", yylineno, s);
  warning_count++;
}

int main() {
  int synerr;
  init_symtab();
  init_func_param_map();
  clear_relop(&current_relop);
  output = fopen("output.s", "w+");

  reset_post_increment_count();

  synerr = yyparse();

  clear_symtab();
  fclose(output);
  
  if(warning_count)
    printf("\n%d warning(s).\n", warning_count);

  if(error_count) {
    remove("output.asm");
    printf("\n%d error(s).\n", error_count);
  }

  if(synerr)
    return -1;  //syntax error
  else if(error_count)
    return error_count & 127; //semantic errors
  else if(warning_count)
    return (warning_count & 127) + 127; //warnings
  else
    return 0; //OK
}

