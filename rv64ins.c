// author: Dusan Erdeljan SW-43/2018
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
#include "decodingRV32.h"
#include "decodingRVC.h"

// Ansi colors escape codes
#define RED "\x1B[31m"
#define CYAN "\x1B[36m"
#define RESET "\x1B[0m"

// ELF header
#define FORMAT_OFFSET 0x4
#define ISA_OFFSET 0x12
#define SECTION_ENTRIES_OFFSET 0x3c
#define ENTRY_POINT_OFFSET 0x18
#define PROGRAM_HTSTART_OFFSET 0x20
#define PROGRAM_HEADER_ENTRIES 0x38
#define NAMES_SECTION_OFFSET 0x3e
#define SECTION_HEADER_OFFSET 0x28
#define ELF_HTSIZE_OFFSET 0x34
#define PROGRAM_HTSIZE_OFFSET 0x36
#define SECTION_HTSIZE_OFFSET 0x3a

// Program header 
#define VIRTUAL_ADDRESS_OFFSET 0x10
#define FILESZ_OFFSET 0x20


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

long long get64bitInteger(unsigned char* buffer, int startIndex)
{
    long long value = buffer[startIndex] |
    (buffer[startIndex+0x1] << 8) | (buffer[startIndex+0x2] << 16) | 
    (buffer[startIndex+0x3] << 24) | ((long long)buffer[startIndex+0x4] << 32) |
    ((long long)buffer[startIndex+0x5] << 40) | ((long long)buffer[startIndex+0x6] << 48) | 
    ((long long)buffer[startIndex+0x7] << 56);
    return value;
}

int get32bitInteger(unsigned char* buffer, int startIndex)
{
    int value = buffer[startIndex] |
    (buffer[startIndex+0x1] << 8) | (buffer[startIndex+0x2] << 16) | 
    (buffer[startIndex+0x3] << 24);
    return value;
}

int get16bitInteger(unsigned char* buffer, int startIndex)
{
    int value = buffer[startIndex] |(buffer[startIndex+0x1] << 8);
    return value;
}

