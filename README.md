# RISC-V Toolchain

Mini RISC-V toolchain for Linux consisting of compiler, simulator and disassembler. Required programs for setting up the toolchain are `flex`, `bison`, `gcc` and `make`.

## Compiler

Compiler supports a limited subset of the C language (called Micro-C) and generates RV32I assembly code.

#### Compilation

Run `make` in the `riscv-toolchain/compiler` directory.

#### Example usage

` ./micro_riscv < example-file.mc`

![image](https://user-images.githubusercontent.com/27950949/192309291-e0d495cc-fe9d-41e1-bbe9-b8ba85fd7ff5.png)

## Simulator

Simulator supports RV32I instruction set.

#### Compilation

Run `make` in the `riscv-toolchain/simulator` directory.

#### Options

Options are:
  * -r Only print the result, without running the interactive mode
  * -s <int> Maximum number of instructions a simulator can execute
  
If no options are given, simulator will run in interactive mode for the maxmimum of 2000 instructions.

#### Example usage

`./riscvsim < sum_up_to.s`

![image](https://user-images.githubusercontent.com/27950949/192308735-6ec91531-966b-46fe-9cb9-b3c2bd006e52.png)

## Disassembler

Disassembler supports RV64GC instruction sets and is made for testing wether the given program is compatible for execution on a Linux-compatible processor/microcontroller which supports given instruction sets.

### Compilation

Run `make compile` in the `riscv-toolchain/disassembler` directory.

### Options

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

### Example usage

`./rv64dis examples/example4.bin`

![image](https://user-images.githubusercontent.com/27950949/192309037-1be0c8ac-090f-42f4-9409-3a1e9080833c.png)

## License

This program is free.</br>
You can redistribute it and/or change it under the terms of **GNU General Public License version 3.0** (GPLv3).</br>
You can find the copy of the license in the repository.
