/*
 * Simulator of microcontrollers (inst_dd.cc)
 *  dd escaped multi-byte opcodes.
 *
 * some z80 code base from Karl Bongers karl@turbobit.com
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
cl_z80::inst_dd_ld(t_mem code)
{
  unsigned short tw;

  switch (code) {
    case 0x21: // LD IX,nnnn
      regs.IX = fetch2();
    return(resGO);
    case 0x22: // LD (nnnn),IX
      tw = fetch2();
      store2(tw, regs.IX);
    return(resGO);
    case 0x26: // LD HX,nn
      regs.ix.h = fetch1();
    return(resGO);
    case 0x2A: // LD IX,(nnnn)
      tw = fetch2();
      regs.IX = get2(tw);
    return(resGO);
    case 0x2E: // LD LX,nn
      regs.ix.l = fetch1();
    return(resGO);
    case 0x36: // LD (IX+dd),nn
      tw = add_u16_disp(regs.IX, fetch());
      store1(tw, fetch());
    return(resGO);
    case 0x44: // LD B,HX
      regs.ix.h = regs.hl.h;
    return(resGO);
    case 0x45: // LD B,LX
      regs.ix.l = regs.hl.l;
    return(resGO);
    case 0x46: // LD B,(IX+dd)
      regs.bc.h = get1(add_u16_disp(regs.IX,fetch()));
    return(resGO);
    case 0x4C: // LD C,HX
      regs.bc.l = regs.ix.h;
    return(resGO);
    case 0x4D: // LD C,LX
      regs.bc.l = regs.ix.l;
    return(resGO);
    case 0x4E: // LD C,(IX+dd)
      regs.bc.l = get1(add_u16_disp(regs.IX,fetch()));
    return(resGO);
    case 0x54: // LD D,HX
      regs.de.h = regs.ix.h;
    return(resGO);
    case 0x55: // LD D,LX
      regs.de.h = regs.ix.l;
    return(resGO);
    case 0x56: // LD D,(IX+dd)
      regs.de.h = get1(add_u16_disp(regs.IX,fetch()));
    return(resGO);
    case 0x5C: // LD E,H
      regs.de.l = regs.hl.h;
    return(resGO);
    case 0x5D: // LD E,L
      regs.de.l = regs.hl.l;
    return(resGO);
    case 0x5E: // LD E,(IX+dd)
      regs.de.l = get1(add_u16_disp(regs.IX,fetch()));
    return(resGO);
    case 0x60: // LD HX,B
      regs.ix.h = regs.bc.h;
    return(resGO);
    case 0x61: // LD HX,C
      regs.ix.h = regs.bc.l;
    return(resGO);
    case 0x62: // LD HX,D
      regs.ix.h = regs.de.h;
    return(resGO);
    case 0x63: // LD HX,E
      regs.ix.h = regs.de.l;
    return(resGO);
    case 0x64: // LD HX,HX
    return(resGO);
    case 0x65: // LD HX,LX
      regs.ix.h = regs.ix.l;
    return(resGO);
    case 0x66: // LD H,(IX+dd)
      regs.hl.h = get1(add_u16_disp(regs.IX,fetch()));
    return(resGO);
    case 0x67: // LD HX,A
      regs.ix.h = regs.A;
    return(resGO);
    case 0x68: // LD LX,B
      regs.ix.l = regs.bc.h;
    return(resGO);
    case 0x69: // LD LX,C
      regs.ix.l = regs.bc.l;
    return(resGO);
    case 0x6A: // LD LX,D
      regs.ix.l = regs.de.h;
    return(resGO);
    case 0x6B: // LD LX,E
      regs.ix.l = regs.de.l;
    return(resGO);
    case 0x6C: // LD LX,HX
      regs.ix.l = regs.hl.h;
    return(resGO);
    case 0x6D: // LD LX,LX
    return(resGO);
    case 0x6E: // LD L,(IX+dd)
      regs.hl.l = get1(add_u16_disp(regs.IX,fetch()));
    return(resGO);
    case 0x6F: // LD LX,A
      regs.ix.l = regs.A;
    return(resGO);
    case 0x70: // LD (IX+dd),B
      store1(add_u16_disp(regs.IX,fetch()), regs.bc.h);
    return(resGO);
    case 0x71: // LD (IX+dd),C
      store1(add_u16_disp(regs.IX,fetch()), regs.bc.l);
    return(resGO);
    case 0x72: // LD (IX+dd),D
      store1(add_u16_disp(regs.IX,fetch()), regs.de.h);
    return(resGO);
    case 0x73: // LD (IX+dd),E
      store1(add_u16_disp(regs.IX,fetch()), regs.de.l);
    return(resGO);
    case 0x74: // LD (IX+dd),H
      store1(add_u16_disp(regs.IX,fetch()), regs.hl.h);
    return(resGO);
    case 0x75: // LD (IX+dd),L
      store1(add_u16_disp(regs.IX,fetch()), regs.hl.l);
    return(resGO);
    case 0x77: // LD (IX+dd),A
      store1(add_u16_disp(regs.IX,fetch()), regs.A);
    return(resGO);
    case 0x7C: // LD A,HX
      regs.A = regs.hl.h;
    return(resGO);
    case 0x7D: // LD A,LX
      regs.A = regs.hl.l;
    return(resGO);
    case 0x7E: // LD A,(IX+dd)
      regs.A = get1(add_u16_disp(regs.IX,fetch()));
    return(resGO);
    case 0xF9: // LD SP,IX
      regs.SP = regs.IX;
    return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_dd_add(t_mem code)
{
#define add_IX_Word(wr) { \
      unsigned int tmp; \
      regs.F &= ~(BIT_A | BIT_N | BIT_C);  /* clear these */ \
      tmp = (unsigned int)regs.IX + (unsigned int)(wr); \
      if (tmp > 0xffff) regs.F |= BIT_C; \
      regs.IX = (unsigned short) tmp; }

  switch (code) {
    case 0x09: // ADD IX,BC
      add_IX_Word(regs.BC);
      return(resGO);
    case 0x19: // ADD IX,DE
      add_IX_Word(regs.DE);
      return(resGO);
    case 0x29: // ADD IX,IX
      add_IX_Word(regs.IX);
      return(resGO);
    case 0x39: // ADD IX,SP
      add_IX_Word(regs.SP);
    return(resGO);
    case 0x84: // ADD A,HX
      add_A_bytereg(regs.ix.h);
      return(resGO);
    case 0x85: // ADD A,LX
      add_A_bytereg(regs.ix.l);
      return(resGO);
    case 0x86: // ADD A,(IX)
      { unsigned char ourtmp;
        t_addr addr;
        addr = add_u16_disp(regs.IX, fetch());
        ourtmp = get1(addr);
        add_A_bytereg(ourtmp);
      }
      return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_dd_push(t_mem code)
{
  switch (code) {
    case 0xe5: // PUSH IX
      push2(regs.IX);
    return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_dd_inc(t_mem code)
{
  switch(code) {
    case 0x23: // INC IX
      ++regs.IX;
    break;
    case 0x24: // INC HX
      inc(regs.ix.h);
    break;
    case 0x2C: // INC LX
      inc(regs.ix.l);
    break;
    case 0x34: // INC (IX+dd)
      {
        t_addr addr;
        addr = add_u16_disp(regs.IX,fetch());
        store1(addr, get1(addr)+1);
      }
    break;
  }
  return(resGO);
}

int
cl_z80::inst_dd_dec(t_mem code)
{
  switch(code) {
    case 0x25: // DEC HX
      dec(regs.ix.h);
    break;
    case 0x2B: // DEC IX
      --regs.IX;
    break;
    case 0x2D: // DEC LX
      dec(regs.ix.l);
    break;
    case 0x35: // DEC (IX+dd)
      {
        t_addr addr;
        addr = add_u16_disp(regs.IX,fetch());
        store1(addr, get1(addr)-1);
      }
    break;
  }
  return(resGO);
}


/* need ADC, SUB, SBC, AND, XOR, OR, CP */
int
cl_z80::inst_dd_misc(t_mem code)
{
  switch(code) {
    case 0x8C: // ADC A,HX
      adc_A_bytereg(regs.ix.h);
    return(resGO);
    case 0x8D: // ADC A,LX
      adc_A_bytereg(regs.ix.l);
    return(resGO);
    case 0x8E: // ADC A,(IX)
      { unsigned char utmp;
        t_addr addr;
        addr = add_u16_disp(regs.IX, fetch());
        utmp = get1(addr);
        adc_A_bytereg(utmp);
      }
    return(resGO);

    case 0x94: // SUB HX
      sub_A_bytereg(regs.ix.h);
    return(resGO);
    case 0x95: // SUB LX
      sub_A_bytereg(regs.ix.l);
    return(resGO);
    case 0x96: // SUB (IX+dd)
      { unsigned char tmp1;
        tmp1 = get1(add_u16_disp(regs.IX, fetch()));
        sub_A_bytereg(tmp1);
      }
    return(resGO);

    case 0x9C: // SBC A,HX
      sbc_A_bytereg(regs.ix.h);
    return(resGO);
    case 0x9D: // SBC A,LX
      sbc_A_bytereg(regs.ix.l);
    return(resGO);
    case 0x9E: // SBC A,(IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs.IX, fetch()));
        sbc_A_bytereg(utmp);
      }
    return(resGO);

    case 0xA4: // AND HX
      and_A_bytereg(regs.ix.h);
    return(resGO);
    case 0xA5: // AND LX
      and_A_bytereg(regs.ix.l);
    return(resGO);
    case 0xA6: // AND (IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs.IX, fetch()));
        and_A_bytereg(utmp);
      }
    return(resGO);

    case 0xAC: // XOR HX
      xor_A_bytereg(regs.ix.h);
    return(resGO);
    case 0xAD: // XOR LX
      xor_A_bytereg(regs.ix.l);
    return(resGO);
    case 0xAE: // XOR (IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs.IX, fetch()));
        xor_A_bytereg(utmp);
      }
    return(resGO);

    case 0xB4: // OR HX
      or_A_bytereg(regs.ix.h);
    return(resGO);
    case 0xB5: // OR LX
      or_A_bytereg(regs.ix.l);
    return(resGO);
    case 0xB6: // OR (IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs.IX, fetch()));
        or_A_bytereg(utmp);
      }
    return(resGO);

    case 0xBC: // CP HX
      cp_bytereg(regs.ix.h);
    return(resGO);
    case 0xBD: // CP LX
      cp_bytereg(regs.ix.l);
    return(resGO);
    case 0xBE: // CP (IX+dd)
      { unsigned char utmp;
        utmp = get1(add_u16_disp(regs.IX, fetch()));
        cp_bytereg(utmp);
      }
    return(resGO);
  }
  return(resGO);
}

