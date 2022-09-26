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

// CSR list
#ifndef CSR_H
#define CSR_H

char* decodeCSR(int csrCode);

// Counters
#define CYCLE 0xC00
#define MCYCLE 0xB00
#define CYCLEH 0xC80
#define MCYCLEH 0xB80

#define TIME 0xC01
#define TIMEH 0xC81

#define INSTRET 0xC02
#define MINSTRET 0xB02
#define INSTRETH 0xC82
#define MINSTRETH 0xB82

// Exception Processing
#define USTATUS 0x000
#define SSTATUS 0x100
#define MSTATUS 0x300

#define UIE 0x004
#define SIE 0x104
#define MIE 0x304

#define UTVEC 0x005
#define STVEC 0x105
#define MTVEC 0x305

#define SEDELEG 0x102
#define MEDELEG 0x302

#define SIDELEG 0x103
#define MIDELEG 0x303

#define SCOUNTEREN 0x106
#define MCOUNTEREN 0x306

// Trap Handling
#define USCRATCH 0x040
#define SSCRATCH 0x140
#define MSCRATCH 0x340

#define UEPC 0x041
#define SEPC 0x141
#define MEPC 0x341

#define UCAUSE 0x042
#define SCAUSE 0x142
#define MCAUSE 0x342

#define UTVAL 0x043
#define STVAL 0x143
#define MTVAL 0x343

#define UIP 0x044
#define SIP 0x144
#define MIP 0x344

// Virtual Memory
#define SATP 0x180

// Informational
#define MISA 0x301
#define MVENDROID 0xF11
#define MARCHID 0xF12
#define MIMPID 0xF13
#define MHARDID 0xF14

// Floatinf Point
#define FFLAGS 0x001
#define FRM 0x002
#define FCSR 0x003

// Performance Monitoring
#pragma region PerformanceMonitoring
#define MHPMEVENT3 0x323
#define HPMCOUNTER3 0xC03
#define MHMCOUNTER3 0xB03
#define HPMCOUNTER3H 0xC83
#define MPHMCOUNTER3H 0xB83

#define MHPMEVENT4 0x324
#define HPMCOUNTER4 0xC04
#define MHMCOUNTER4 0xB04
#define HPMCOUNTER4H 0xC84
#define MPHMCOUNTER4H 0xB84

#define MHPMEVENT5 0x325
#define HPMCOUNTER5 0xC05
#define MHMCOUNTER5 0xB05
#define HPMCOUNTER5H 0xC85
#define MPHMCOUNTER5H 0xB85

#define MHPMEVENT6 0x326
#define HPMCOUNTER6 0xC06
#define MHMCOUNTER6 0xB06
#define HPMCOUNTER6H 0xC86
#define MPHMCOUNTER6H 0xB86

#define MHPMEVENT7 0x327
#define HPMCOUNTER7 0xC07
#define MHMCOUNTER7 0xB07
#define HPMCOUNTER7H 0xC87
#define MPHMCOUNTER7H 0xB87

#define MHPMEVENT8 0x328
#define HPMCOUNTER8 0xC08
#define MHMCOUNTER8 0xB08
#define HPMCOUNTER8H 0xC88
#define MPHMCOUNTER8H 0xB88

#define MHPMEVENT9 0x329
#define HPMCOUNTER9 0xC09
#define MHMCOUNTER9 0xB09
#define HPMCOUNTERR9H 0xC89
#define MPHMCOUNTER9H 0xB89

#define MHPMEVENT10 0x32A
#define HPMCOUNTER10 0xC0A
#define MHMCOUNTER10 0xB0A
#define HPMCOUNTER10H 0xC8A
#define MPHMCOUNTER10H 0xB8A

#define MHPMEVENT11 0x32B
#define HPMCOUNTER11 0xC0B
#define MHMCOUNTER11 0xB0B
#define HPMCOUNTER11H 0xC8B
#define MPHMCOUNTER11H 0xB8B

#define MHPMEVENT12 0x32C
#define HPMCOUNTER12 0xC0C
#define MHMCOUNTER12 0xB0C
#define HPMCOUNTER12H 0xC8C
#define MPHMCOUNTER12H 0xB8C

#define MHPMEVENT13 0x32D
#define HPMCOUNTER13 0xC0D
#define MHMCOUNTER13 0xB0D
#define HPMCOUNTER13H 0xC8D
#define MPHMCOUNTER13H 0xB8D

#define MHPMEVENT14 0x32E
#define HPMCOUNTER14 0xC0E
#define MHMCOUNTER14 0xB0E
#define HPMCOUNTER14H 0xC8E
#define MPHMCOUNTER14H 0xB8E

#define MHPMEVENT15 0x32F
#define HPMCOUNTER15 0xC0F
#define MHMCOUNTER15 0xB0F
#define HPMCOUNTER15H 0xC8F
#define MPHMCOUNTER15H 0xB8F

