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

// Compressed(RVC) instruction set
#ifndef DECODINGRVC_H
#define DECODINGRVC_H

#include "instruction.h"

// Quadrants
#define C0 0
#define C1 1
#define C2 2

// Quadrant 0 subtypes
#define CADDI4SPN 0
#define CFLD 1
#define CLW 2
#define CFLW 3 
#define CFSD 5
#define CSW 6
#define CFSW 7

// Quadrant 1 subtypes
#define CADDI 0 // c.addi, c.nop
#define CJAL 1
#define CLI 2
#define CLUI 3
#define CARITHMETIC 4 //c.srli, c.srai, c.andi, c.sub, c.xor, c.or, c.and, c.subw(bit 12=1), c.addw(bit 12=1)
#define CJ 5
#define CBEQZ 6
#define CBNEZ 7

// Quadrant 2 subtypes
#define CSLLI 0
#define CFLDSP 1
#define CLWSP 2
#define CFLWSP 3
#define CADDITION 4 // c.mv, c.add, c.ebreak, c.jalr, c.jr
#define CFSDSP 5
#define CSWSP 6
#define CFSWSP 7

#define QUADRANT_MASK 0x3
#define SUBTYPE_MASK_C 0xE000

#define MAX_LENGTH 50

char* getRVCInstructionMnemonic(int insCode, int offset, INSTRUCTIONS* st_ins);

//Quadrants
void quadrant0(char* insStr, int insCode, int offset, INSTRUCTIONS* st_ins);
void quadrant1(char* insStr, int insCode, int offset, INSTRUCTIONS* st_ins);
void quadrant2(char* insStr, int insCode, int offset, INSTRUCTIONS* st_ins);

void cArithmeticInstruction(char* insStr, int insCode, INSTRUCTIONS* st_ins);
void quadrant2Instructions100Funct(char* insStr, int insCode, int offset, INSTRUCTIONS* st_ins);

// RVC instruction formats
void CITypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int uimm);
void CITypeInstructionFormatShortRegisters(char* insStr, const char* mnemonic, int ins, int uimm);
void CThreeRegisterInstructionFormat(char* insStr, const char* mnemonic, int ins);
void CITypeInstructionFormatShortRegistersZeroImmediate(char* insStr, const char* mnemonic, int ins);
void CJTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int offset);
void CBTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int offset);
void CLuiInstructionFormat(char* insStr, int ins);
void CAddi16spInstructionFormat(char* insStr, int ins);
void CAddi4spnInstructionFormat(char* insStr, int ins);
void CLSTypeInstructionFormat(char* insStr, const char* mnemonic, int ins, int uimmPattern);
void CStoreSPInstructionFormat(char* insStr, const char* mnemonic, int ins, int uimmPattern);
void CLoadSPInstructionFormat(char* insStr, const char* mnemonic, int ins, int uimmPattern);


#endif