int
cl_z80::inst_dd(void)
{
  t_mem code;

  if (fetch(&code))
    return(resBREAKPOINT);

  switch (code)
    {
      case 0x21: // LD IX,nnnn
      case 0x22: // LD (nnnn),IX
      case 0x26: // LD HX,nn
      case 0x2A: // LD IX,(nnnn)
      case 0x2E: // LD LX,nn
      case 0x36: // LD (IX+dd),nn
      case 0x44: // LD B,HX
      case 0x45: // LD B,LX
      case 0x46: // LD B,(IX+dd)
      case 0x4C: // LD C,HX
      case 0x4D: // LD C,LX
      case 0x4E: // LD C,(IX+dd)
      case 0x54: // LD D,HX
      case 0x55: // LD D,LX
      case 0x56: // LD D,(IX+dd)
      case 0x5C: // LD E,H
      case 0x5D: // LD E,L
      case 0x5E: // LD E,(IX+dd)
      case 0x60: // LD HX,B
      case 0x61: // LD HX,C
      case 0x62: // LD HX,D
      case 0x63: // LD HX,E
      case 0x64: // LD HX,HX
      case 0x66: // LD H,(IX+dd)
      case 0x67: // LD HX,A
      case 0x68: // LD LX,B
      case 0x69: // LD LX,C
      case 0x6A: // LD LX,D
      case 0x6B: // LD LX,E
      case 0x6C: // LD LX,HX
      case 0x6D: // LD LX,LX
      case 0x6E: // LD L,(IX+dd)
      case 0x6F: // LD LX,A
      case 0x70: // LD (IX+dd),B
      case 0x71: // LD (IX+dd),C
      case 0x72: // LD (IX+dd),D
      case 0x73: // LD (IX+dd),E
      case 0x74: // LD (IX+dd),H
      case 0x75: // LD (IX+dd),L
      case 0x77: // LD (IX+dd),A
      case 0x7C: // LD A,HX
      case 0x7D: // LD A,LX
      case 0x7E: // LD A,(IX+dd)
      case 0xF9: // LD SP,IX
        return(inst_dd_ld(code));

      case 0x23: // INC IX
      case 0x24: // INC HX
      case 0x2C: // INC LX
      case 0x34: // INC (IX+dd)
        return(inst_dd_inc(code));
      {
        t_addr addr;
        addr = add_u16_disp(regs.IX,fetch());
        store1(addr, get1(addr)+1);
      }

      case 0x09: // ADD IX,BC
      case 0x19: // ADD IX,DE
      case 0x29: // ADD IX,IX
      case 0x39: // ADD IX,SP
      case 0x84: // ADD A,HX
      case 0x85: // ADD A,LX
      case 0x86: // ADD A,(IX)
        return(inst_dd_add(code));

      case 0x25: // DEC HX
      case 0x2B: // DEC IX
      case 0x2D: // DEC LX
      case 0x35: // DEC (IX+dd)
      return(inst_dd_dec(code));

      case 0x8C: // ADC A,HX
      case 0x8D: // ADC A,LX
      case 0x8E: // ADC A,(IX)
      case 0x94: // SUB HX
      case 0x95: // SUB LX
      case 0x96: // SUB (IX+dd)
      case 0x9C: // SBC A,HX
      case 0x9D: // SBC A,LX
      case 0x9E: // SBC A,(IX+dd)
      case 0xA4: // AND HX
      case 0xA5: // AND LX
      case 0xA6: // AND (IX+dd)
      case 0xAC: // XOR HX
      case 0xAD: // XOR LX
      case 0xAE: // XOR (IX+dd)
      case 0xB4: // OR HX
      case 0xB5: // OR LX
      case 0xB6: // OR (IX+dd)
      case 0xBC: // CP HX
      case 0xBD: // CP LX
      case 0xBE: // CP (IX+dd)
        return(inst_dd_misc(code));
      break;

      case 0xCB: // escape, IX prefix to CB commands
        return(inst_ddcb()); /* see inst_ddcb.cc */
      break;

      case 0xE1: // POP IX
        regs.IX = get2(regs.SP);
        regs.SP+=2;
      return(resGO);

      case 0xE3: // EX (SP),IX
        {
          TYPE_UWORD tempw;

          tempw = regs.IX;
          regs.IX = get1(regs.SP);
          store1(regs.SP, tempw);
        }
      return(resGO);

      case 0xE5: // PUSH IX
        push2(regs.IX);
      return(resGO);

      case 0xE9: // JP (IX)
        PC = get2(regs.IX);
      return(resGO);

      default:
      return(resINV_INST);
    }
  return(resINV_INST);
}

/* End of z80.src/inst_dd.cc */