#define MHPMEVENT16 0x330
#define HPMCOUNTER16 0xC10
#define MHMCOUNTER16 0xB10
#define HPMCOUNTER16H 0xC90
#define MPHMCOUNTER16H 0xB90

#define MHPMEVENT17 0x331
#define HPMCOUNTER17 0xC11
#define MHMCOUNTER17 0xB11
#define HPMCOUNTER17H 0xC91
#define MPHMCOUNTER17H 0xB91

#define MHPMEVENT18 0x332
#define HPMCOUNTER18 0xC12
#define MHMCOUNTER18 0xB12
#define HPMCOUNTER18H 0xC92
#define MPHMCOUNTER18H 0xB92

#define MHPMEVENT19 0x333
#define HPMCOUNTER19 0xC13
#define MHMCOUNTER19 0xB13
#define HPMCOUNTER19H 0xC93
#define MPHMCOUNTER19H 0xB93

#define MHPMEVENT20 0x334
#define HPMCOUNTER20 0xC14
#define MHMCOUNTER20 0xB14
#define HPMCOUNTER20H 0xC94
#define MPHMCOUNTER20H 0xB94

#define MHPMEVENT21 0x335
#define HPMCOUNTER21 0xC15
#define MHMCOUNTER21 0xB15
#define HPMCOUNTER21H 0xC95
#define MPHMCOUNTER21H 0xB95

#define MHPMEVENT22 0x336
#define HPMCOUNTER22 0xC16
#define MHMCOUNTER22 0xB16
#define HPMCOUNTER22H 0xC96
#define MPHMCOUNTER22H 0xB96

#define MHPMEVENT23 0x337
#define HPMCOUNTER23 0xC17
#define MHMCOUNTER23 0xB17
#define HPMCOUNTER23H 0xC97
#define MPHMCOUNTER23H 0xB97

#define MHPMEVENT24 0x338
#define HPMCOUNTER24 0xC18
#define MHMCOUNTER24 0xB18
#define HPMCOUNTER24H 0xC98
#define MPHMCOUNTER24H 0xB98

#define MHPMEVENT25 0x339
#define HPMCOUNTER25 0xC19
#define MHMCOUNTER25 0xB19
#define HPMCOUNTER25H 0xC99
#define MPHMCOUNTER25H 0xB99

#define MHPMEVENT26 0x33A
#define HPMCOUNTER26 0xC1A
#define MHMCOUNTER26 0xB1A
#define HPMCOUNTER26H 0xC9A
#define MPHMCOUNTER26H 0xB9A

#define MHPMEVENT27 0x33B
#define HPMCOUNTER27 0xC1B
#define MHMCOUNTER27 0xB1B
#define HPMCOUNTER27H 0xC9B
#define MPHMCOUNTER27H 0xB9B

#define MHPMEVENT28 0x33C
#define HPMCOUNTER28 0xC1C
#define MHMCOUNTER28 0xB1C
#define HPMCOUNTER28H 0xC9C
#define MPHMCOUNTER28H 0xB9C

#define MHPMEVENT29 0x33D
#define HPMCOUNTER29 0xC1D
#define MHMCOUNTER29 0xB1D
#define HPMCOUNTER29H 0xC9D
#define MPHMCOUNTER29H 0xB9D

#define MHPMEVENT30 0x33E
#define HPMCOUNTER30 0xC1E
#define MHMCOUNTER30 0xB1E
#define HPMCOUNTER30H 0xC9E
#define MPHMCOUNTER30H 0xB9E

#define MHPMEVENT31 0x33F
#define HPMCOUNTER31 0xC1F
#define MHMCOUNTER31 0xB1F
#define HPMCOUNTER31H 0xC9F
#define MPHMCOUNTER31H 0xB9F
#pragma endregion

// Physical Memory Protection
#define PMPCFG0 0x3A0
#define PMPCFG1 0x3A1
#define PMPCFG2 0x3A2
#define PMPCFG3 0x3A3

#define PMPADDR0 0x3B0
#define PMPADDR1 0x3B1
#define PMPADDR2 0x3B2
#define PMPADDR3 0x3B3
#define PMPADDR4 0x3B4
#define PMPADDR5 0x3B5
#define PMPADDR6 0x3B6
#define PMPADDR7 0x3B7
#define PMPADDR8 0x3B8
#define PMPADDR9 0x3B9
#define PMPADDR10 0x3BA
#define PMPADDR11 0x3BB
#define PMPADDR12 0x3BC
#define PMPADDR13 0x3BD
#define PMPADDR14 0x3BE
#define PMPADDR15 0x3BF

// Debug/Trace
#define TSELECT 0x7A0
#define TDATA1 0x7A1
#define TDATA2 0x7A2
#define TDATA3 0x7A3

#define DCSR 0x7B0
#define DPC 0x7B1
#define SCRATCH 0x7B2

#endif