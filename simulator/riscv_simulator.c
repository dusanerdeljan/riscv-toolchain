#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "riscv_simulator.h"
#include "defs.h"

extern int yylineno;
int max_steps = -1;

word section_data[SECTION_DATA_LENGTH];
word stack_segment[STACK_SEGMENT_LENGTH];
s_instruction section_text[SECTION_TEXT_LENGTH];
s_symbol symbol_table[SYMTAB_LENGTH];
s_symbol globals[SECTION_DATA_LENGTH];
s_processor processor;

char source_buffer[256];
s_source source[2000];  // :D

int symtab_index = 0;
int data_index = 0;
int text_index = 0;
int source_index = 0;
int global_index = 0;

/* reads from keypress, doesn't echo
   AUTHOR: Zobayer Hasan, http://zobayer.blogspot.com/2010/12/getch-getche-in-gccg.html */
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

//zamenjuje jedan podstring drugim
int string_replace(char** String, char* ReplaceWhat, char* ReplaceTo) {
    char *NewString, *c, *lastf;
    char *found = *String;
    int count = 0;
    int RepToLen = strlen(ReplaceTo);
    int RepWhatLen = strlen(ReplaceWhat);

    while (found) {
        if ((found = strstr(found, ReplaceWhat)) != 0) {
            found++;
            count++;
        }
    }
    if (count) {
        c = NewString = malloc(strlen(*String) + count*(RepToLen - RepWhatLen) + 1);
        lastf = *String;
        while ((found = strstr(lastf, ReplaceWhat)) != 0) {
            if (found != lastf) {
                strncpy(c, lastf, found-lastf);
                c += found-lastf;
            }
            strncpy(c, ReplaceTo, RepToLen);
            c += RepToLen;
            lastf = found+RepWhatLen;
        }
        strcpy(c, lastf);
        free(*String);
        *String = NewString;
    }
    return count;
}

//funkcija koja radi isto što i printf, samo što pre ispisa
//konvertuje konstante za boje u odgovarajuće escape sekvence
int cprintf(const char *format, ...) {
    size_t size;
    size_t sizem=1024;
    va_list ap,apc;
    char* buff;
    int ret;

    va_start(ap,format);
    do {
        buff = (char*)malloc(sizem);
        va_copy(apc, ap);
        size = vsnprintf(buff, sizem, format, apc);
        va_end(apc);
        if (size<sizem) break;
        else {
            free(buff);
            sizem = size+1;
        }
     } while (1);
    va_end(ap);

    string_replace(&buff,"{BLU}", "\033[1;34m");
    string_replace(&buff,"{GRN}", "\033[1;32m");
    string_replace(&buff,"{CYN}", "\033[1;36m");
    string_replace(&buff,"{RED}", "\033[1;31m");
    string_replace(&buff,"[BLU]", "\033[1;44m");
    string_replace(&buff,"[GRN]", "\033[1;42m");
    string_replace(&buff,"[CYN]", "\033[1;46m");
    string_replace(&buff,"[RED]", "\033[1;41m");
    string_replace(&buff,"{NRM}", "\033[0m");
    string_replace(&buff,"[NRM]", "\033[0m");

    ret=printf("%s",buff);
    free(buff);
    return ret;
}

word get_scaled_4b_aligned_offset(word offset) {
    if (offset % 4 != 0) {
        simerror("get_scaled_4b_aligned_offset - offset %d is not aligned to 4 bytes", offset);
    }
    return offset / 4;
}

word get_memory_offset(uchar reg, word offset) {
    return *get_reg(reg) / 4 + get_scaled_4b_aligned_offset(offset);
}

word *get_memory(uchar reg, word offset) {
    word scaled = get_memory_offset(reg, offset);
    if (reg == GLOBAL_POINTER) {
        if (scaled >= SECTION_DATA_LENGTH || scaled < 0) {
            simerror("get_memory invalid access to global memory - %d(%s)", offset, abi_regs[reg]);
        }
        return &section_data[scaled];
    } else if (reg == FRAME_POINTER || reg == STACK_POINTER) {
        if (scaled >= STACK_SEGMENT_LENGTH || scaled < 0) {
            simerror("get_memory invalid access to stack segment - %d(%s)", offset, abi_regs[reg]);
        }
        return &stack_segment[scaled];
    } else {
        simerror("get_memory invalid use of base register - use sp, fp or gp");
    }
}

