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
#include "../inc/decodingRVC.h"
#include "../inc/decodingRVG.h"

// 3 bit encoded integer registers
char* REGISTERS3[] = {"s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5"};

// 5 bit encoded integer registers
char* REGISTERS5[] = {"zero", "ra", "sp", "gp", "tp", "t0",
"t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", 
"s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", 
"t3", "t4", "t5", "t6"};

// 3 bit encoded floating-point registers
char* FPREGISTERS3[] = {"fs0", "fs1", "fa0", "fa1", "fa2", "fa3", "fa4", "fa5"};

void CITypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int uimm)
{
    int test = ins;
    int reg = (test & 0xF80) >> 7;
    if (mnemonic == "addi" && reg == 0){
        snprintf(insStr, MAX_LENGTH, "%s zero,zero,0", "addi");
        return;
    }
    char* rd = REGISTERS5[reg];
    test = ins;
    int imm4_0 = (test & 0x7C) >> 2;
    int imm5 = (ins & 0x1000) >> 7;
    int immediate = imm4_0 | imm5;
    if(!uimm)
        immediate = getImmedaiteN(immediate, 6);
    if(mnemonic == "li")
        snprintf(insStr, MAX_LENGTH, "addi %s,zero,%d", rd, immediate);
    else if (mnemonic == "slli" && immediate == 0)
        snprintf(insStr, MAX_LENGTH, "slli %s,%s,64", rd, rd);
    else 
        snprintf(insStr, MAX_LENGTH, "%s %s,%s,%d", mnemonic, rd, rd, immediate);
}

void CITypeInstructionFormatShortRegisters(char* insStr, const char* mnemonic, int ins, int uimm)
{
    int test = ins;
    char* rd_rs1 = REGISTERS3[(test & 0x380) >> 7];
    test = ins;
    int uimm4_0 = (test & 0x7C) >> 2;
    int uimm5 = (ins & 0x1000) >> 7;
    int uimmediate = uimm4_0 | uimm5;
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,%d", mnemonic, rd_rs1, rd_rs1, uimmediate);
}

void CITypeInstructionFormatShortRegistersZeroImmediate(char* insStr, const char* mnemonic, int ins)
{
    char* rd_rs1 = REGISTERS3[(ins & 0x380) >> 7];
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,64", mnemonic, rd_rs1, rd_rs1);
}

void CThreeRegisterInstructionFormat(char* insStr, const char* mnemonic, int ins)
{
    int test = ins;
    char* rs2 = REGISTERS3[(test & 0x1C) >> 2];
    char* rd_rs1 = REGISTERS3[(ins & 0x380) >> 7];
    snprintf(insStr, MAX_LENGTH, "%s %s,%s,%s", mnemonic, rd_rs1, rd_rs1, rs2);
}

void CBTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int offset)
{
    int imm2_1 = ins;
    imm2_1 = (imm2_1 & 0x18) >> 3;
    int imm4_3 = ins;
    imm4_3 = (imm4_3 & 0xC00) >> 8;
    int imm5 = ins;
    imm5 = (imm5 & 0x4) << 2;
    int imm7_6 = ins;
    imm7_6 = (imm7_6 & 0x60);
    int imm8 = ins;
    imm8 = (imm8 & 0x1000) >> 5;
    int immediate = imm2_1 | imm4_3 | imm5 | imm7_6 | imm8;
    immediate = getJImmediateOffsetN(immediate, 8);
    immediate += offset;
    char* rs = REGISTERS3[(ins & 0x380) >> 7];
    snprintf(insStr, MAX_LENGTH, "%s %s,zero,0x%x", mnemonic, rs, immediate);
}

void CJTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int offset)
{
    int imm3_1 = ins;
    imm3_1 = (imm3_1 & 0x38) >> 3;
    int imm4 = ins;
    imm4 = (imm4 & 0x800) >> 8;
    int imm5 = ins;
    imm5 = (imm5 & 0x4) << 2;
    int imm6 = ins;
    imm6 = (imm6 & 0x80) >> 2;
    int imm7 = ins;
    imm7 &= 0x40;
    int imm9_8 = ins;
    imm9_8 = (imm9_8 & 0x600) >> 2;
    int imm10 = ins;
    imm10 = (imm10 & 0x100) << 1;
    int imm11 = (ins & 0x1000) >> 2;
    int immediate = imm3_1 | imm4 | imm5 | imm6 | imm7 | imm9_8 | imm10 | imm11;
    immediate = getJImmediateOffsetN(immediate, 11);
    immediate += offset;
    if(mnemonic == "j")
        snprintf(insStr, MAX_LENGTH, "jal zero,0x%x", immediate);
    else if(mnemonic == "jal")
        snprintf(insStr, MAX_LENGTH, "jal ra,0x%x", immediate);
}

