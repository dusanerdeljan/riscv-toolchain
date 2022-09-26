/*
    This file is part of RISC-V Disassembler.

    RISC-V Disassembler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// RV64GC Instruction set, RV128IFDQ
#ifndef DECODINGRV32_H
#define DECODINGRV32_H

#include "instruction.h"

// RV32G/RV64G instruction opcodes
#define BRANCHING 0x18
#define JALR 0x19
#define JAL 0x1b
#define LUI 0x0d
#define AUIPC 0x05
#define ARITHMETIC_I 0x04
#define ARITHMETIC 0x0c
#define LOAD 0x00
#define STORE 0x08
#define FENCE 0x03
#define SYSTEM 0x1c
#define ATOMIC 0xB
#define FLOAD 0x1
#define FSTORE 0x9
#define FMADD 0x10
#define FMSUB 0x11
#define FNMSUB 0x12
#define FNMADD 0x13
#define FPINS 0x14

//RV64I instruction opcodes
#define ARITHMETIC_IW 0x6
#define ARITHMETIC_W 0xe

// Branching instruction subtypes
#define BEQ 0
#define BNE 1
#define BLT 4
#define BGE 5
#define BLTU 6
#define BGEU 7

// Arithmetic_I instruction subtypes, same for Arithmetic subtypes
#define ADDI 0 // bit 30 is 0, SUB bit 30 is 1 (only in Arithmetic)
#define SLLI 1
#define SLTI 2
#define SLTIU 3
#define XORI 4
#define SRLI 5 // bit 30 is 0, SRAI bit 30 is 1
#define ORI 6
#define ANDI 7

// RV32M/RV64M instructions
#define MUL 0         // 32/64             
#define MULH 1   
#define MULHSU 2
#define MULHU 3
#define DIV 4         // 32/64
#define DIVU 5        // 32/64
#define REM 6         // 32/64
#define REMU 7        // 32/64

// Load instruction subtypes
#define LB 0
#define LH 1
#define LW 2
#define LBU 4
#define LHU 5
// RV64I loads
#define LWU 6
#define LD 3

// Store instruction subtypes
#define SB 0
#define SH 1
#define SW 2
// RV64I stores
#define SD 3

// Fence instruction subtypes
#define FENCE_ 0
#define FENCE_I 1

// Fence predecessor
#define FENCE_PI 1 << 27
#define FENCE_PO 1 << 26
#define FENCE_PR 1 << 25
#define FENCE_PW 1 << 24
// Fence succesor
#define FENCE_SI 1 << 23
#define FENCE_SO 1 << 22
#define FENCE_SR 1 << 21
#define FENCE_SW 1 << 20

// System instruction subtypes
#define ECALL 0x0
#define CSRRW 0x1
#define CSRRS 0x2
#define CSRRC 0x3
#define CSRRWI 0x5
#define CSRRSI 0x6
#define CSRRCI 0x7

// Atomic instruction subtypes
#define ATOMIC32 0x2
#define ATOMIC64 0x3
// Atomic instruction bits 31..27
#define LR 0x2
#define SC 0x3
#define AMOSWAP 0x1
#define AMOADD 0x0
#define AMOXOR 0x4
#define AMOAND 0xc
#define AMOOR 0x8
#define AMOMIN 0x10
#define AMOMAX 0x14
#define AMOMINU 0x18
#define AMOMAXU 0x1c

// Floating-point arithmetic instructions (bits 31..27)
#define FADD 0x0
#define FSUB 0x1
#define FMUL 0x2
#define FDIV 0x3
#define FSQRT 0xB

// Floating sign injection
#define FSGNJ 0x4 // funt3 -> FSGNJ(000),FSGNJN(001),FSGNJX(010)
#define FSGNJ_F3 0
#define FSGNJN_F3 1
#define FSGNJX_F3 2

#define FMINMAX 0x5 // funct3 -> FMIN(000), FMAX(001)
#define FMIN_F3 0
#define FMAX_F3 1

#define FCOMPARISON 0x14 // funct3 -> FEQ(010), FLT(001), FLE(000)
#define FEQ 2
#define FLT 1
#define FLE 0

// Floating-point conversion instructions (31..27)
#define FCVTWS 0x18
#define FCVTSW 0x1A
// bits 24..20
#define FWS_R2 0
#define FWUS_R2 1
#define FLS_R2 2
#define FLUS_R2 3

#define FCLASS 0x1C
#define FMVWX 0x1E
// funct3
#define FCLASS_F3 1
#define FMVXW_F3 0

// Floating-point instruction standrads
#define S 0x0
#define D 0x1
#define Q 0x3

// Register order
#define INTFLOAT 1
#define FLOATINT 0

#define OPCODE_MASK 0x7c // bits 6..2
#define SUBTYPE_MASK 0x7000 // bits 14..12
#define SYSTEM_SUBTYPE_MASK 0xfff00000 // bits 31..20
#define FP_STANDARD_MASK 0x6000000 // bits 26..25
#define FP_SUBTYPE_MASK 0xF8000000 // bits 31..27

// Max instruction string length
#define MAX_LENGTH 50

char* getRV32IInstructionMnemonic(int insCode, int offset, INSTRUCTIONS* st_ins);

// Instruction formats
void RTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void ITypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void IShamtTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void ILoadTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void STypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void BTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int offset, INSTRUCTIONS* st_ins);
void JTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int offset, INSTRUCTIONS* st_ins);
void UTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void CSRInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void CSRIInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void FPLoadInstructionFormat(char* insStr, const char* mnemonic, int ins);
void FPStoreInstructionFormat(char* insStr, const char* mnemonic, int ins);
void FPFusedInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void FArithmeticInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins);
void FRTypeNoRoundingInstructionFormat(char* insStr, const char* mnemonic, int ins, int type, INSTRUCTIONS* st_ins);
void FRTwoRegInstructionFormat(char* insStr, const char* mnemonic, const char* type, int ins, int regOrder, INSTRUCTIONS* st_ins);
void FMoveInstructionFormat(char* insStr, int ins, int regOrder, INSTRUCTIONS* st_ins);

// Immediate decoding
int getJImmediateOffsetN(int imm, int numOfBits);
int getImmedaiteN(int imm, int numOfBits);
char* decimalToBinary(int n, int len);

// Subtype decoding
void arithmetic_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins);
void arithmeticI_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins);
void arithmeticIW_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins);
void arithmeticW_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins);
void load_Instrucion(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins);
void store_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins);
void branching_Instruction(char* insStr, int subtype, int ins, int offset, INSTRUCTIONS* st_ins);
void fence_Instruction(char* insStr, int subtype, int ins);
void jalr_Instruction(char* insStr, int ins, int offset, INSTRUCTIONS* st_ins);
void system_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins);
void mul_Instruction(char* insStr, int subtype, int ins, int type, INSTRUCTIONS* st_ins);
void atomic_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins);
void floatingPoint_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins);
void floatingConversion_Instruction(char* insStr, int subtype, int ins, int regOrder, INSTRUCTIONS* st_ins);

#endif