// author: Dusan Erdeljan
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
#include <unistd.h>
#include <getopt.h>
#include <inttypes.h>
#include "../inc/decodingRVG.h"
#include "../inc/decodingRVC.h"

// Ansi colors escape codes
#define RED "\x1B[31m"
#define CYAN "\x1B[36m"
#define RESET "\x1B[0m"

// ELF header
#define FORMAT_OFFSET 0x4
#define ISA_OFFSET 0x12
#define ENTRY_POINT_OFFSET 0x18

// 64-bit ELF header
#define SECTION_ENTRIES_OFFSET 0x3c
#define PROGRAM_HTSTART_OFFSET 0x20
#define PROGRAM_HEADER_ENTRIES 0x38
#define NAMES_SECTION_OFFSET 0x3e
#define SECTION_HEADER_OFFSET 0x28
#define ELF_HTSIZE_OFFSET 0x34
#define PROGRAM_HTSIZE_OFFSET 0x36
#define SECTION_HTSIZE_OFFSET 0x3a

// 32-bit ELF header
#define SECTION_ENTRIES_OFFSET_32 0x30
#define PROGRAM_HTSTART_OFFSET_32 0x1c
#define PROGRAM_HEADER_ENTRIES_32 0x2c
#define NAMES_SECTION_OFFSET_32 0x32
#define SECTION_HEADER_OFFSET_32 0x20
#define ELF_HTSIZE_OFFSET_32 0x28
#define PROGRAM_HTSIZE_OFFSET_32 0x2a
#define SECTION_HTSIZE_OFFSET_32 0x2e

// 64-bit Program header 
#define VIRTUAL_ADDRESS_OFFSET 0x10
#define FILESZ_OFFSET 0x20
#define FLAGS_OFFSET 0x04

// 32-bit Program header
#define VIRTUAL_ADRESS_OFFSET_32 0x08
#define FILESZ_OFFSET_32 0x10
#define FLAGS_OFFSET_32 0x18

// 64-bit Section header
#define SH_OFFSET 0x18
#define SH_SIZE 0x20

// 32-bit Section header
#define SH_OFFSET_32 0x10
#define SH_SIZE_32 0x14

int ELF_FILE_TYPE;


int getFileSize(FILE* ptr)
{
    fseek(ptr, 0, SEEK_END);
    int size = ftell(ptr);
    fseek(ptr, 0, SEEK_SET);
    return size;
}

int noInstructionSets(INSTRUCTIONS* st_ins)
{
    if(st_ins->i || st_ins->f || st_ins->a || st_ins->d || st_ins->c || st_ins->m || st_ins->e || st_ins->q)
        return 0;
    return 1;
}

void printInstructionSet(INSTRUCTIONS* st_ins)
{
    char* isaStr = (char*)calloc(30, sizeof(char));
    char* istr = st_ins->i ? "I" : "";
    char* estr = st_ins->e ? "E" : "";
    char* fstr = st_ins->f ? "F" : "";
    char* astr = st_ins->a ? "A" : "";
    char* mstr = st_ins->m ? "M" : "";
    char* dstr = st_ins->d ? "D" : "";
    char* qstr = st_ins->q ? "Q" : "";
    char* cstr = st_ins->c ? "C" : "";
    snprintf(isaStr, 29, "RV%d%s%s%s%s%s%s%s%s", st_ins->type, istr, estr, fstr, astr, mstr, dstr, qstr ,cstr);
    printf("\nInstruction set: %s\n", isaStr);
    free(isaStr);
}

void setInstructionsSetsToOne(INSTRUCTIONS* st_ins)
{
    st_ins->i = 1;
    st_ins->f = 1;
    st_ins->a = 1;
    st_ins->m = 1;
    st_ins->d = 1;
    st_ins->c = 1;
    st_ins->e = 0;
    st_ins->q = 0;
}

void hexdump(unsigned char* buffer, int offset, int n)
{
    int j = 0;
    for(int i = offset; i < offset+n; i++)
    {
        printf("%02x ", buffer[i]);
        if(++j % 16 == 0){
            printf("\n");
        }
    } 
    printf("\n");
}