word *get_reg(uchar reg) {
    if (reg >= RV32I_REG_NUM) {
        simerror("get_reg: invalid register index");
    }
    return &processor.regs[reg];
}

int get_label_address(word label_index) {
    if (label_index >= symtab_index) {
        simerror("get_label_address: invalid label index");
    }
    return symbol_table[label_index].offset;
}

void insert_label_unchecked(char *name, uchar defined) {
    symbol_table[symtab_index].name = strdup(name);
    symbol_table[symtab_index].defined = defined;
    symbol_table[symtab_index].offset = NO_ADDRESS;
    //debug("creating label: %s, on index: %d, defined: %d, address: %d", name, symtab_index, defined, symbol_table[symtab_index].offset);
    symtab_index++;
}

s_operand create_reg_operand(uchar reg) {
    s_operand op;
    op.operand_type = OP_REGISTER;
    op.register_index = reg;
    return op;
}

s_operand create_imm_operand(word imm) {
    s_operand op;
    op.operand_type = OP_IMMEDIATE;
    op.data = imm;
    return op;
}

s_operand create_reg_offset_operand(word offset, uchar reg) {
    s_operand op;
    op.operand_type = OP_REGISTER_OFFSET;
    op.data = offset;
    op.register_index = reg;
    return op;
}

s_operand create_address_operand(char *name) {
    s_operand op;
    op.operand_type = OP_ADDRESS;
    //debug("creating address operand, symtab_index: %d", symtab_index);
    op.data = ensure_label(name);
    return op;
}

void insert_label(char *name) {
    int i;
    for (i = 0; i < symtab_index; i++) {
        if (strcmp(name, symbol_table[i].name) == 0) {
            if (symbol_table[i].defined == TRUE) {
                parsererror("label %s already defined", name);
            } else {
                symbol_table[i].defined = TRUE;
                //debug("found label %s which is not defined...with address %d...", symbol_table[i].name, symbol_table[i].offset);
                return;
            }
        }
    }
    insert_label_unchecked(name, TRUE);
}

int ensure_label(char *name) {
    int i;
    for (i = 0; i < symtab_index; i++) {
        if (strcmp(name, symbol_table[i].name) == 0) {
            //debug("names matched: %s == %s", name, symbol_table[i].name);
            return i;
        }
    }
    //debug("didn't find label: %s", name);
    i = symtab_index;
    // Insert the label because it does not exist
    insert_label_unchecked(name, FALSE);
    //debug("value of newly created label: %d", i);
    return i;
}

void insert_data(word data) {
    section_data[data_index] = data;
    data_index++;
}

void insert_global(char *name) {
    int i;
    for (i = 0; i < global_index; i++) {
        if (strcmp(name, globals[i].name) == 0) {
            parsererror("redefinition of global symbol: %s", name);
        }
    }
    globals[global_index].name = strdup(name);
    globals[global_index].offset = data_index;
    global_index++;
}

void insert_jump(uchar ins_type, char *name) {
    section_text[text_index].instruction_type = ins_type;
    if (ins_type != INS_RET) {
        section_text[text_index].destination = create_address_operand(name);
    }
    insert_instruction(&section_text[text_index]);
}

void insert_branch(uchar ins_type, uchar sign_type, uchar rs1, uchar rs2, char *name) {
    section_text[text_index].instruction_type = ins_type;
    section_text[text_index].sign_type = sign_type;
    section_text[text_index].destination = create_address_operand(name);
    section_text[text_index].source1 = create_reg_operand(rs1);
    section_text[text_index].source2 = create_reg_operand(rs2);
    insert_instruction(&section_text[text_index]);
}

void insert_load_store(uchar ins_type, uchar rd, word offset, uchar rs) {
    section_text[text_index].instruction_type = ins_type;
    if (ins_type == INS_SW) {
        section_text[text_index].destination = create_reg_offset_operand(offset, rs);
        section_text[text_index].source1 = create_reg_operand(rd);
    } else {
        section_text[text_index].destination = create_reg_operand(rd);
        if (ins_type == INS_LI) {
            section_text[text_index].source1 = create_imm_operand(offset);
        } else {
            section_text[text_index].source1 = create_reg_offset_operand(offset, rs);
        }
    }
    insert_instruction(&section_text[text_index]);
}

