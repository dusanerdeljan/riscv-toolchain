# rv64dis 

rv64dis is a program which disassembles RISC-V ELF files. As a command line argument you can pass the specific instruction set (or extension) that you want to disassemble and architecture type - 32-bit, 64-bit or 128-bit.

## Usage

**./rv64dis <option(s)> riscv-elf-file**

Options are:
	-t X	Architecture type, where X is 32/64/128
	-i	Disassembles RVXI (base integer) instruction set
	-f	Disassembles RVXF (single-precision, floating-point) standard extension
	-a	Disassembles RVXA (atomic) standard extension
	-m	Disassembles RVXM (multiplication) standard extension
	-d	Disassembles RVXD (double-precision, floating-point, requires F standard extension) standard extension
	-c	Disassembles RVXC (compressed) instruction set
	-e	Disassembles RV32E (can only be combined with M,A,C standard extensions, used on embedded systems) instruction set
	-q	Disassembles RV128Q (quad-precision, floating-point, requires RV64IFD instruction set) extension

If no options are given the default instruction set is RV64GC (RV64IFAMDC)