uint64_t get64bitInteger(unsigned char* buffer, int startIndex)
{
    uint64_t value = buffer[startIndex] |
    (buffer[startIndex+0x1] << 8) | (buffer[startIndex+0x2] << 16) | 
    (buffer[startIndex+0x3] << 24) | ((long long)buffer[startIndex+0x4] << 32) |
    ((long long)buffer[startIndex+0x5] << 40) | ((long long)buffer[startIndex+0x6] << 48) | 
    ((long long)buffer[startIndex+0x7] << 56);
    return value;
}

uint32_t get32bitInteger(unsigned char* buffer, int startIndex)
{
    uint32_t value = buffer[startIndex] |
    (buffer[startIndex+0x1] << 8) | (buffer[startIndex+0x2] << 16) | 
    (buffer[startIndex+0x3] << 24);
    return value;
}

uint16_t get16bitInteger(unsigned char* buffer, int startIndex)
{
    uint16_t value = buffer[startIndex] |(buffer[startIndex+0x1] << 8);
    return value;
}

int isValidFile(unsigned char* buffer)
{
    if(buffer[0] == 0x7f && buffer[1] == 0x45 && buffer[2] == 0x4c && buffer[3] == 0x46){
        if ((buffer[ISA_OFFSET] == 0xf3) && (buffer[FORMAT_OFFSET] == 0x2 || buffer[FORMAT_OFFSET] == 0x1)){
            ELF_FILE_TYPE = buffer[FORMAT_OFFSET] == 0x2 ? 64 : 32;
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

int getNumInustructions(unsigned char* buffer, int offset, int length, INSTRUCTIONS* st_ins)
{
    printf("\n  Number  Address     Instruction  Length   RISC-V Instruction\n");
    printf("--------------------------------------------------------------------------\n");
    int numInstructions = 0;
    uint64_t entryPoint = ELF_FILE_TYPE == 64 ? get64bitInteger(buffer, ENTRY_POINT_OFFSET) : get32bitInteger(buffer, ENTRY_POINT_OFFSET);
    int unknowns = 0;
    for(int i = offset; i < (offset+length) - 1; i+=2)
    {
        int insCode = buffer[i] | ((int)buffer[i+1] << 8);
        // if the binary form of the instruction hex code ends with 11 -> 32bit
        if(insCode % 4 == 3){
            insCode |= (int)buffer[i+2] << 16 | (int)buffer[i+3] << 24;
            numInstructions++;
            //char* str = decimalToBinary(insCode, 32);
            char* insFormat = getRV32IInstructionMnemonic(insCode, entryPoint+(i-offset), st_ins);
            if(strstr(insFormat, "Unknown") != NULL){
                unknowns++;
                printf("%s  %-6d  0x%-8lx  0x%08x   32-bit   %s%s\n",CYAN,numInstructions,entryPoint+(i-offset),insCode, insFormat, RESET);
            } else {
                printf("  %-6d  0x%-8lx  0x%08x   32-bit   %s\n", numInstructions,entryPoint+(i-offset),insCode, insFormat);
            }
            //free(str);
            free(insFormat);
            i += 2;
        }
        // binary form of the instruction hex code ends with 01, 10, 00 -> 16bit 
        else {
            numInstructions++;
            if(!st_ins->c){
                printf("%s  %-6d  0x%-8lx  0x%04x       16-bit   Unknown instruction%s\n", CYAN, numInstructions,entryPoint+(i-offset),insCode, RESET);
                unknowns++;
                continue;
            }
            //char* str = decimalToBinary(insCode, 16);
            char* insFormat = getRVCInstructionMnemonic(insCode, entryPoint+(i-offset), st_ins);
            if(strstr(insFormat, "Unknown") != NULL){
                unknowns++;
                printf("%s  %-6d  0x%-8lx  0x%04x       16-bit   %s%s\n", CYAN, numInstructions,entryPoint+(i-offset),insCode, insFormat, RESET);
            } else {
                printf("  %-6d  0x%-8lx  0x%04x       16-bit   %s\n", numInstructions,entryPoint+(i-offset),insCode, insFormat);
            }
            free(insFormat);
            //free(str);
        }
    }
    printInstructionSet(st_ins);
    printf("\nNumber of disassembled instructions: %d (%d%%)\n", (numInstructions-unknowns), (int)(((float)(numInstructions-unknowns)/numInstructions)*100));
    return numInstructions;
}

int countInstructionsForNoSectionFile(unsigned char* buffer, INSTRUCTIONS* st_ins)
{
    int instructions = 0;
    // get the length of ELF header
    uint16_t elfHeaderSize = ELF_FILE_TYPE == 64 ? get16bitInteger(buffer, ELF_HTSIZE_OFFSET) : get16bitInteger(buffer, ELF_HTSIZE_OFFSET_32);
    // get the number of program header tables in the file
    uint16_t programHeaderEntries = ELF_FILE_TYPE == 64 ? get16bitInteger(buffer, PROGRAM_HEADER_ENTRIES) : get16bitInteger(buffer, PROGRAM_HEADER_ENTRIES_32);
    // find where the program header table begins
    uint64_t programHTStart = ELF_FILE_TYPE == 64 ? get64bitInteger(buffer, PROGRAM_HTSTART_OFFSET) : get32bitInteger(buffer, PROGRAM_HTSTART_OFFSET_32);
    // find the size of a single program header table entry
    uint16_t programHeaderEntrySize = ELF_FILE_TYPE == 64 ? get16bitInteger(buffer, PROGRAM_HTSIZE_OFFSET) : get16bitInteger(buffer, PROGRAM_HTSIZE_OFFSET_32);
    // the first program header table begins at the offset programHTStart, and the length of a single program header table is programHeaderEntrySize bytes
    // check if the current program header table has EXECUTE flag on (E flag = 1 -> flags % 2 == 1)
    for(int i = programHTStart; i < programHTStart + programHeaderEntries*programHeaderEntrySize; i += programHeaderEntrySize){
        int flags = ELF_FILE_TYPE == 64 ? get32bitInteger(buffer, i+FLAGS_OFFSET) : get32bitInteger(buffer, i+FLAGS_OFFSET_32);
        if (flags % 2 == 1){
            // get the size of the current segment in memory
            uint64_t fileSize = ELF_FILE_TYPE == 64 ? get64bitInteger(buffer, i + FILESZ_OFFSET) : get32bitInteger(buffer, i + FILESZ_OFFSET_32);
            // get the number of bytes that instructions occupy
            int lengthOfInstrucions = fileSize - elfHeaderSize - programHeaderEntries*programHeaderEntrySize;
            // find the offset to the beginnig of instrucions
            int startOfInstructions = fileSize - lengthOfInstrucions;
            // count the number of instructions
            instructions += getNumInustructions(buffer, startOfInstructions, lengthOfInstrucions, st_ins);
        }
    }
    return instructions;
}

int countInstructions(unsigned char* buffer, int size, int numSectinos, INSTRUCTIONS* st_ins)
{
    // get the index of section with the names
    int index = ELF_FILE_TYPE == 64 ? get16bitInteger(buffer, NAMES_SECTION_OFFSET) : get16bitInteger(buffer, NAMES_SECTION_OFFSET_32);
    // get the offset to the beggining of the section header
    uint64_t sectionHeaderOffset = ELF_FILE_TYPE == 64 ? get64bitInteger(buffer, SECTION_HEADER_OFFSET) : get32bitInteger(buffer, SECTION_HEADER_OFFSET_32);
    // get the size of a single entry in section header table
    uint16_t sectionEntrySize = ELF_FILE_TYPE == 64 ? get16bitInteger(buffer, SECTION_HTSIZE_OFFSET) : get16bitInteger(buffer, SECTION_HTSIZE_OFFSET_32);
    // jump to the offset: sectionHeaderOffset + sectionEntrySize * index -> names
    uint64_t namesOffset = sectionHeaderOffset + sectionEntrySize*index;
    // get the offset to the begginig of the string
    uint64_t stringOffset = ELF_FILE_TYPE == 64 ? get64bitInteger(buffer, namesOffset + SH_OFFSET) : get32bitInteger(buffer, namesOffset + SH_OFFSET_32);
    // get the size of the string
    uint64_t stringSize = ELF_FILE_TYPE == 64 ? get64bitInteger(buffer, namesOffset+SH_SIZE) : get32bitInteger(buffer, namesOffset+SH_SIZE_32);
    // find the index where '.text' begins
    int relativeDistance = 0;
    for(int i = stringOffset; i < stringOffset + stringSize - 5; i++){
        if ((buffer[i] == '.') && (buffer[i+1] == 't') && (buffer[i+2] == 'e') && (buffer[i+3] == 'x') && (buffer[i+4] == 't')){
            relativeDistance = i - stringOffset;
        }
    }
    // find section which name index == relativeDistance -> section .text
    for(int i = 0; i < numSectinos; i++){
        long long newOffset = sectionHeaderOffset + i*sectionEntrySize;
        uint32_t nameIndex = get32bitInteger(buffer, newOffset);
        // Program data = 0x1
        if((nameIndex == relativeDistance) && get32bitInteger(buffer, newOffset+4) ==0x1){
            // find the offset to the beggining of instructions
            uint64_t textOffset = ELF_FILE_TYPE == 64 ? get64bitInteger(buffer, newOffset+SH_OFFSET) : get32bitInteger(buffer, newOffset+SH_OFFSET_32);
            // find the size of the section .text
            uint64_t sizeText = ELF_FILE_TYPE == 64 ? get64bitInteger(buffer, newOffset+SH_SIZE) : get32bitInteger(buffer, newOffset+SH_SIZE_32);
            //hexdump(buffer, textOffset, sizeText);
            // count the number of instructions
            return getNumInustructions(buffer, textOffset, sizeText, st_ins);
        }
    }
    return -1;
}

void printUsage()
{
    printf("Usage: ./rv64dis <option(s)> riscv64-elf-file\n");
    printf("If you have installed the programm using \"sudo make install\" command you can run it as: rv64dis <option(s)> riscv64-elf-file\n");
    printf("  Options are:\n");
    printf("    -tX\tArchitecture type, where X is 32/64/128\n");
    printf("    -i\tDisassembles RVXI (base integer) instruction set\n");
    printf("    -f\tDisassembles RVXF (single-precision, floating-point) standard extension\n");
    printf("    -a\tDisassembles RVXA (atomic) standard extension\n");
    printf("    -m\tDisassembles RVXM (multiplication) standard extension\n");
    printf("    -d\tDisassembles RVXD (double-precision, floating-point, requires F standard extension) standard extension\n");
    printf("    -c\tDisassembles RVXC (compressed) instruction set\n");
    printf("    -e\tDisassembles RV32E (can only be combined with M,A,C standard extensions, used on embedded systems) instruction set\n");
    printf("    -q\tDisassembles RV128Q (floating-point, requires RV64IFD instruction set) extension\n");
    printf("    -h\tPrint usage\n");
    printf("  If no options are given the default instruction set is RV64IFAMDC (RV64GC)\n");
    exit(2);
}

int validArguments(INSTRUCTIONS* st_ins)
{
    if(!(st_ins->type == 32 || st_ins->type == 64 || st_ins->type == 128))
    {
        printf("%sError:%s Architecture type must be 32/64/128.\n", RED, RESET);
        return 0;
    }
    if(st_ins->i)
    {
        if(st_ins->e)
        {
            printf("%sError:%s Can't use RV%dI Instruction set with RV32E Instruction set.\n", RED, RESET, st_ins->type);
            return 0;
        }
    }
    if(st_ins->f)
    {
        if(st_ins->e)
        {
            printf("%sError:%s Can't use RV%dF Instruction set with RV32E Instruction set.\n", RED, RESET, st_ins->type);
            return 0;
        }
    }
    if(st_ins->d)
    {
        if(st_ins->e)
        {
            printf("%sError:%s Can't use RV%dD Instruction set with RV32E Instruction set.\n", RED, RESET, st_ins->type);
            return 0;
        }
        if(!st_ins->f)
        {
            printf("%sError:%s Can't use RV%dD Instruction without RV%dF Instruction set.\n", RED, RESET, st_ins->type, st_ins->type);
            return 0;
        }
    }
    if(st_ins->q)
    {
        if(st_ins->type != 128)
        {
            printf("%sError:%s Can't use RV%d Instruction sets with RV128Q Instruction set.\n", RED,RESET, st_ins->type);
            return 0;
        }
        if(!(st_ins->f || st_ins->q || st_ins->i))
        {
            printf("%sError:%s Can't use RV128Q Instruction without RV64IFD Instruction set.\n", RED, RESET);
            return 0;
        }
        if(st_ins->e)
        {
            printf("%sError:%s Can't use RV%dQ Instruction set with RV32E Instruction set.\n", RED, RESET, st_ins->type);
            return 0;
        }
    }
    if(st_ins->e)
    {
        if(st_ins->type != 32)
        {
            printf("%sError:%s Can't use RV%d Instruction sets with RV32E Instruction set.\n", RED,RESET, st_ins->type);
            return 0;
        }
        if(st_ins->i)
        {
            printf("%sError:%s Can't use RV32E Instruction set with RV32I Instruction set.\n", RED, RESET);
            return 0;
        }
        if(st_ins->d)
        {
            printf("%sError:%s Can't use RV32E Instruction set with RV32D Instruction set.\n", RED, RESET);
            return 0;
        }
        if(st_ins->f)
        {
            printf("%sError:%s Can't use RV32E Instruction set with RV32F Instruction set.\n", RED, RESET);
            exit(EXIT_FAILURE);
        }
        if(st_ins->q)
        {
            printf("%sError:%s Can't use RV32E Instruction set with RV128Q Instruction set.\n", RED, RESET);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int instructions = 0;
    INSTRUCTIONS* st_ins = (INSTRUCTIONS*)malloc(sizeof(INSTRUCTIONS));
    st_ins->type=64;
    if(argc < 2)
    {
        setInstructionsSetsToOne(st_ins);
        st_ins->type = 64;
    }
    int option;
    // Parse optional arguments
    while((option = getopt(argc, argv, "ifamdceqht:")) != -1)
    {
        switch(option)
        {
            case 'i':
                st_ins->i = 1;
                break;
            case 'f':
                st_ins->f = 1;
                break;
            case 'a':
                st_ins->a = 1;
                break;
            case 'm':
                st_ins->m = 1;
                break;
            case 'd':
                st_ins->d = 1;
                break;
            case 'c':
                st_ins->c = 1;
                break;
            case 'q':
                st_ins->type = 128;
                st_ins->q = 1;
                break;
            case 'e':
                st_ins->e = 1;
                st_ins->type = 32;
                break;
            case 't':
                st_ins->type = atoi(optarg);
                break;
            case 'h':
            default:
                printUsage();
        }
    }
    if(!validArguments(st_ins))
        exit(EXIT_FAILURE);
    if(noInstructionSets(st_ins))
        setInstructionsSetsToOne(st_ins);
    argc -= optind;
    argv += optind;
    const char* fname = argv[0];
    if(fname == NULL)
        printUsage();
    FILE* ptr = fopen(fname, "r");
    if(ptr != NULL){
        int size = getFileSize(ptr);
        unsigned  char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * size);
        fread(buffer, sizeof(unsigned char) * size, 1, ptr);
        fclose(ptr);
        if (isValidFile(buffer) == 0){
            printf("%sError%s: File \'%s\' is not in RISCV64 ELF format!\n", RED,RESET, fname);
            exit(EXIT_FAILURE);
        }
        if (buffer[SECTION_ENTRIES_OFFSET] == 0){
            instructions = countInstructionsForNoSectionFile(buffer, st_ins);
        } else {
            instructions = countInstructions(buffer, size, (int)buffer[SECTION_ENTRIES_OFFSET], st_ins);
        }
        printf("\nNumber of machine instructions in the file \'%s\': %d\n", fname, instructions);
        free(buffer);
        free(st_ins);
    } else {
        printf("%sError%s: File \'%s\' does not exists in the current directory.\n", RED,RESET, fname);
        exit(EXIT_FAILURE);
    }
    return 0;
}