void insert_arithmetic(uchar ins_type, uchar rd, uchar rs1, uchar rs2) {
    section_text[text_index].instruction_type = ins_type;
    section_text[text_index].destination = create_reg_operand(rd);
    section_text[text_index].source1 = create_reg_operand(rs1);
    section_text[text_index].source2 = create_reg_operand(rs2);
    insert_instruction(&section_text[text_index]);
}

void insert_arithmetic_immediate(uchar ins_type, uchar rd, uchar rs1, word immediate) {
    section_text[text_index].instruction_type = ins_type;
    section_text[text_index].destination = create_reg_operand(rd);
    section_text[text_index].source1 = create_reg_operand(rs1);
    section_text[text_index].source2 = create_imm_operand(immediate);
    insert_instruction(&section_text[text_index]);
}

void insert_nop() {
    section_text[text_index].instruction_type = INS_NOP;
    insert_instruction(&section_text[text_index]);
}

void insert_instruction(s_instruction *ins) {
    //debug("inserted instruction at index: %d", text_index);
    // check if there are any labels with unassigned address and assign them to this instruction
    //debug("symbol table index: %d", symtab_index);
    int i;
    for (i = symtab_index-1; i >= 0; i--) {
        if (symbol_table[i].offset == NO_ADDRESS && symbol_table[i].defined == TRUE) {
            //debug("assigning value %d to label: %s", text_index, symbol_table[i].name);
            symbol_table[i].offset = text_index; // point to the currently inserted instruction
            //debug("assigned %d to label: %s...", text_index, symbol_table[i].name);
        }
    }
    text_index++;
}

// Copy-pase from hipsim :D
void insert_source_f(char *s) {
    source[source_index].text = strdup(s);
    source[source_index].address = text_index;
    source_index++;
    //debug("inserting source code at index: %d", source_index);
}

char type_char(int sign_type) {
    if (sign_type == UNSIGNED_TYPE) {
        return 'u';
    }
    return ' ';
}

void init_simulator() {
    int i;
    processor.done = FALSE;
    processor.pc = 0;
    // initialize frame, stack and global pointer
    processor.regs[FRAME_POINTER]  = 4*(STACK_SEGMENT_LENGTH - 1);
    processor.regs[STACK_POINTER]  = 4*(STACK_SEGMENT_LENGTH - 1);
    processor.regs[GLOBAL_POINTER] = 0;
    for (i = 0; i < SECTION_TEXT_LENGTH; i++) {
        section_text[i].instruction_type = INS_NOP;
        section_text[i].sign_type = NO_TYPE;
    }
    //debug("initialized simulator...");
}

void check_undefined_labels() {
    int i;
    for (i = 0; i < symtab_index; i++) {
        if (symbol_table[i].defined == FALSE) {
            parsererror("undefined label: %s", symbol_table[i].name);
        }
        if (symbol_table[i].offset == NO_ADDRESS) {
            parsererror("unassigned label: %s", symbol_table[i].name);
        }
        //debug("%s: %d", symbol_table[i].name, symbol_table[i].offset);
    }
}

