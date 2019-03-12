# rv64dis 

rv64dis is a program which disassembles RISC-V ELF files. As a command line argument you can pass the specific instruction set (or extension) that you want to disassemble and architecture type - 32-bit, 64-bit or 128-bit.

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

If no options are given the **default instruction set is RV64GC (RV64IFAMDC)**