void CLuiInstructionFormat(char* insStr, int ins)
{
    int nzimm16_12 = ins;
    nzimm16_12 = (nzimm16_12 & 0x7C) >> 2;
    int nzimm17 = ins;
    nzimm17 = (nzimm17 & 0x1000) >> 7;
    int immediate = nzimm16_12 | nzimm17;
    immediate = getImmedaiteN(immediate, 6);
    char* rd = REGISTERS5[(ins & 0xF80) >> 7];
    snprintf(insStr, MAX_LENGTH, "lui %s,0x%x", rd, immediate);
}


void CAddi16spInstructionFormat(char* insStr, int ins)
{
    int nzimm4 = ins;
    nzimm4 = (nzimm4 & 0x40) >> 6;
    int nzimm5 = ins;
    nzimm5 = (nzimm5 & 0x4) >> 1;
    int nzimm6 = ins;
    nzimm6 = (nzimm6 & 0x20) >> 3;
    int nzimm8_7 = ins;
    nzimm8_7 &= 0x18;
    int nzimm9 = ins;
    nzimm9 = (nzimm9 & 0x1000) >> 7;
    int immediate = nzimm4 | nzimm5 | nzimm6 | nzimm8_7 | nzimm9;
    immediate = getImmedaiteN(immediate, 6) << 4;
    snprintf(insStr, MAX_LENGTH, "addi sp,sp,%d", immediate);
}

void CAddi4spnInstructionFormat(char* insStr, int ins)
{
    int imm3 = ins;
    imm3 = (imm3 & 0x20) >> 4;
    int imm2 = ins;
    imm2 = (imm2 & 0x40) >> 6;
    int imm5_4 = ins;
    imm5_4 = (imm5_4 & 0x1800) >> 9;
    int imm9_6 = ins;
    imm9_6 = (imm9_6 & 0x780) >> 3;
    int immediate = (imm2 | imm3 | imm5_4 | imm9_6) << 2;
    char* rd = REGISTERS3[(ins & 0x1c) >> 2];
    snprintf(insStr, MAX_LENGTH, "addi %s,sp,%d", rd, immediate);
}

void CLSTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int uimmPattern)
{
    int test = ins;
    char* rd = strstr(mnemonic, "f") != NULL ? FPREGISTERS3[(test & 0x1c) >> 2] : REGISTERS3[(test & 0x1c) >> 2];
    test = ins;
    char* rs1 = REGISTERS3[(test & 0x380) >> 7];
    int immediate = 0;
    if(uimmPattern == 5376)
    {
        int uimm5_3 = ins;
        uimm5_3 = (uimm5_3 & 0x1C00) >> 10;
        int uimm7_6 = ins;
        uimm7_6 = (uimm7_6 & 0x60) >> 2;
        immediate = (uimm5_3 | uimm7_6) << 3;
    } 
    else if (uimmPattern == 54876)
    {
        int uimm5_4 = ins;
        uimm5_4 = (ins & 0x1800) >> 11;
        int uimm7_6 = ins;
        uimm7_6 = (uimm7_6 & 0x60) >> 3;
        int uimm8 = ins;
        uimm8 = (uimm8 & 0x400) >> 6;
        immediate = (uimm5_4 | uimm7_6 | uimm8) << 4;
    }
    else if (uimmPattern == 5326)
    {
        int uimm5_3 = ins;
        uimm5_3 = (uimm5_3 & 0x1C00) >> 9;
        int uimm2 = ins;
        uimm2 = (uimm2 & 0x40) >> 6;
        int uimm6 = ins;
        uimm6 = (uimm6 & 0x20) >> 1;
        immediate = (uimm2 | uimm5_3 | uimm6) << 2;
    }
    snprintf(insStr, MAX_LENGTH, "%s %s,%d(%s)", mnemonic, rd, immediate, rs1);
}