void step() {
    if (processor.pc < 0 || processor.pc >= text_index) {
        simerror("step: invalid value in program counter");
    }
    s_instruction *ins = &section_text[processor.pc];
    switch (ins->instruction_type) {
        case INS_JAL:
            //debug("jal");
            *get_reg(RETURN_ADDRESS_REG) = processor.pc + 1;
            processor.pc = get_label_address(ins->destination.data);
            break;
        case INS_RET: 
            //debug("ret");
            processor.pc = *get_reg(RETURN_ADDRESS_REG);
            break;
        case INS_J: 
            //debug("j");
            //debug("jumping to: %d", get_label_address(ins->destination.data));
            processor.pc = get_label_address(ins->destination.data);
            break;
        case INS_BGE: 
            //debug("bge");
            GENERATE_BRANCH(>=);
            break;
        case INS_BLE: 
            //debug("ble");
            GENERATE_BRANCH(<=);
            break;
        case INS_BGT: 
            //debug("bgt");
            GENERATE_BRANCH(>);
            break;
        case INS_BLT: 
            //debug("blt");
            GENERATE_BRANCH(<);
            break;
        case INS_BEQ: 
            //debug("beq");
            GENERATE_BRANCH(==);
            break;
        case INS_BNE: 
            //debug("bne");
            GENERATE_BRANCH(!=);
            break;
        case INS_ADD: 
            //debug("add");
            *get_reg(ins->destination.register_index) = *get_reg(ins->source1.register_index) + *get_reg(ins->source2.register_index); 
            processor.pc++;
            break;
        case INS_ADDI: 
            //debug("addi");
            *get_reg(ins->destination.register_index) = *get_reg(ins->source1.register_index) + ins->source2.data; 
            processor.pc++;
            break;
        case INS_SUB: 
            //debug("sub");
            *get_reg(ins->destination.register_index) = *get_reg(ins->source1.register_index) - *get_reg(ins->source2.register_index); 
            processor.pc++;
            break;
        case INS_MV: 
            //debug("mv");
            *get_reg(ins->destination.register_index) = *get_reg(ins->source1.register_index);
            processor.pc++;
            break;
        case INS_LW: 
            //debug("lw");
            *get_reg(ins->destination.register_index) = *get_memory(ins->source1.register_index, ins->source1.data);
            processor.pc++;
            break;
        case INS_SW: 
            //debug("sw");
            *get_memory(ins->destination.register_index, ins->destination.data) = *get_reg(ins->source1.register_index);
            processor.pc++;
            break;
        case INS_LI: 
            //debug("li");
            *get_reg(ins->destination.register_index) = ins->source1.data;
            processor.pc++;
            break;
        case INS_NOP:
            //debug("nop");
            processor.done = TRUE; 
            processor.pc++;
            break;
        default: {
            simerror("step encountered an invalid instruction type");
        }
    }
}

void print_registers() {
    static word reg_cache[RV32I_REG_NUM];
    int i;
    cprintf("\n\n{BLU}### Registers ###{NRM}\n");
    printf("PC=%-#10x", processor.pc * 4 + TEXT_SEGMENT_START);
    for (i = 0; i < RV32I_REG_NUM; i++) {
        word reg_value = processor.regs[i];
        if (i == GLOBAL_POINTER) reg_value += STATIC_DATA_START + reg_value*4;
        if (i == STACK_POINTER) reg_value = STACK_SEGMENT_START - (STACK_SEGMENT_LENGTH - 1 - reg_value)*4;
        if (i == FRAME_POINTER) reg_value = STACK_SEGMENT_START - (STACK_SEGMENT_LENGTH - 1 - reg_value)*4;
        if (i % 4 == 0) printf("\n");
        printf("[x%-2d] %-4s= ", i, abi_regs[i]);
        if (processor.regs[i] == reg_cache[i]) {
            printf("%-12d ", reg_value);
        } else {
            cprintf("{RED}%-12d{NRM} ", reg_value);
        }
        reg_cache[i] = processor.regs[i];
    }
}

void print_global_segment() {
    static word global_cache[SECTION_DATA_LENGTH];
    int i;
    cprintf("\n\n{BLU}### Global segment ###{NRM}\n");
    for (i = 0; i < global_index; i++) {
        if (i == processor.regs[GLOBAL_POINTER]) {
            if (section_data[i] == global_cache[i]) {
                cprintf("[%#10x] %-10s = %-5d {GRN}<- gp{NRM}", i*4 + STATIC_DATA_START, globals[i].name, section_data[i]);
            } else {
                cprintf("[%#10x] %-10s = {RED}%-5d{NRM} {GRN}<- gp{NRM}", i*4 + STATIC_DATA_START, globals[i].name, section_data[i]);
            }
        } else {
            if (section_data[i] == global_cache[i]) {
            printf("[%#10x] %-10s = %-5d", i*4 + STATIC_DATA_START, globals[i].name, section_data[i]);
            } else {
                cprintf("[%#10x] %-10s = {RED}%-5d{NRM}", i*4 + STATIC_DATA_START, globals[i].name, section_data[i]);
            }
        }
        global_cache[i] = section_data[i];
        printf("\n");
    }
}