int isValidFile(unsigned char* buffer)
{
    if(buffer[0] == 0x7f && buffer[1] == 0x45 && buffer[2] == 0x4c && buffer[3] == 0x46){
        if ((buffer[ISA_OFFSET] == 0xf3) && (buffer[FORMAT_OFFSET] == 0x2)){
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
    int entryPoint = get64bitInteger(buffer, ENTRY_POINT_OFFSET);
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
                printf("%s  %-6d  0x%-8x  0x%08x   32-bit   %s%s\n",CYAN,numInstructions,entryPoint+(i-offset),insCode, insFormat, RESET);
            } else {
                printf("  %-6d  0x%-8x  0x%08x   32-bit   %s\n", numInstructions,entryPoint+(i-offset),insCode, insFormat);
            }
            //free(str);
            free(insFormat);
            i += 2;
        }
        // binary form of the instruction hex code ends with 01, 10, 00 -> 16bit 
        else {
            numInstructions++;
            if(!st_ins->c){
                printf("%s  %-6d  0x%-8x  0x%04x       16-bit   Unknown instruction%s\n", CYAN, numInstructions,entryPoint+(i-offset),insCode, RESET);
                unknowns++;
                continue;
            }
            //char* str = decimalToBinary(insCode, 16);
            char* insFormat = getRVCInstructionMnemonic(insCode, entryPoint+(i-offset), st_ins);
            if(strstr(insFormat, "Unknown") != NULL){
                unknowns++;
                printf("%s  %-6d  0x%-8x  0x%04x       16-bit   %s%s\n", CYAN, numInstructions,entryPoint+(i-offset),insCode, insFormat, RESET);
            } else {
                printf("  %-6d  0x%-8x  0x%04x       16-bit   %s\n", numInstructions,entryPoint+(i-offset),insCode, insFormat);
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
    int elfHeaderSize = get16bitInteger(buffer, ELF_HTSIZE_OFFSET);
    // get the number of program header tables in the file
    int programHeaderEntries = get16bitInteger(buffer, PROGRAM_HEADER_ENTRIES);
    // find where the program header table begins
    long long programHTStart = get64bitInteger(buffer, PROGRAM_HTSTART_OFFSET);
    // find the size of a single program header table entry
    int programHeaderEntrySize = get16bitInteger(buffer, PROGRAM_HTSIZE_OFFSET);
    // the first program header table begins at the offset programHTStart, and the length of a single program header table is programHeaderEntrySize bytes
    // check if the current program header table has EXECUTE flag on (E flag = 1 -> flags % 2 == 1)
    for(int i = programHTStart; i < programHTStart + programHeaderEntries*programHeaderEntrySize; i += programHeaderEntrySize){
        int flags = get32bitInteger(buffer, i+0x04);
        if (flags % 2 == 1){
            // get the size of the current segment in memory
            long long fileSize = get64bitInteger(buffer, i + FILESZ_OFFSET);
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
    int index = (int)buffer[NAMES_SECTION_OFFSET];
    // get the offset to the beggining of the section header
    long long sectionHeaderOffset = get64bitInteger(buffer, SECTION_HEADER_OFFSET);
    // get the size of a single entry in section header table
    int sectionEntrySize = get16bitInteger(buffer, SECTION_HTSIZE_OFFSET);
    // jump to the offset: sectionHeaderOffset + sectionEntrySize * index -> names
    long long namesOffset = sectionHeaderOffset + sectionEntrySize*index;
    // get the offset to the begginig of the string
    long long stringOffset = get64bitInteger(buffer, namesOffset + 24);
    // get the size of the string
    long long stringSize = get64bitInteger(buffer, namesOffset+32);
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
        int nameIndex = get32bitInteger(buffer, newOffset);
        if((nameIndex == relativeDistance) && buffer[newOffset+4] ==0x1){
            // find the offset to the beggining of instructions
            long long textOffset = get64bitInteger(buffer, newOffset+24);
            // find the size of the section .text
            long long sizeText = get64bitInteger(buffer, newOffset+32);
            //hexdump(buffer, textOffset, sizeText);
            // count the number of instructions
            return getNumInustructions(buffer, textOffset, sizeText, st_ins);
        }
    }
    return -1;
}

void printUsage()
{
    printf("Usage: ./rv64dis [Instruction set extensions(ifamdcqe)] [type = {32, 64, 128}] file_name\n");
    exit(2);
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
    while((option = getopt(argc, argv, "ifamdceqt:")) != -1)
    {
        switch(option)
        {
            case 'i':
                if(st_ins->e){
                    printf("%sError:%s Can't use RV%dI Instruction set with RV32E Instruction set.\n", RED, RESET, st_ins->type);
                    exit(EXIT_FAILURE);
                }
                st_ins->i = 1;
                break;
            case 'f':
                if(st_ins->e){
                    printf("%sError:%s Can't use RV%dF Instruction set with RV32E Instruction set.\n", RED, RESET, st_ins->type);
                    exit(EXIT_FAILURE);
                }
                st_ins->f = 1;
                break;
            case 'a':
                st_ins->a = 1;
                break;
            case 'm':
                st_ins->m = 1;
                break;
            case 'd':
                if(st_ins->e){
                    printf("%sError:%s Can't use RV%dD Instruction set with RV32E Instruction set.\n", RED, RESET, st_ins->type);
                    exit(EXIT_FAILURE);
                }
                if(!st_ins->f){
                    printf("%sError:%s Can't use RV%dD Instruction without RV%dF Instruction set.\n", RED, RESET, st_ins->type, st_ins->type);
                    exit(EXIT_FAILURE);
                }
                st_ins->d = 1;
                break;
            case 'c':
                st_ins->c = 1;
                break;
            case 'q':
                if(!(st_ins->f || st_ins->q || st_ins->i)){
                    printf("%sError:%s Can't use RV128Q Instruction without RV64IFD Instruction set.\n", RED, RESET);
                    exit(EXIT_FAILURE);
                }
                if(st_ins->e){
                    printf("%sError:%s Can't use RV%dQ Instruction set with RV32E Instruction set.\n", RED, RESET, st_ins->type);
                    exit(EXIT_FAILURE);
                }
                st_ins->q = 1;
                st_ins->type = 128;
                break;
            case 'e':
                if(st_ins->i){
                    printf("%sError:%s Can't use RV32E Instruction set with RV32I Instruction set.\n", RED, RESET);
                    exit(EXIT_FAILURE);
                }
                if(st_ins->d){
                    printf("%sError:%s Can't use RV32E Instruction set with RV32D Instruction set.\n", RED, RESET);
                    exit(EXIT_FAILURE);
                }
                if(st_ins->f){
                    printf("%sError:%s Can't use RV32E Instruction set with RV32F Instruction set.\n", RED, RESET);
                    exit(EXIT_FAILURE);
                }
                if(st_ins->q){
                    printf("%sError:%s Can't use RV32E Instruction set with RV128Q Instruction set.\n", RED, RESET);
                    exit(EXIT_FAILURE);
                }
                st_ins->e = 1;
                st_ins->type = 32;
                break;
            case 't':
                st_ins->type = atoi(optarg);
                if(!(st_ins->type == 32 || st_ins->type == 64 || st_ins->type == 128)){
                    printUsage();
                }
                if(st_ins->e && st_ins->type != 32){
                    printf("%sError:%s Can't use RV%d Instruction sets with RV32E Instruction set.\n", RED,RESET, st_ins->type);
                    exit(EXIT_FAILURE);
                }
                if(st_ins->q && st_ins->type != 128){
                    printf("%sError:%s Can't use RV%d Instruction sets with RV128Q Instruction set.\n", RED,RESET, st_ins->type);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                printUsage();
        }
    }
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