void CStoreSPInstructionFormat(char* insStr, const char* mnemonic, int ins, int uimmPattern)
{
    int test = ins;
    char* rs2 = REGISTERS5[(test & 0x7C) >> 2];
    int immediate = 0;
    if(uimmPattern == 5496)
    {
        int uimm5_4 = ins;
        uimm5_4 = (uimm5_4 & 0x1800) >> 11;
        int uimm9_6 = ins;
        uimm9_6 = (uimm9_6 & 0x780) >> 5;
    }
    else if(uimmPattern == 5276)
    {
        int uimm5_2 = ins;
        uimm5_2 = (uimm5_2 & 0x1E00) >> 9;
        int uimm7_6 = ins;
        uimm7_6 = (uimm7_6 & 0x180) >> 3;
        immediate = (uimm5_2 | uimm7_6) << 2;
    }
    else if(uimmPattern == 5386)
    {
        int uimm5_3 = ins;
        uimm5_3 = (uimm5_3 & 0x1C00) >> 10;
        int uimm8_6 = ins;
        uimm8_6 = (uimm8_6 & 0x380) >> 4;
        immediate = (uimm5_3 | uimm8_6) << 3;
    }
    snprintf(insStr, MAX_LENGTH, "%s %s,%d(sp)",mnemonic, rs2, immediate);
}

void CLoadSPInstructionFormat(char* insStr, const char* mnemonic, int ins, int uimmPattern)
{
    int rdIndex = ins;
    rdIndex = (rdIndex & 0xF80) >> 7;
    char* rd = strstr(mnemonic, "f") != NULL ? FPREGISTERS3[rdIndex] : REGISTERS3[rdIndex];
    int immediate = 0;
    int uimm5 = ins;
    uimm5 &= 0x1000;
    if(uimmPattern == 5496)
    {
        int uimm4 = ins;
        uimm4 = (uimm4 & 0x40) >> 6;
        uimm5 >>= 11;
        int uimm9_6 = ins;
        uimm9_6 = (uimm9_6 & 0x3C);
        immediate = (uimm4 | uimm5 | uimm9_6) << 4;
    }
    else if(uimmPattern == 54276)
    {
        int uimm4_2 = ins;
        uimm4_2 = (uimm4_2 & 0x70) >> 4;
        uimm5 >>= 9;
        int uimm7_6 = ins;
        uimm7_6 = (uimm7_6 & 0xC) << 2;
        immediate = (uimm4_2 | uimm5 | uimm7_6) << 2;
    }
    else if(uimmPattern == 54386)
    {
        int uimm4_3 = ins;
        uimm4_3 = (uimm4_3 & 0x60) >> 5;
        uimm5 >>= 10;
        int uimm8_6 = ins;
        uimm8_6 = (uimm8_6 & 0x1C) << 1;
        immediate = (uimm4_3 | uimm5 | uimm8_6) << 3;
    }
    snprintf(insStr, MAX_LENGTH, "%s %s,%d(sp)", mnemonic, rd, immediate);
}

void cArithmeticInstruction(char* insStr, int insCode, INSTRUCTIONS* st_ins)
{
    int test = insCode;
    test = (test & 0xC00) >> 10;
    int bit12 = insCode;
    bit12 &= 0x1000;
    int bits23456 = insCode;
    bits23456 &= 0x7C;
    int bits56 = insCode;
    bits56 = (bits56 & 0x60) >> 5;
    switch(test)
    {
        case 0:
            if((bit12 | bits23456) != 0)
                CITypeInstructionFormatShortRegisters(insStr, "srli", insCode, 1);
            else{
                if(st_ins->type == 128)
                    CITypeInstructionFormatShortRegistersZeroImmediate(insStr, "srli", insCode);
                else 
                    snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            }
            break;
        case 1:
            if((bit12 | bits23456) != 0)
                CITypeInstructionFormatShortRegisters(insStr, "srai", insCode, 1);
            else {
                if(st_ins->type == 128)
                    CITypeInstructionFormatShortRegistersZeroImmediate(insStr, "srai", insCode);
                else 
                    snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            }
            break;
        case 2:
            CITypeInstructionFormatShortRegisters(insStr, "andi", insCode, 0);
            break;
        case 3:
            switch(bits56)
            {
                case 0:
                    if(bit12){
                        if(st_ins->type != 32)
                            CThreeRegisterInstructionFormat(insStr, "subw", insCode);
                        else 
                            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
                    }
                    else 
                        CThreeRegisterInstructionFormat(insStr, "sub", insCode);
                    break;
                case 1:
                    if(bit12){
                        if(st_ins->type != 32)
                            CThreeRegisterInstructionFormat(insStr, "addw", insCode);
                        else 
                            snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
                    }
                    else 
                        CThreeRegisterInstructionFormat(insStr, "xor", insCode);
                    break;
                case 2:
                    CThreeRegisterInstructionFormat(insStr, "or", insCode);
                    break;
                case 3:
                    CThreeRegisterInstructionFormat(insStr, "and", insCode);
                    break;
            }
            break;
        default:
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown arithmetic RVC\0");
            break;
    }
}