void print_stack_segment() {
    static word stack_cache[STACK_SEGMENT_LENGTH];
    cprintf("\n\n{BLU}### Stack segment ###{NRM}\n");
    int lines = 10;
    int max_stack_idx = STACK_SEGMENT_LENGTH - 1;
    int min_stack_idx = 0;
    int fp_idx = 0;
    int sp_idx = 1;
    int i;
    int first[2];
    int last[2];
    int rescaled[] = {processor.regs[FRAME_POINTER] / 4, processor.regs[STACK_POINTER] / 4};
    for (i = 0; i < 2; i++) {
        first[i] = rescaled[i] + lines/2 + lines%2;
        last[i] = rescaled[i] - lines/2;
        if (first[i] > max_stack_idx) { last[i] = last[i] - (first[i] - max_stack_idx); first[i] = max_stack_idx; }
        if (last[i] < 0) { first[i] = first[i] + last[i]; last[i] = 0; }
        if (first[i] > max_stack_idx) { first[i] = max_stack_idx; }
    }
    char *names[] = {"fp", "sp"};
    cprintf("{BLU}FP relative stack             | SP relative stack             | SP~FP relative offset{NRM}\n");
    for (;first[0]>=last[0] && first[1]>=last[1]; first[0]--, first[1]--) {
        for (i = 0; i < 2; i++) {
            if (first[i] >= last[i]) {
                if (stack_cache[first[i]] != stack_segment[first[i]]) cprintf("{RED}");
                printf("[%#10x] %-5d", STACK_SEGMENT_START - (STACK_SEGMENT_LENGTH - first[i] - 1)*4, stack_segment[first[i]]);
                if (first[i] == rescaled[i]) {
                    cprintf(" {GRN}<-     %s{NRM} ", names[i]);
                } else {
                    int diff = 4*(first[i] - rescaled[i]);
                    printf(" <- %3d(%s)", diff, names[i]);
                }
                // If we are displaying stack pointer, then also display the relative offset based on the frame pointer
                if (i == sp_idx) {
                    int fp_diff = 4*(first[sp_idx] - rescaled[fp_idx]);
                    cprintf(" {BLU}[%5d(fp)]{NRM}", fp_diff);
                }
                cprintf("{NRM}");
                stack_cache[first[i]] = stack_segment[first[i]];
            }
            if (i == fp_idx) printf(" | ");
        }
        printf("\n");
    }
}

void print_code_segment() {
    //debug("i am here");
    int lines = 10;
    int i, first, last;
    for (i = 0; i < source_index; i++)
        if (source[i].address == processor.pc) break;
    first = i - lines/2;
    last = i + lines/2 + lines%2;
    if (first < 0) { last = last - first; first = 0; }
    if (last > source_index) { first = first + source_index - last; last = source_index; }
    if (first < 0) { first = 0; }
    cprintf("{BLU}### Code segment ###{NRM}");
    cprintf("\n{BLU}PC    Addr         Label        Instruction{NRM}");
    for (i = first; i < last ; i++) {
        char c;
        if (source[i].address == source[i+1].address) c = ' ';
        else if (source[i].address == processor.pc) c = '>';
        else c = ' ';
        cprintf("\n{RED}%c{NRM} [%#10x] %s%s{NRM}", c, TEXT_SEGMENT_START + 4*source[i].address,
                c == '>' ? "[RED]" : "", source[i].text);
    }
}

word run_interactive() {
    //debug("running interactiveee dsjgnsksdkgsdkdg");
    do {
        system("clear");
        //debug("i am hereeeeeeee");
        print_code_segment();
        print_global_segment();
        print_registers();
        print_stack_segment();
        printf("\nPress any key to continue, ctrl+c for exit...");
        getch();
        step();
    } while (!processor.done);
    system("clear");
    print_code_segment();
    print_global_segment();
    print_registers();
    print_stack_segment();
    cprintf("\n\n{BLU}Program exit code (%s): {GRN}%d{NRM}\n", abi_regs[FUNCTION_REGISTER], processor.regs[FUNCTION_REGISTER]);
    printf("\nAll OK.\n");
}

word run_simulator() {
    int i;
    // for (i = 0; i < source_index; i++) {
    //     debug("%d: %s", source[i].address, source[i].text);
    // }
    // for (i = 0; i < global_index; i++) {
    //     debug("%s: %d", globals[i].name, globals[i].offset);
    // }
    check_undefined_labels();
    do {
        step();
        if (max_steps > 0) max_steps--;
    } while (!processor.done && (max_steps != 0));
    return processor.regs[FUNCTION_REGISTER];
}