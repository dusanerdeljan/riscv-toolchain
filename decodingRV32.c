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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decodingRV32.h"
#include "csr.h"


// Integer registers - ABI names
char* REGISTERS[] = {"zero", "ra", "sp", "gp", "tp", "t0",
"t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", 
"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", 
"t3", "t4", "t5", "t6"};

// Floating-point registers - ABI names
char* FP_REGISTERS[] = {"ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7",
"fs0", "fs1", "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7",
"fs2", "fs3", "fs4", "fs5", "fs6", "fs7", "fs8", "fs9", "fs10", "fs11",
"ft8", "ft9", "ft10", "ft11"};

// Rounding modes encoding
char* ROUNDING_MODES[] = {"rne", "rtz", "rdn", "rup", "rmm", NULL, NULL, "dynamic"};

int getJImmediateOffsetN(int imm, int numOfBits)
{
    int mask = 1;
    int signMask = 1 << (numOfBits - 1);
    for(int i = 1; i < numOfBits; i++){
        mask <<= 1;
        mask += 1;
    }
    int immediate = imm;
    int sign = (immediate & signMask);
    if(!sign){
        return imm << 1;
    } else {
        immediate = (imm ^ mask) + 1;          // Two's complement
        immediate = immediate << 1;
        return -immediate;
    }
}

int getImmedaiteN(int imm, int numOfBits)
{
    int mask = 1;
    int signMask = 1 << (numOfBits-1);
    for(int i = 1; i < numOfBits; i++){
        mask <<= 1;
        mask += 1;
    }
    int sign = imm;
    sign &= signMask;
    if(!sign){
        return imm;
    } else {
        imm = (imm ^ mask) + 1;
        return -imm;
    }
}

char* decimalToBinary(int n, int len)
{
    int c, d, count;
    char* pointer = (char*)malloc(sizeof(char) * (len+1));
    count = 0;
    for(c = (len-1); c >= 0; c--){
        d = n >> c;
        if(d & 1){
            *(pointer+count) = 1 + '0';
        } else {
            *(pointer+count) = 0 + '0';
        }
        count++;
    }
    *(pointer+count) = '\0';
    return pointer;
}

void RTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    int rs1Index = (ins & 0xF8000) >> 15;
    int rs2Index = (ins & 0x1F00000) >> 20;
    int rdIndex = (ins & 0xF80) >> 7;
    if(st_ins->type == 32 && st_ins->e){
        if(rs1Index > 15 || rdIndex > 15 || rs2Index > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rs1 = REGISTERS[rs1Index];
    char* rs2 = REGISTERS[rs2Index];
    char* rd = REGISTERS[rdIndex];
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,%s", mnemonic, rd, rs1, rs2);
}

void ITypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    int rs1Index = (ins & 0xF8000) >> 15;
    int rdIndex = (ins & 0xF80) >> 7;
    if(st_ins->type == 32 && st_ins->e){
        if(rs1Index > 15 || rdIndex > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rs1 = REGISTERS[rs1Index];
    int immediate = (ins & 0xFFF00000) >> 20;
    immediate = getImmedaiteN(immediate, 12);
    char* rd = REGISTERS[rdIndex];
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,%d", mnemonic, rd, rs1, immediate);
}

void IShamtTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    int rs1Index = (ins & 0xF8000) >> 15;
    int rdIndex = (ins & 0xF80) >> 7;
    if(st_ins->type == 32 && st_ins->e){
        if(rs1Index > 15 || rdIndex > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rs1 = REGISTERS[rs1Index];
    int immediate = st_ins->type >= 64 ? (ins & 0x3F00000) >> 20 : (ins & 0x1F00000) >> 20;
    char* rd = REGISTERS[rdIndex];
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,%d", mnemonic, rd, rs1, immediate);
}

void ILoadTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    int rs1Index = (ins & 0xF8000) >> 15;
    int rdIndex = (ins & 0xF80) >> 7;
    if(st_ins->type == 32 && st_ins->e){
        if(rs1Index > 15 || rdIndex > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rs1 = REGISTERS[rs1Index];
    int immediate = (ins & 0xFFF00000) >> 20;
    immediate = getImmedaiteN(immediate, 12);
    char* rd = REGISTERS[rdIndex];
    snprintf(insStr, MAX_LENGTH, "%s %s,%d(%s)", mnemonic, rd, immediate, rs1);
}

void STypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    int rs1Index = (ins & 0xF8000) >> 15;
    int rs2Index = (ins & 0x1F00000) >> 20;
    if(st_ins->type == 32 && st_ins->e){
        if(rs1Index > 15 || rs2Index > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rs1 = REGISTERS[rs1Index];
    char* rs2 = REGISTERS[rs2Index];
    int imm4_0 = (ins & 0xF80) >> 7;
    int imm11_5 = (ins & 0xFE000000) >> 20;
    int immediate = imm4_0 | imm11_5;
    immediate = getImmedaiteN(immediate, 12);
    snprintf(insStr, MAX_LENGTH, "%s %s,%d(%s)", mnemonic, rs2, immediate, rs1); 
}

void BTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int offset, INSTRUCTIONS* st_ins)
{
    int rs1Index = (ins & 0xF8000) >> 15;
    int rs2Index = (ins & 0x1F00000) >> 20;
    if(st_ins->type == 32 && st_ins->e){
        if(rs1Index > 15 || rs2Index > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rs1 = REGISTERS[rs1Index];
    char* rs2 = REGISTERS[rs2Index]; 
    int immediate = 0;
    int imm11 = (ins & 0x80) << 3;
    int imm4_1 = (ins & 0xF00) >> 8;
    int imm12 = (ins & 0x80000000) >> 20;
    int imm10_5 = (ins & 0x7E000000) >> 21;
    immediate = imm4_1 | imm10_5 | imm11 | imm12;
    immediate = getJImmediateOffsetN(immediate, 12);
    immediate += offset;
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,0x%x", mnemonic, rs1, rs2, immediate);
}

void JTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int offset, INSTRUCTIONS* st_ins)
{
    int rdIndex = (ins & 0xF80) >> 7;
    if(st_ins->type == 32 && st_ins->e){
        if(rdIndex > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rd = REGISTERS[rdIndex];
    int immediate10_1 = (ins & 0x7FE00000) >> 21;
    int immediate11 = (ins & 0x100000) >> 10;
    int immediate19_12 = (ins & 0xFF000) >> 1;
    int immediate20 = (ins & 0x80000000) >> 12;
    int immediate = 0;
    immediate = immediate10_1 | immediate11 | immediate19_12 | immediate20;
    immediate = getJImmediateOffsetN(immediate, 20);
    immediate += offset;
    snprintf(insStr, MAX_LENGTH, "%s %s,0x%x", mnemonic, rd, immediate);
}

void UTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    int rdIndex = (ins & 0xF80) >> 7;
    if(st_ins->type == 32 && st_ins->e){
        if(rdIndex > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rd = REGISTERS[rdIndex];
    int immediate = (ins & 0xFFFFF000) >> 12;
    immediate = getImmedaiteN(immediate, 20);
    snprintf(insStr, MAX_LENGTH, "%s %s,0x%x", mnemonic, rd, immediate);
}

void CSRInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    int rdIndex = (ins & 0xF80) >> 7;
    int rs1Index = (ins & 0xF8000) >> 15;
    if(st_ins->type == 32 && st_ins->e){
        if(rdIndex > 15 || rs1Index > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rd = REGISTERS[rdIndex];
    char* rs1 = REGISTERS[rs1Index];
    int csrCode = (ins & 0xFFF00000) >> 20;
    char* csr = decodeCSR(csrCode);
    if(st_ins->type >= 64)
    {
        for(int i = 17; i >= 0; i--)
        {
            if(csr[i] == 0)
                continue;
            else 
            {
                if(csr[i] == 'h')
                {
                    if(strstr(csr, "scratch") == NULL) {
                        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
                        free(csr);
                        return;
                    }
                    break;
                }
                break;
            }
        }
    }
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,%s", mnemonic, rd, csr, rs1);
    free(csr);
}

void CSRIInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    int rdIndex = (ins & 0xF80) >> 7;
    if(st_ins->type == 32 && st_ins->e){
        if(rdIndex > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rd = REGISTERS[rdIndex];
    int imm = (ins & 0xF8000) >> 15;
    int csrCode = (ins & 0xFFF00000) >> 20;
    char* csr = decodeCSR(csrCode);
    if(st_ins->type >= 64)
    {
        for(int i = 17; i >= 0; i--)
        {
            if(csr[i] == 0)
                continue;
            else 
            {
                if(csr[i] == 'h')
                {
                    if(strstr(csr, "scratch") == NULL) {
                        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
                        free(csr);
                        return;
                    }
                    break;
                }
                break;
            }
        }
    }
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,%d", mnemonic, rd, csr, imm);
    free(csr);
}

void FPLoadInstructionFormat(char* insStr, const char* mnemonic, int ins)
{
    char* rd = FP_REGISTERS[(ins & 0xF80) >> 7];
    char* rs1 = REGISTERS[(ins & 0xF8000) >> 15];
    int immediate = (ins & 0xFFF00000) >> 20;
    immediate = getImmedaiteN(immediate, 12);
    snprintf(insStr, MAX_LENGTH, "%s %s,%d(%s)", mnemonic, rd, immediate, rs1);
}

void FPStoreInstructionFormat(char* insStr, const char* mnemonic, int ins)
{
    char* rs2 = FP_REGISTERS[(ins & 0x1F00000) >> 20];
    char* rs1 = REGISTERS[(ins & 0xF8000) >> 15];
    int imm4_0 = (ins & 0xF80) >> 7;
    int imm11_5 = (ins & 0xFE000000) >> 20;
    int immediate = imm4_0 | imm11_5;
    immediate = getImmedaiteN(immediate, 12);
    snprintf(insStr, MAX_LENGTH, "%s %s,%d(%s)", mnemonic, rs2, immediate, rs1);
}

void FPFusedInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    if(!st_ins->f){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int type = (ins & FP_STANDARD_MASK) >> 25;
    if(!st_ins->d && type == D){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    char* rd = FP_REGISTERS[(ins & 0xF80) >> 7];
    char* rs1 = FP_REGISTERS[(ins & 0xF8000) >> 15];
    char* rs2 = FP_REGISTERS[(ins & 0x1F00000) >> 20];
    char* rs3 = FP_REGISTERS[(ins & 0xF8000000) >> 27];
    char* rm = ROUNDING_MODES[(ins & 0x7000) >> 12];
    char* ext = type == S ? ".s" : ".d";
    if(rm == ROUNDING_MODES[7])
        snprintf(insStr, MAX_LENGTH, "%s%s %s,%s,%s,%s", mnemonic, ext, rd, rs1, rs2, rs3);     
    else 
        snprintf(insStr, MAX_LENGTH, "%s%s %s,%s,%s,%s,%s", mnemonic, ext, rd, rs1, rs2, rs3, rm);
}

void FArithmeticInstructionFormat(char* insStr, const char* mnemonic, int ins, INSTRUCTIONS* st_ins)
{
    int type = (ins & FP_STANDARD_MASK) >> 25;
    if(!st_ins->d && type == D){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    if(!st_ins->q && type == Q){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    char* rd = FP_REGISTERS[(ins & 0xF80) >> 7];
    char* rs1 = FP_REGISTERS[(ins & 0xF8000) >> 15];
    char* rs2 = FP_REGISTERS[(ins & 0x1F00000) >> 20];
    char* rm = ROUNDING_MODES[(ins & SUBTYPE_MASK) >> 12];
    char* ext;
    if(type == D)
        ext = ".d";
    else if(type == S)
        ext = ".s";
    else if(type == Q)
        ext = ".q";
    if(rm == ROUNDING_MODES[7])
    {
        if(mnemonic == "fsqrt")
            snprintf(insStr, MAX_LENGTH, "%s%s %s,%s", mnemonic, ext, rd, rs1);
        else 
            snprintf(insStr, MAX_LENGTH, "%s%s %s,%s,%s", mnemonic, ext, rd, rs1, rs2);
    }
    else
    {
        if(mnemonic == "fsqrt")
            snprintf(insStr, MAX_LENGTH, "%s%s %s,%s,%s", mnemonic, ext, rd, rs1, rm);
        else 
            snprintf(insStr, MAX_LENGTH, "%s%s %s,%s,%s,%s", mnemonic, ext, rd, rs1, rs2, rm);
    }
}

void FRTypeNoRoundingInstructionFormat(char* insStr, const char* mnemonic, int ins, int type, INSTRUCTIONS* st_ins)
{
    int standard = (ins & FP_STANDARD_MASK) >> 25;
    if(!st_ins->d && standard == D){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    if(!st_ins->q && standard == Q){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int rdIndex = (ins & 0xF80) >> 7;
    char* rd = type == FCOMPARISON ? REGISTERS[rdIndex] : FP_REGISTERS[rdIndex];
    char* rs1 = FP_REGISTERS[(ins & 0xF8000) >> 15];
    char* rs2 = FP_REGISTERS[(ins & 0x1F00000) >> 20];
    char* ext;
    if(standard == D)
        ext = ".d";
    else if(standard == S)
        ext = ".s";
    else if(standard == Q)
        ext = ".q";
    snprintf(insStr, MAX_LENGTH, "%s%s %s,%s,%s", mnemonic, ext, rd, rs1, rs2);
}


void FRTwoRegInstructionFormat(char* insStr, const char* mnemonic, const char* type, int ins, int regOrder, INSTRUCTIONS* st_ins)
{
    int standard = (ins & FP_STANDARD_MASK) >> 25;
    if(standard == D && !st_ins->d){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    if(standard == Q && !st_ins->q){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int rdIndex = (ins & 0xF80) >> 7;
    int rsIndex = (ins & 0xF8000) >> 15;
    char* rd = regOrder == INTFLOAT ? REGISTERS[rdIndex] : FP_REGISTERS[rdIndex];
    char* rs1 = regOrder == INTFLOAT ? FP_REGISTERS[rsIndex] : REGISTERS[rsIndex];
    char* rm = ROUNDING_MODES[(ins & SUBTYPE_MASK) >> 12];
    char* ext;
    if(standard == D)
        ext = "d";
    else if(standard == S)
        ext = "s";
    else if(standard == Q)
        ext = ".q";
    if(regOrder == INTFLOAT)
    {
        if((type == "") && (rm == ROUNDING_MODES[7]))
            snprintf(insStr, MAX_LENGTH, "%s.%s %s,%s", mnemonic, ext, rd, rs1);
        else if(mnemonic == "fclass")
            snprintf(insStr, MAX_LENGTH, "%s.%s %s,%s", mnemonic, ext, rd, rs1);
        else if(rm == ROUNDING_MODES[7])
            snprintf(insStr, MAX_LENGTH, "%s.%s.%s %s,%s", mnemonic, type, ext, rd, rs1);
        else 
            snprintf(insStr, MAX_LENGTH, "%s.%s.%s %s,%s,%s", mnemonic, type, ext, rd, rs1, rm);
    }
    else 
    {
        if(rm == ROUNDING_MODES[7] || (mnemonic == "fcvt" && (type == "w" || type == "wu")))
            snprintf(insStr, MAX_LENGTH, "%s.%s.%s %s,%s", mnemonic, ext, type, rd, rs1);
        else 
            snprintf(insStr, MAX_LENGTH, "%s.%s.%s %s,%s,%s", mnemonic, ext, type, rd, rs1, rm);
    }
}

void FMoveInstructionFormat(char* insStr, int ins, int regOrder, INSTRUCTIONS* st_ins)
{
    int standard = (ins & FP_STANDARD_MASK) >> 25;
    if(standard == D){
        if(!(st_ins->d && st_ins->type >= 64)){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    int rdIndex = (ins & 0xF80) >> 7;
    int rsIndex = (ins & 0xF8000) >> 15;
    char* rd = regOrder == INTFLOAT ? REGISTERS[rdIndex] : FP_REGISTERS[rdIndex];
    char* rs1 = regOrder == INTFLOAT ? FP_REGISTERS[rsIndex] : REGISTERS[rsIndex];
    char* ext;
    if(standard == S)
        ext = "w";
    else if(standard == D)
        ext = "d";
    if(regOrder == INTFLOAT)
        snprintf(insStr, MAX_LENGTH, "fmv.x.%s %s,%s", ext, rd, rs1);
    else 
        snprintf(insStr, MAX_LENGTH, "fmv.%s.x %s,%s", ext, rd, rs1);
}

void arithmetic_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins)
{
    if(!(st_ins->i || st_ins->m || st_ins->e)){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int test = ins;
    int bit25 = ins;
    bit25 &= 0x2000000;
    if(bit25){
        if(st_ins->m)
            mul_Instruction(insStr, subtype, ins, 32, st_ins);
        else 
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    if(!st_ins->i){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    switch (subtype)
    {
        case ADDI:
            test = (test & 0x40000000);
            if(test)
                RTypeInstructionFormat(insStr, "sub", ins, st_ins);
            else
                RTypeInstructionFormat(insStr, "add", ins, st_ins);
            break;
        case SLLI:
            RTypeInstructionFormat(insStr, "sll", ins, st_ins);
            break;
        case SLTI:
            RTypeInstructionFormat(insStr, "slt", ins, st_ins);
            break;
        case SLTIU:
            RTypeInstructionFormat(insStr, "sltu", ins, st_ins);
            break;
        case XORI:
            RTypeInstructionFormat(insStr, "xor", ins, st_ins);
            break;
        case SRLI:
            test = (test & 0x40000000);
            if(test)
                RTypeInstructionFormat(insStr, "sra", ins, st_ins);
            else 
                RTypeInstructionFormat(insStr, "srl", ins, st_ins);
            break;
        case ORI:
            RTypeInstructionFormat(insStr, "or", ins, st_ins);
            break;
        case ANDI:
            RTypeInstructionFormat(insStr, "and", ins, st_ins);
            break;
        default:
            strncpy(insStr, "Unknown arithmetic\0", 20);
            break;
    }
}

void arithmeticI_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins)
{
    if(!(st_ins->i || st_ins->e)){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int test = ins;
    switch (subtype)
    {
        case ADDI:
            ITypeInstructionFormat(insStr, "addi", ins, st_ins);
            break;
        case SLLI:
            IShamtTypeInstructionFormat(insStr, "slli", ins, st_ins);
            break;
        case SLTI:
            ITypeInstructionFormat(insStr, "slti", ins, st_ins);
            break;
        case SLTIU:
            ITypeInstructionFormat(insStr, "sltiu", ins, st_ins);
            break;
        case XORI:
            ITypeInstructionFormat(insStr, "xori", ins, st_ins);
            break;
        case SRLI:
            test = (test & 0x40000000);
            if(test)
                IShamtTypeInstructionFormat(insStr, "srai", ins, st_ins);
            else 
                IShamtTypeInstructionFormat(insStr, "srli", ins, st_ins);
            break;
        case ORI:
            ITypeInstructionFormat(insStr, "ori", ins, st_ins);
            break;
        case ANDI:
            ITypeInstructionFormat(insStr, "andi", ins, st_ins);
            break;
        default:
            strncpy(insStr, "Unknown arithmetic_I\0", 20);
            break;
    }
}

void arithmeticIW_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins)
{
    if(!(st_ins->i && (st_ins->type >= 64))){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int test = ins;
    switch(subtype)
    {
        case ADDI:
            ITypeInstructionFormat(insStr, "addiw", ins, st_ins);
            break;
        case SLLI:
            IShamtTypeInstructionFormat(insStr, "slliw", ins, st_ins);
            break;
        case SRLI:
            test = (test & 0x40000000);
            if(test)
                IShamtTypeInstructionFormat(insStr, "sraiw", ins, st_ins);
            else 
                IShamtTypeInstructionFormat(insStr, "srliw", ins, st_ins);
            break;
    }
}

void arithmeticW_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins)
{
    if(!((st_ins->i || st_ins->m) && (st_ins->type >= 64))){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int test = ins;
    int bit25 = ins;
    bit25 &= 0x2000000;
    if(bit25){
        if(st_ins->m)
            mul_Instruction(insStr, subtype, ins, 64, st_ins);
        else 
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    if(!(st_ins->i && st_ins->type >= 64)){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    switch(subtype)
    {
        case ADDI:
            test = (test & 0x40000000);
            if(test)
                RTypeInstructionFormat(insStr, "subw", ins, st_ins);
            else 
                RTypeInstructionFormat(insStr, "addw", ins, st_ins);
            break;
        case SLLI:
            RTypeInstructionFormat(insStr, "sllw", ins, st_ins);
            break;
        case SRLI:
            test = (test & 0x40000000);
            if(test)
                RTypeInstructionFormat(insStr, "sraw", ins, st_ins);
            else 
                RTypeInstructionFormat(insStr, "srlw", ins, st_ins);
            break;
    }
}

void load_Instrucion(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins)
{
    if(!(st_ins->i || st_ins->e)){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    switch(subtype)
    {
        case LB:
            ILoadTypeInstructionFormat(insStr, "lb", ins, st_ins);
            break;
        case LH:
            ILoadTypeInstructionFormat(insStr, "lh", ins, st_ins);
            break;
        case LW:
            ILoadTypeInstructionFormat(insStr, "lw", ins, st_ins);
            break;
        case LBU:
            ILoadTypeInstructionFormat(insStr, "lbu", ins, st_ins);
            break;
        case LHU:
            ILoadTypeInstructionFormat(insStr, "lhu", ins, st_ins);
            break;
        case LWU:
            if(st_ins->type >= 64)
                ILoadTypeInstructionFormat(insStr, "lwu", ins, st_ins);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case LD:
            if(st_ins->type >= 64)
                ILoadTypeInstructionFormat(insStr, "ld", ins, st_ins);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        default:
            strncpy(insStr, "Unknown load\0", 13);
            break;
    }
}

void store_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins)
{
    if(!(st_ins->i || st_ins->e)){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    switch(subtype)
    {
        case SB:
            STypeInstructionFormat(insStr, "sb", ins, st_ins);
            break;
        case SH:
            STypeInstructionFormat(insStr, "sh", ins, st_ins);
            break;
        case SW:
            STypeInstructionFormat(insStr, "sw", ins, st_ins);
            break;
        case SD:
            if(st_ins->type >= 64)
                STypeInstructionFormat(insStr, "sd", ins, st_ins);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        default:
            strncpy(insStr, "Unknown store\0", 14);
            break;
    }
}

void branching_Instruction(char* insStr, int subtype, int ins, int offset, INSTRUCTIONS* st_ins)
{
    switch(subtype)
    {
        case BEQ:
            BTypeInstructionFormat(insStr, "beq", ins, offset, st_ins);
            break;
        case BNE:
            BTypeInstructionFormat(insStr, "bne", ins, offset, st_ins);
            break;
        case BLT:
            BTypeInstructionFormat(insStr, "blt", ins, offset, st_ins);
            break;
        case BGE:
            BTypeInstructionFormat(insStr, "bge", ins, offset, st_ins);
            break;
        case BLTU:
            BTypeInstructionFormat(insStr, "bltu", ins, offset, st_ins);
            break;
        case BGEU:
            BTypeInstructionFormat(insStr, "bgeu", ins, offset, st_ins);
            break;
        default:
            strncpy(insStr, "Unknown branching\0", 20);
            break;
    }
}

void fence_Instruction(char* insStr, int subtype, int ins)
{
    if(subtype == FENCE_I) 
        snprintf(insStr, MAX_LENGTH, "%s", "fence.i");
    else 
    {
        char* predsuccs[] = {"", "w", "r", "rw", "o", "ow", "or", "orw", "i",
        "iw", "ir", "irw", "io", "iow", "ior", "iorw"};
        char* pred = predsuccs[(ins & 0xF000000) >> 24];
        char* succ = predsuccs[(ins & 0xF00000) >> 20];
        if(pred != "" && succ != ""){
            if(pred == predsuccs[0xF] && succ == predsuccs[0xF]){
                snprintf(insStr, MAX_LENGTH, "%s", "fence");
            } else {
                snprintf(insStr, MAX_LENGTH, "fence %s,%s", pred, succ);
            }
        } else if (pred == "" ^ succ == "") {
            char* str = pred == "" ? succ : pred;
            snprintf(insStr, MAX_LENGTH, "fence %s", str);
        } else {
            snprintf(insStr, MAX_LENGTH, "%s", "fence");
        }
    }
}

void jalr_Instruction(char* insStr, int ins, int offset, INSTRUCTIONS* st_ins)
{
    int rs1Index = (ins & 0xF8000) >> 15;
    int rdIndex = (ins & 0xF80) >> 7;
    if(st_ins->type == 32 && st_ins->e){
        if(rs1Index > 15 || rdIndex > 15){
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
        }
    }
    char* rs1 = REGISTERS[rs1Index];
    int immediate = (ins & 0xFFF00000) >> 20;
    immediate = getImmedaiteN(immediate, 12) + offset;
    char* rd = REGISTERS[rdIndex];
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,<0x%x>", "jalr", rd, rs1, immediate);
}

void system_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins)
{
    if(!(st_ins->i || st_ins->e)){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int sysSubtype = ins;
    sysSubtype = (sysSubtype & SYSTEM_SUBTYPE_MASK) >> 20;
    switch(subtype)
    {
        case ECALL:
            if(sysSubtype)
                snprintf(insStr, MAX_LENGTH, "%s", "ebreak");
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "ecall");
            break;
        case CSRRW:
            CSRInstructionFormat(insStr, "csrrw", ins, st_ins);
            break;
        case CSRRS:
            CSRInstructionFormat(insStr, "csrrs", ins, st_ins);
            break;
        case CSRRC:
            CSRInstructionFormat(insStr, "csrrc", ins, st_ins);
            break;
        case CSRRWI:
            CSRIInstructionFormat(insStr, "csrrwi", ins, st_ins);
            break;
        case CSRRSI:
            CSRIInstructionFormat(insStr, "csrrsi", ins, st_ins);
            break;
        case CSRRCI:
            CSRIInstructionFormat(insStr, "csrrci", ins, st_ins);
            break;
        default:
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown syscall");
            break;
    }
}

void mul_Instruction(char* insStr, int subtype, int ins, int type, INSTRUCTIONS* st_ins)
{
    switch(subtype)
    {
        case MUL:
            if(type == 32)
                RTypeInstructionFormat(insStr, "mul", ins, st_ins);
            else
                RTypeInstructionFormat(insStr, "mulw", ins, st_ins);
            break;
        case MULH:
            RTypeInstructionFormat(insStr, "mulh", ins, st_ins);
            break;
        case MULHSU:
            RTypeInstructionFormat(insStr, "mulhsu", ins, st_ins);
            break;
        case MULHU:
            RTypeInstructionFormat(insStr, "mulhu", ins, st_ins);
            break;
        case DIV:
            if(type == 32)
                RTypeInstructionFormat(insStr, "div", ins, st_ins);
            else 
                RTypeInstructionFormat(insStr, "divw", ins, st_ins);
            break;
        case DIVU:
            if(type == 32)
                RTypeInstructionFormat(insStr, "divu", ins, st_ins);
            else 
                RTypeInstructionFormat(insStr, "divuw", ins, st_ins);
            break;
        case REM:
            if(type == 32)
                RTypeInstructionFormat(insStr, "rem", ins, st_ins);
            else 
                RTypeInstructionFormat(insStr, "remw", ins, st_ins);
            break;
        case REMU:
            if(type == 32)
                RTypeInstructionFormat(insStr, "remu", ins, st_ins);
            else 
                RTypeInstructionFormat(insStr, "remuw", ins, st_ins);
            break;
    }
}

void atomic_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins)
{
    if(!st_ins->a){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    if(st_ins->type == 32 && (subtype != ATOMIC32)){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int rs1Index = (ins & 0xF8000) >> 15;
    int rs2Index = (ins & 0x1F00000) >> 20;
    int rdIndex = (ins & 0xF80) >> 7;
    if(st_ins->e && st_ins->type == 32){
        if(rs1Index > 15 || rs2Index > 15 || rdIndex > 15)
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            return;
    }
    int aq = ins;
    aq &= 0x4000000;
    char* aqStr = (aq != 0) ? ".aq" : "";
    int rl = ins;
    rl &= 0x2000000;
    char* rlStr = (rl != 0) ? ".rl" : "";
    char* typeStr = (subtype == ATOMIC32) ? ".w" : ".d";
    char* mnemonic;
    subtype = ins;
    subtype = (subtype & 0xF8000000) >> 27;
    switch(subtype)
    {
        case LR:
            mnemonic = "lr";
            break;
        case SC:
            mnemonic = "sc";
            break;
        case AMOSWAP:
            mnemonic = "amoswap";
            break;
        case AMOADD:
            mnemonic = "amoadd";
            break;
        case AMOXOR:
            mnemonic = "amoxor";
            break;
        case AMOOR:
            mnemonic = "amoor";
            break;
        case AMOMIN:
            mnemonic = "amomin";
            break;
        case AMOMAX:
            mnemonic = "amomax";
            break;
        case AMOMINU:
            mnemonic = "amominu";
            break;
        case AMOMAXU:
            mnemonic = "amomaxu";
            break;
    }
    char* rs1 = REGISTERS[rs1Index];
    char* rs2 = REGISTERS[rs2Index];
    char* rd = REGISTERS[rdIndex];
    if(mnemonic == "lr")
        snprintf(insStr, MAX_LENGTH, "%s%s%s%s %s,(%s)", mnemonic, typeStr, aqStr, rlStr, rd, rs1);
    else 
        snprintf(insStr, MAX_LENGTH, "%s%s%s%s %s,%s,(%s)", mnemonic, typeStr, aqStr, rlStr, rd, rs2, rs1);
}


void floatingPoint_Instruction(char* insStr, int subtype, int ins, INSTRUCTIONS* st_ins)
{
    if(!st_ins->f){
        snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
        return;
    }
    int type = subtype;
    subtype = (ins & SUBTYPE_MASK) >> 12;
    int conversionType = (ins & 0x1F00000) >> 20;
    switch(type)
    {
        // Arithmetic instructions
        case FADD:
            FArithmeticInstructionFormat(insStr, "fadd", ins, st_ins);
            break;
        case FSUB:
            FArithmeticInstructionFormat(insStr, "fsub", ins, st_ins);
            break;
        case FMUL:
            FArithmeticInstructionFormat(insStr, "fmul", ins, st_ins);
            break;
        case FDIV:
            FArithmeticInstructionFormat(insStr, "fdiv", ins, st_ins);
            break;
        case FSQRT:
            FArithmeticInstructionFormat(insStr, "fsqrt", ins, st_ins);
            break;
        // Sign injection
        case FSGNJ:
            switch(subtype)
            {
                case FSGNJ_F3:
                    FRTypeNoRoundingInstructionFormat(insStr, "fsgnj", ins, type, st_ins);
                    break;
                case FSGNJN_F3:
                    FRTypeNoRoundingInstructionFormat(insStr, "fsgnjn", ins, type, st_ins);
                    break;
                case FSGNJX_F3:
                    FRTypeNoRoundingInstructionFormat(insStr, "fsgnjx", ins, type, st_ins);
                    break;
            }
            break;
        // Min/Max
        case FMINMAX:
            switch(subtype)
            {
                case FMIN_F3:
                    FRTypeNoRoundingInstructionFormat(insStr, "fmin", ins, type, st_ins);
                    break;
                case FMAX_F3:
                    FRTypeNoRoundingInstructionFormat(insStr, "fmax", ins, type, st_ins);
                    break;
            }
            break;
        // Comprasion
        case FCOMPARISON:
            switch(subtype)
            {
                case FEQ:
                    FRTypeNoRoundingInstructionFormat(insStr, "feq", ins, type, st_ins);
                    break;
                case FLT:
                    FRTypeNoRoundingInstructionFormat(insStr, "flt", ins, type, st_ins);
                    break;
                case FLE:
                    FRTypeNoRoundingInstructionFormat(insStr, "fle", ins, type, st_ins);
                    break;
            }
            break;
        // Conversion integer->floating-point
        case FCVTWS:
            floatingConversion_Instruction(insStr, conversionType, ins, INTFLOAT, st_ins);
            break;
        // Conversion floating-point->integer
        case FCVTSW:
            floatingConversion_Instruction(insStr, conversionType, ins, FLOATINT, st_ins);
            break;
        case FCLASS:
            switch(subtype)
            {
                case FCLASS_F3:
                    FRTwoRegInstructionFormat(insStr, "fclass", "", ins, INTFLOAT, st_ins);
                    break;
                case FMVXW_F3:
                    FMoveInstructionFormat(insStr, ins, INTFLOAT, st_ins);
                    break;
            }
            break;
        case FMVWX:
            FMoveInstructionFormat(insStr, ins, FLOATINT, st_ins);
            break;
    }
}

void floatingConversion_Instruction(char* insStr, int subtype, int ins, int regOrder, INSTRUCTIONS* st_ins)
{
    switch(subtype)
    {
        case FWS_R2:
            FRTwoRegInstructionFormat(insStr, "fcvt", "w", ins, regOrder, st_ins);
            break;
        case FWUS_R2:
            FRTwoRegInstructionFormat(insStr, "fcvt", "wu", ins, regOrder, st_ins);
            break;
        case FLS_R2:
            if(st_ins->type >= 64)
                FRTwoRegInstructionFormat(insStr, "fcvt", "l", ins, regOrder, st_ins);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case FLUS_R2:
            if(st_ins->type >= 64)
                FRTwoRegInstructionFormat(insStr, "fcvt", "lu", ins, regOrder, st_ins);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
    }
}

char* getRV32IInstructionMnemonic(int insCode, int offset, INSTRUCTIONS* st_ins)
{
    int test = insCode;
    int type = (test & OPCODE_MASK) >> 2;
    char* instructionString = (char*)calloc(MAX_LENGTH+1, sizeof(char));
    char unknownInstruction[] = "Unknown instruction\0";
    test = insCode;
    int subtype = (test & SUBTYPE_MASK) >> 12;
    int fpSubtype = (insCode & FP_SUBTYPE_MASK) >> 27;
    switch (type)
    {
       case ARITHMETIC:
           arithmetic_Instruction(instructionString, subtype, insCode, st_ins);
           break;
       case ARITHMETIC_I:
           arithmeticI_Instruction(instructionString, subtype, insCode, st_ins);
           break;
        case ARITHMETIC_IW: 
            arithmeticIW_Instruction(instructionString, subtype, insCode, st_ins);
            break;
        case ARITHMETIC_W: 
            arithmeticW_Instruction(instructionString, subtype, insCode, st_ins);
            break;
       case LOAD:
           load_Instrucion(instructionString, subtype, insCode, st_ins);
           break;
        case STORE:
            store_Instruction(instructionString, subtype, insCode, st_ins);
            break;
        case BRANCHING:
            if(st_ins->i || st_ins->e)
                branching_Instruction(instructionString, subtype, insCode, offset, st_ins);
            else 
                snprintf(instructionString, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case JALR:
            if(st_ins->i || st_ins->e)
                jalr_Instruction(instructionString, insCode, offset, st_ins);
            else 
                snprintf(instructionString, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case JAL:
            if(st_ins->i || st_ins->e)
                JTypeInstructionFormat(instructionString, "jal", insCode, offset, st_ins);
            else 
                snprintf(instructionString, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case LUI:
            if(st_ins->i || st_ins->e)
                UTypeInstructionFormat(instructionString, "lui", insCode, st_ins);
            else 
                snprintf(instructionString, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case AUIPC:
            if(st_ins->i || st_ins->e)
                UTypeInstructionFormat(instructionString, "auipc", insCode, st_ins);
            else 
                snprintf(instructionString, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case FENCE:
            if(st_ins->i || st_ins->e)
                fence_Instruction(instructionString, subtype, insCode);
            else 
                snprintf(instructionString, MAX_LENGTH, "%s", "Unknown instruction");
            break;    
        case SYSTEM:
            system_Instruction(instructionString, subtype, insCode, st_ins);
            break;
        case ATOMIC:
            atomic_Instruction(instructionString, subtype, insCode, st_ins);
            break;
        case FLOAD:
            if(subtype == 2 && st_ins->f)
                FPLoadInstructionFormat(instructionString, "flw", insCode);
            else if(subtype == 3 && st_ins->f && st_ins->d)
                FPLoadInstructionFormat(instructionString, "fld", insCode);
            else if(subtype == 1 && st_ins->f && st_ins->d && st_ins->q)
                FPLoadInstructionFormat(instructionString, "flq", insCode);
            else
                snprintf(instructionString, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case FSTORE:
            if(subtype == 2 && st_ins->f)
                FPStoreInstructionFormat(instructionString, "fsw", insCode);
            else if(subtype == 3 && st_ins->f && st_ins->d)
                FPStoreInstructionFormat(instructionString, "fsd", insCode);
            else if(subtype == 1 && st_ins->f && st_ins->d && st_ins->q)
                FPStoreInstructionFormat(instructionString, "fsq", insCode);
            else 
                snprintf(instructionString, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case FMADD:
            FPFusedInstructionFormat(instructionString, "fmad", insCode, st_ins);
            break;
        case FMSUB:
            FPFusedInstructionFormat(instructionString, "fmsub", insCode, st_ins);
            break;
        case FNMSUB:
            FPFusedInstructionFormat(instructionString, "fnmsub", insCode, st_ins);
            break;
        case FNMADD:
            FPFusedInstructionFormat(instructionString, "fnmadd", insCode, st_ins);
            break;
        case FPINS:
            floatingPoint_Instruction(instructionString, fpSubtype, insCode, st_ins);
            break;
        default:
            strncpy(instructionString, unknownInstruction, sizeof(unknownInstruction));
    }
    return instructionString;
}
