/*
 * Simulator of microcontrollers (inst_ed.cc)
 *   ED escaped multi-byte opcodes for Z80.
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include "ddconfig.h"

// local
#include "z80cl.h"
#include "regsz80.h"
#include "z80mac.h"


int
cl_z80::inst_ed_(t_mem code)
{
  switch(code) {
  }
  return(resGO);
}

/******** start CB codes *****************/
int
cl_z80::inst_ed(void)
{
  t_mem code;
  unsigned short tw;

  if (fetch(&code))
    return(resBREAKPOINT);

  switch (code)
  {
    case 0x00:
    return(resGO);
    case 0x40: // IN B,(C)
    return(resGO);
    case 0x41: // OUT (C),B
    return(resGO);
    case 0x42: // SBC HL,BC
      sbc_HL_wordreg(regs.BC);
    return(resGO);
    case 0x43: // LD (nnnn),BC
      tw = fetch2();
      store2(tw, regs.BC);
    return(resGO);
    case 0x44: // NEG
      regs.F &= ~(BIT_ALL);  /* clear these */
      regs.A -= regs.A;
      regs.F |= BIT_N; /* not addition */
      if (regs.A == 0) regs.F |= BIT_Z;
      if (regs.A & 0x80) regs.F |= BIT_S;
      /* Skip BIT_A for now */
    return(resGO);
    case 0x45: // RETN
      pop2(PC);
    return(resGO);
    case 0x46: // IM 0
      /* interrupt device puts opcode on data bus */
    return(resGO);
    case 0x47: // LD IV,A
      regs.iv = regs.A;
    return(resGO);
    case 0x48: // IN C,(C)
    return(resGO);
    case 0x49: // OUT (C),C
    return(resGO);
    case 0x4A: // ADC HL,BC
      adc_HL_wordreg(regs.BC);
    return(resGO);
    case 0x4B: // LD BC,(nnnn)
      tw = fetch2();
      regs.BC = get2(tw);
    return(resGO);
    case 0x4D: // RETI
      pop2(PC);
    return(resGO);
    case 0x4F: // LD R,A
      /* Load "refresh" register(whats that?) */
    return(resGO);
    case 0x50: // IN D,(C)
    return(resGO);
    case 0x51: // OUT (C),D
    return(resGO);
    case 0x52: // SBC HL,DE
      sbc_HL_wordreg(regs.DE);
    return(resGO);
    case 0x53: // LD (nnnn),DE
      tw = fetch2();
      store2(tw, regs.DE);
    return(resGO);
    case 0x56: // IM 1
    return(resGO);
    case 0x57: // LD A,IV
      regs.A = regs.iv;
    return(resGO);
    case 0x58: // IN E,(C)
    return(resGO);
    case 0x59: // OUT (C),E
    return(resGO);
    case 0x5A: // ADC HL,DE
      adc_HL_wordreg(regs.DE);
    return(resGO);
    case 0x5B: // LD DE,(nnnn)
      tw = fetch2();
      regs.DE = get2(tw);
    return(resGO);

    case 0x5E: // IM 2
    return(resGO);
    case 0x5F: // LD A,R
    return(resGO);
    case 0x60: // IN H,(C)
    return(resGO);
    case 0x61: // OUT (C),H
    return(resGO);
    case 0x62: // SBC HL,HL
      sbc_HL_wordreg(regs.HL);
    return(resGO);
    case 0x63: // LD (nnnn),HL opcode 22 does the same faster
      tw = fetch2();
      store2(tw, regs.HL);
    return(resGO);

    case 0x67: // RRD
    return(resGO);
    case 0x68: // IN L,(C)
    return(resGO);
    case 0x69: // OUT (C),L
    return(resGO);
    case 0x6A: // ADC HL,HL
      adc_HL_wordreg(regs.HL);
    return(resGO);
    case 0x6B: // LD HL,(nnnn) opcode 2A does the same faster
      tw = fetch2();
      regs.HL = get2(tw);
    return(resGO);

    case 0x6F: // RLD
      /* rotate 1 bcd digit left between ACC and memory location */
    return(resGO);
    case 0x70: // IN (C)  set flags only (TSTI)
    return(resGO);
    case 0x71: //  OUT (C),0
    return(resGO);
    case 0x72: // SBC HL,SP
      sbc_HL_wordreg(regs.SP);
    return(resGO);
    case 0x73: // LD (nnnn),SP
      tw = fetch2();
      store2(tw, regs.SP);
    return(resGO);


    case 0x78: // IN A,(C)
    return(resGO);
    case 0x79: // OUT (C),A
    return(resGO);
    case 0x7A: // ADC HL,SP
      adc_HL_wordreg(regs.SP);
    return(resGO);
    case 0x7B: // LD SP,(nnnn)
      tw = fetch2();
      regs.SP = get2(tw);
    return(resGO);

    case 0xA0: // LDI
    return(resGO);
    case 0xA1: // CPI
    return(resGO);
    case 0xA2: // INI
    return(resGO);
    case 0xA3: // OUTI
    return(resGO);
    case 0xA8: // LDD
    return(resGO);
    case 0xA9: // CPD
    return(resGO);
    case 0xAA: // IND
    return(resGO);
    case 0xAB: // OUTD
    return(resGO);
    case 0xB0: // LDIR
    return(resGO);
    case 0xB1: // CPIR
    return(resGO);
    case 0xB2: // INIR
    return(resGO);
    case 0xB3: // OTIR
    return(resGO);
    case 0xB8: // LDDR
    return(resGO);
    case 0xB9: // CPDR
    return(resGO);
    case 0xBA: // INDR
    return(resGO);
    case 0xBB: // OTDR
    return(resGO);
    default:
    return(resINV_INST);
  }

  return(resINV_INST);
}

/* End of z80.src/inst_ed.cc */
