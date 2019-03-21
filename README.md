# rv64dis v1.4

rv64dis is a program which disassembles RISC-V ELF files. As a command line argument you can pass the specific instruction set (or extension) that you want to disassemble and architecture type - 32-bit, 64-bit or 128-bit. The program is made for testing whether a RISC-V program is compatible to be executed on a processor/microcontroller which implements given instruction set.

## Compilation

In order to compile this program you need to have **gcc compiler**(version which supports C99 standard) and **Makefile**. It is compiled using **make** command (or **sudo make install** command which requires sudo password and will install rv64dis to /usr/local/bin). After compilation, rv64dis executable will be created in your working directory. The program has been tested on Linux Ubuntu 18.10, gcc v8.2.0, make v4.2.1.

## Usage

**./rv64dis <option(s)> riscv-elf-file**

Options are:
  * -t X	Architecture type, where X is 32/64/128
  * -i		Disassembles RVXI (<em>base integer</em>) instruction set
  * -f		Disassembles RVXF (<em>single-precision, floating-point</em>) standard extension
  * -a		Disassembles RVXA (<em>atomic</em>) standard extension
  * -m		Disassembles RVXM (<em>multiplication</em>) standard extension
  * -d		Disassembles RVXD (<em>double-precision, floating-point, requires F standard extension</em>) standard extension
  * -c		Disassembles RVXC (<em>compressed</em>) instruction set
  * -e		Disassembles RV32E (<em>can only be combined with M,A,C standard extensions, used on embedded systems</em>) instruction set
  * -q		Disassembles RV128Q (<em>quad-precision, floating-point, requires RV64IFD instruction set</em>) extension
  * -h  Show help

If no options are given the **default instruction set is RV64GC (RV64IFAMDC)**

Program creates a table which has the following columns:
1. Number - ordinal number of the instruction
2. Address - virtual address of the instructions
3. Instructions - hexadecimal representation of the instruction
4. Legnth - length of the instruction - 32-bit or 16-bit
5. RISC-V Instruction - RISC-V assembly base instruction 

After printing the table with instruction data, program prints the total number of disassembled instructions. If the percentage of disassembled instructions is 100% that means that the given RISC-V program can be executed on a processor/microcontroller which implements the given instruction set.
In the foler **examples** are 6 executable RISC-V ELF files:
  * **example.bin** - program calculates the greatest common divider of 12 and 8, file does not contain section .text and only consists of instructions from RV32I instruction set
  * **example2.bin** - program calculates the greatest common divider of 12 and 8, file contains section .text and only consists of instructions from RV32I instruction set
  * **example3.bin** - program calculates the greatest common divider of 12 and 8, file does not contain section .text and only consists of instructions from RV32IC instruction set
  * **example4.bin** - program which is a result of inserting a non-executable program segment in the file example.bin
  * **example5.bin** - program made for testing, file does contain sectinon .text and contains instructions from RV64IFAMD istruction set
  * **example6.bin** - rv64dis compiled with RISC-V compiler (<em>pre-built toolchain: riscv-linux-gnu-gcc</em>)
  
## How does it work?

If the given file is valid program checks whether the file is 32-bit or 64-bit by checking ELF Header - **<em>e_ident[EI_CLASS]</em>** and based on that it continues to search for a program segment which contains instructions.</br>
Firstly, program checks if the file has sections. If not, it searches for program segments which have **<em>execute flag</em>** (1) set. The found program segment is then analized <em>word</em> by <em>word</em> (2 bytes) - if the 2 lowest bytes of the instructions are 11 then the instruction's length is 32-bit, else if they are 00, 10 or 01 the instruction is 16-bit long.</br>
If, on the other hand, the given file has sections, program searches for a section which contains the string with the names of all the sections (section with index **<em>e_shstrndx</em>**) and in that string it searches for a substring **<em>".text"</em>**, and the index which corresponds to the index of the beginning of the substring is the index of the sections .text which contains instructions, which are then processed as in the first case.</br>
When the program finds the program segment with the instructions it disassembles only the instructions which belong to the given instruction set.

## License

This program is free.</br>
You can redistribute it and/or change it under the terms of **GNU General Public License version 3.0** (GPLv3).</br>
You can find the copy of the license in the repository.