void quadrant2Instructions100Funct(char* insStr, int insCode, int offset, INSTRUCTIONS* st_ins)
{
    int bit12 = insCode;
    bit12 = (bit12 & 0x1000) >> 12;
    int rs2bits = insCode;
    rs2bits = (rs2bits & 0x7C) >> 2;
    int rs1rdbits = insCode;
    rs1rdbits = (rs1rdbits & 0xF80) >> 7;
    if(bit12 == 0 && rs1rdbits != 0 && rs2bits == 0)
        snprintf(insStr, MAX_LENGTH, "jalr zero,%s,0", REGISTERS5[rs1rdbits]);
    else if(bit12 == 0 && rs1rdbits != 0 && rs2bits != 0)
        snprintf(insStr, MAX_LENGTH, "add %s,zero,%s", REGISTERS5[rs1rdbits], REGISTERS5[rs2bits]);
    else if(bit12 == 1 && rs1rdbits == 0 && rs2bits == 0)
        snprintf(insStr, MAX_LENGTH, "%s", "ebreak");
    else if(bit12 == 1 && rs1rdbits != 0 && rs2bits == 0)
        snprintf(insStr, MAX_LENGTH, "jalr ra,%s,0", REGISTERS5[rs1rdbits]);
    else if(bit12 == 1 && rs1rdbits != 0 && rs2bits != 0)
        snprintf(insStr, MAX_LENGTH, "add %s,%s,%s", REGISTERS5[rs1rdbits], REGISTERS5[rs1rdbits], REGISTERS5[rs2bits]);
}

void quadrant0(char* insStr, int insCode, int offset, INSTRUCTIONS* st_ins)
{
    int subtype = insCode;
    subtype = (subtype & SUBTYPE_MASK_C) >> 13;
    switch(subtype)
    {
        case CADDI4SPN:
            CAddi4spnInstructionFormat(insStr, insCode);
            break;
        case CFLD:
            if(st_ins->d && st_ins->type != 128)
                CLSTypeInstructionFormat(insStr, "fld", insCode, 5376);
            else if(st_ins->type == 128)
                CLSTypeInstructionFormat(insStr, "lq", insCode, 54876);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case CLW:
            CLSTypeInstructionFormat(insStr, "lw", insCode, 5326);
            break;
        case CFLW:
            if(st_ins->type != 32)
                CLSTypeInstructionFormat(insStr, "ld", insCode, 5376);
            else if(st_ins->f && st_ins->type == 32)
                CLSTypeInstructionFormat(insStr, "flw", insCode, 5326);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case CFSD:
            if(st_ins->type == 128)
                CLSTypeInstructionFormat(insStr, "sq", insCode, 54876);
            else if(st_ins->d && st_ins->type != 128)
                CLSTypeInstructionFormat(insStr, "fsd", insCode, 5376);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case CSW:
            CLSTypeInstructionFormat(insStr, "sw", insCode, 5326);
            break;
        case CFSW:
            if(st_ins->type != 32)
                CLSTypeInstructionFormat(insStr, "sd", insCode, 5376);
            else if(st_ins->f && st_ins->type == 32)
                CLSTypeInstructionFormat(insStr, "fsw", insCode, 5326);
            break;
        default:
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown RVC0 instruction\0");
            break;
    }
}

