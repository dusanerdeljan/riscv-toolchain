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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/csr.h"

char* decodeCSR(int csrCode)
{
    char* csr = (char*)calloc(18, sizeof(char));
    if(csrCode >= PMPADDR0 && csrCode <= PMPADDR15)
    {
        int num = csrCode - PMPADDR0;
        snprintf(csr, 17, "pmpaddr%d", num);
        return csr;
    }
    else if(csrCode >= MHPMEVENT3 && csrCode <= MHPMEVENT31)
    {
        int num = csrCode - MHPMEVENT3 + 3;
        snprintf(csr, 17, "mhpmevent%d", num);
        return csr;
    }
    else if(csrCode >= HPMCOUNTER3 && csrCode <= HPMCOUNTER31)
    {
        int num = csrCode - HPMCOUNTER3 + 3;
        snprintf(csr, 17, "hpmcounter%d", num);
        return csr;
    }
    else if(csrCode >= MHMCOUNTER3 && csrCode <= MHMCOUNTER31)
    {
        int num = csrCode - MHMCOUNTER3 + 3;
        snprintf(csr, 17, "mhpmcounter%d", num);
        return csr;
    }
    else if(csrCode >= HPMCOUNTER3H && csrCode <= HPMCOUNTER31H)
    {
        int num = csrCode - HPMCOUNTER3H + 3;
        snprintf(csr, 17, "hpmcounter%dh", num);
        return csr;
    }
    else if(csrCode >= MPHMCOUNTER3H && csrCode <= MPHMCOUNTER31H)
    {
        int num = csrCode - MPHMCOUNTER3H + 3;
        snprintf(csr, 17, "mhpmcounter%dh", num);
        return csr;
    }
    switch(csrCode)
    {
        case CYCLE:
            strncpy(csr, "cycle", 17);
            break;
        case MCYCLE:
            strncpy(csr, "mcycle", 17);
            break;
        case CYCLEH:
            strncpy(csr, "cycleh", 17);
            break;
        case MCYCLEH:
            strncpy(csr, "mcycleh", 17);
            break;
        case TIME:
            strncpy(csr, "time", 17);
            break;
        case TIMEH:
            strncpy(csr, "timeh", 17);
            break;
        case INSTRET:
            strncpy(csr, "instret", 17);
            break;
        case MINSTRET:
            strncpy(csr, "minstret", 17);
            break;
        case INSTRETH:
            strncpy(csr, "instreth", 17);
            break;
        case MINSTRETH:
            strncpy(csr, "minstreth", 17);
            break;
        case USTATUS:
            strncpy(csr, "ustatus", 17);
            break;
        case SSTATUS:
            strncpy(csr, "sstatus", 17);
            break;
        case MSTATUS:
            strncpy(csr, "mstatus", 17);
            break;
        case UIE:
            strncpy(csr, "uie", 17);
            break;
        case SIE:
            strncpy(csr, "sie", 17);
            break;
        case MIE:
            strncpy(csr, "mie", 17);
            break;
        case UTVEC:
            strncpy(csr, "utvec", 17);
            break;
        case STVEC:
            strncpy(csr, "stvec", 17);
            break;
        case MTVEC:
            strncpy(csr, "mtvec", 17);
            break;
        case SEDELEG:
            strncpy(csr, "sedeleg", 17);
            break;
        case MEDELEG:
            strncpy(csr, "medelg", 17);
            break;
        case SIDELEG:
            strncpy(csr, "sideleg", 17);
            break;
        case MIDELEG:
            strncpy(csr, "mideleg", 17);
            break;
        case SCOUNTEREN:
            strncpy(csr, "scounteren", 17);
            break;
        case MCOUNTEREN:
            strncpy(csr, "mcounteren", 17);
            break;
        case USCRATCH:
            strncpy(csr, "scratch", 17);
            break;
        case SSCRATCH:
            strncpy(csr, "sscratch", 17);
            break;
        case MSCRATCH:
            strncpy(csr, "mscratch", 17);
            break;
        case UEPC:
            strncpy(csr, "uepc", 17);
            break;
        case SEPC:
            strncpy(csr, "sepc", 17);
            break;
        case MEPC:
            strncpy(csr, "mepc", 17);
            break;
        case UIP:
            strncpy(csr, "uip", 17);
            break;
        case SIP:
            strncpy(csr, "sip", 17);
            break;
        case MIP:
            strncpy(csr, "mip", 17);
            break;
        case SATP:
            strncpy(csr, "satp", 17);
            break;
        case MISA:
            strncpy(csr, "misa", 17);
            break;
        case MVENDROID:
            strncpy(csr, "mvendroid", 17);
            break;
        case MARCHID:
            strncpy(csr, "marchid", 17);
            break;
        case MIMPID:
            strncpy(csr, "mimpid", 17);
            break;
        case MHARDID:
            strncpy(csr, "mhardid", 17);
            break;
        case FFLAGS:
            strncpy(csr, "fflags", 17);
            break;
        case FRM:
            strncpy(csr, "frm", 17);
            break;
        case FCSR:
            strncpy(csr, "fcsr", 17);
            break;
        case PMPCFG0:
            strncpy(csr, "pmpcfg0", 17);
            break;
        case PMPCFG1:
            strncpy(csr, "pmpcfg1", 17);
            break;
        case PMPCFG2:
            strncpy(csr, "pmpcfg2", 17);
            break;
        case PMPCFG3:
            strncpy(csr, "pmpcfg3", 17);
            break;
        case TSELECT:
            strncpy(csr, "tselect", 17);
            break;
        case TDATA1:
            strncpy(csr, "tdata1", 17);
            break;
        case TDATA2:
            strncpy(csr, "tdata2", 17);
            break;
        case TDATA3:
            strncpy(csr, "tdata3", 17);
            break;
        case DCSR:
            strncpy(csr, "dcsr", 17);
            break;
        case DPC:
            strncpy(csr, "dpc", 17);
            break;
        case SCRATCH:
            strncpy(csr, "scratch", 17);
            break; 
    }
    return csr;
}