void quadrant1(char* insStr, int insCode, int offset, INSTRUCTIONS* st_ins)
{
    int subtype = insCode;
    subtype = (subtype & SUBTYPE_MASK_C) >> 13;
    int test = insCode;
    test = (test & 0xF80) >> 7;
    switch(subtype)
    {
        case CADDI:
            CITypeInstructionFormat(insStr, "addi", insCode, 0);
            break;
        case CJAL:
            if(test)
                CITypeInstructionFormat(insStr, "addiw", insCode, 0);
            else 
                CJTypeInstructionFormat(insStr, "jal", insCode, offset);
            break;
        case CLI:
            CITypeInstructionFormat(insStr, "li", insCode, 0);
            break;
        case CLUI:
            if(test == 2)
                CAddi16spInstructionFormat(insStr, insCode);
            else 
                CLuiInstructionFormat(insStr, insCode);
            break;
        case CARITHMETIC:
            cArithmeticInstruction(insStr, insCode, st_ins);
            break;
        case CJ:
            CJTypeInstructionFormat(insStr, "j", insCode, offset);
            break;
        case CBEQZ:
            CBTypeInstructionFormat(insStr, "beq", insCode, offset);
            break;
        case CBNEZ:
            CBTypeInstructionFormat(insStr, "bne", insCode, offset);
            break;
        default:
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown RVC1 instruction\0");
            break;
    }
}

void quadrant2(char* insStr, int insCode, int offset, INSTRUCTIONS* st_ins)
{
    int subtype = insCode;
    subtype = (subtype & SUBTYPE_MASK_C) >> 13;
    int bit12 = insCode;
    bit12 &= 0x1000;
    int bits23456 = insCode;
    bits23456 &= 0x7C;
    int immediate = 0;
    immediate = bit12 | bits23456;
    switch(subtype)
    {
        case CSLLI:
            if((st_ins->type == 128 && immediate == 0) || immediate != 0)
                CITypeInstructionFormat(insStr, "slli", insCode, 1);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case CFLDSP:
            if(st_ins->type == 128)
                CLoadSPInstructionFormat(insStr, "lq", insCode,5496);
            else if(st_ins->d && st_ins->type != 128)
                CLoadSPInstructionFormat(insStr, "fld", insCode,54386);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case CLWSP:
            CLoadSPInstructionFormat(insStr, "lw", insCode, 54276);
            break;
        case CFLWSP:
            if(st_ins->type != 32)
                CLoadSPInstructionFormat(insStr, "ld", insCode, 54386);
            else if(st_ins->f && st_ins->type == 32)
                CLoadSPInstructionFormat(insStr, "flw", insCode, 54276);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case CADDITION:
            quadrant2Instructions100Funct(insStr, insCode, offset, st_ins);
            break;
        case CFSDSP:
            if(st_ins->type == 128)
                CStoreSPInstructionFormat(insStr, "sq", insCode, 5496);
            else if(st_ins->d && st_ins->type != 128)
                CStoreSPInstructionFormat(insStr, "fsd", insCode, 5386);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        case CSWSP:
            CStoreSPInstructionFormat(insStr, "sw", insCode, 5276);
            break;
        case CFSWSP:
            if(st_ins->type != 32)
                CStoreSPInstructionFormat(insStr, "sd", insCode, 5386);
            else if(st_ins->f && st_ins->type == 32)
                CStoreSPInstructionFormat(insStr, "fsw", insCode, 5276);
            else 
                snprintf(insStr, MAX_LENGTH, "%s", "Unknown instruction");
            break;
        default:
            snprintf(insStr, MAX_LENGTH, "%s", "Unknown RVC2 instruction\0");
            break;
    }
}

char* getRVCInstructionMnemonic(int insCode, int offset, INSTRUCTIONS* st_ins)
{
    char* instructionString = (char*)calloc(MAX_LENGTH+1, sizeof(char));
    int quadrant = insCode;
    quadrant &= QUADRANT_MASK;
    switch (quadrant)
    {
        case C0:
            quadrant0(instructionString, insCode, offset, st_ins);
            break;
        case C1:
            quadrant1(instructionString, insCode, offset, st_ins);
            break;
        case C2:
            quadrant2(instructionString, insCode, offset, st_ins);
            break;
        default:
            strncpy(instructionString, "Unknown RVC instruction\0", 25);
            break;
    } 
    return instructionString;
}