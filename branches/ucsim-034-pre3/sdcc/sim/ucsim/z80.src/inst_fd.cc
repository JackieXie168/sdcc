/*
 * Simulator of microcontrollers (inst_fd.cc)
 *  FD escaped multi-byte opcodes.
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
cl_z80::inst_fd_ld(t_mem code)
{
  unsigned short tw;

  switch (code) {
    case 0x21: // LD IY,nnnn
      regs.IY = fetch2();
    return(resGO);
    case 0x22: // LD (nnnn),IY
      tw = fetch2();
      store2(tw, regs.IY);
    return(resGO);
    case 0x26: // LD HY,nn
      regs.iy.h = fetch1();
    return(resGO);
    case 0x2A: // LD IY,(nnnn)
      tw = fetch2();
      regs.IY = get2(tw);
    return(resGO);
    case 0x2E: // LD LY,nn
      regs.iy.l = fetch1();
    return(resGO);
    case 0x36: // LD (IY+dd),nn
      tw = fetch();
      store1(regs.IY+tw, fetch());
    return(resGO);
    case 0x44: // LD B,HY
      regs.iy.h = regs.hl.h;
    return(resGO);
    case 0x45: // LD B,LY
      regs.iy.l = regs.hl.l;
    return(resGO);
    case 0x46: // LD B,(IY+dd)
      regs.bc.h = get1(add_u16_disp(regs.IY,fetch()));
    return(resGO);
    case 0x4C: // LD C,HY
      regs.bc.l = regs.iy.h;
    return(resGO);
    case 0x4D: // LD C,LY
      regs.bc.l = regs.iy.l;
    return(resGO);
    case 0x4E: // LD C,(IY+dd)
      regs.bc.l = get1(add_u16_disp(regs.IY,fetch()));
    return(resGO);
    case 0x54: // LD D,HY
      regs.de.h = regs.iy.h;
    return(resGO);
    case 0x55: // LD D,LY
      regs.de.h = regs.iy.l;
    return(resGO);
    case 0x56: // LD D,(IY+dd)
      regs.de.h = get1(add_u16_disp(regs.IY,fetch()));
    return(resGO);
    case 0x5C: // LD E,H
      regs.de.l = regs.hl.h;
    return(resGO);
    case 0x5D: // LD E,L
      regs.de.l = regs.hl.l;
    return(resGO);
    case 0x5E: // LD E,(IY+dd)
      regs.de.l = get1(add_u16_disp(regs.IY,fetch()));
    return(resGO);
    case 0x60: // LD HY,B
      regs.iy.h = regs.bc.h;
    return(resGO);
    case 0x61: // LD HY,C
      regs.iy.h = regs.bc.l;
    return(resGO);
    case 0x62: // LD HY,D
      regs.iy.h = regs.de.h;
    return(resGO);
    case 0x63: // LD HY,E
      regs.iy.h = regs.de.l;
    return(resGO);
    case 0x64: // LD HY,HY
    return(resGO);
    case 0x65: // LD HY,LY
      regs.iy.h = regs.iy.l;
    return(resGO);
    case 0x66: // LD H,(IY+dd)
      regs.hl.h = get1(add_u16_disp(regs.IY,fetch()));
    return(resGO);
    case 0x67: // LD HY,A
      regs.iy.h = regs.A;
    return(resGO);
    case 0x68: // LD LY,B
      regs.iy.l = regs.bc.h;
    return(resGO);
    case 0x69: // LD LY,C
      regs.iy.l = regs.bc.l;
    return(resGO);
    case 0x6A: // LD LY,D
      regs.iy.l = regs.de.h;
    return(resGO);
    case 0x6B: // LD LY,E
      regs.iy.l = regs.de.l;
    return(resGO);
    case 0x6C: // LD LY,HY
      regs.iy.l = regs.hl.h;
    return(resGO);
    case 0x6D: // LD LY,LY
    return(resGO);
    case 0x6E: // LD L,(IY+dd)
      regs.hl.l = get1(add_u16_disp(regs.IY,fetch()));
    return(resGO);
    case 0x6F: // LD LY,A
      regs.iy.l = regs.A;
    return(resGO);
    case 0x70: // LD (IY+dd),B
      store1(add_u16_disp(regs.IY,fetch()), regs.bc.h);
    return(resGO);
    case 0x71: // LD (IY+dd),C
      store1(add_u16_disp(regs.IY,fetch()), regs.bc.l);
    return(resGO);
    case 0x72: // LD (IY+dd),D
      store1(add_u16_disp(regs.IY,fetch()), regs.de.h);
    return(resGO);
    case 0x73: // LD (IY+dd),E
      store1(add_u16_disp(regs.IY,fetch()), regs.de.l);
    return(resGO);
    case 0x74: // LD (IY+dd),H
      store1(add_u16_disp(regs.IY,fetch()), regs.hl.h);
    return(resGO);
    case 0x75: // LD (IY+dd),L
      store1(add_u16_disp(regs.IY,fetch()), regs.hl.l);
    return(resGO);
    case 0x77: // LD (IY+dd),A
      store1(add_u16_disp(regs.IY,fetch()), regs.A);
    return(resGO);
    case 0x7C: // LD A,HY
      regs.A = regs.hl.h;
    return(resGO);
    case 0x7D: // LD A,LY
      regs.A = regs.hl.l;
    return(resGO);
    case 0x7E: // LD A,(IY+dd)
      regs.A = get1(add_u16_disp(regs.IY,fetch()));
    return(resGO);
    case 0xF9: // LD SP,IY
      regs.SP = regs.IY;
    return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_fd_add(t_mem code)
{
#define add_IY_Word(wr) { \
      unsigned int tmp; \
      regs.F &= ~(BIT_A | BIT_N | BIT_C);  /* clear these */ \
      tmp = (unsigned int)regs.IY + (unsigned int)(wr); \
      if (tmp > 0xffff) regs.F |= BIT_C; \
      regs.IY = (unsigned short) tmp; }

  switch (code) {
    case 0x09: // ADD IY,BC
      add_IY_Word(regs.BC);
      return(resGO);
    case 0x19: // ADD IY,DE
      add_IY_Word(regs.DE);
      return(resGO);
    case 0x29: // ADD IY,IY
      add_IY_Word(regs.IY);
      return(resGO);
    case 0x39: // ADD IY,SP
      add_IY_Word(regs.SP);
    return(resGO);
    case 0x84: // ADD A,HY
      add_A_bytereg(regs.iy.h);
      return(resGO);
    case 0x85: // ADD A,LY
      add_A_bytereg(regs.iy.l);
      return(resGO);
    case 0x86: // ADD A,(IY)
      { unsigned char ourtmp;
        ourtmp = get1(regs.IY);
        add_A_bytereg(ourtmp);
      }
      return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_fd_push(t_mem code)
{
  switch (code) {
    case 0xe5: // PUSH IY
      push2(regs.IY);
    return(resGO);
  }
  return(resINV_INST);
}

int
cl_z80::inst_fd_inc(t_mem code)
{
  switch(code) {
    case 0x23: // INC IY
      ++regs.IY;
    break;
    case 0x24: // INC HY
      inc(regs.iy.h);
    break;
    case 0x2C: // INC LY
      inc(regs.iy.l);
    break;
    case 0x34: // INC (IY+dd)
      {
        t_addr addr;
        addr = add_u16_disp(regs.IY,fetch());
        store1(addr, get1(addr)+1);
      }
    break;
  }
  return(resGO);
}

int
cl_z80::inst_fd_dec(t_mem code)
{
  switch(code) {
    case 0x25: // DEC HY
      dec(regs.iy.h);
    break;
    case 0x2B: // DEC IY
      --regs.IY;
    break;
    case 0x2D: // DEC LY
      dec(regs.iy.l);
    break;
    case 0x35: // DEC (IY+dd)
      {
        t_addr addr;
        addr = add_u16_disp(regs.IY,fetch());
        store1(addr, get1(addr)-1);
      }
    break;
  }
  return(resGO);
}


/* need ADC, SUB, SBC, AND, XOR, OR, CP */
int
cl_z80::inst_fd_misc(t_mem code)
{
  switch(code) {
    case 0x8C: // ADC A,HY
      adc_A_bytereg(regs.iy.h);
    return(resGO);
    case 0x8D: // ADC A,LY
      adc_A_bytereg(regs.iy.l);
    return(resGO);
    case 0x8E: // ADC A,(IY)
      { unsigned char utmp;
        utmp = get1(regs.IY);
        adc_A_bytereg(utmp);
      }
    return(resGO);

    case 0x94: // SUB HY
      sub_A_bytereg(regs.iy.h);
    return(resGO);
    case 0x95: // SUB LY
      sub_A_bytereg(regs.iy.l);
    return(resGO);
    case 0x96: // SUB (IY+dd)
      { unsigned char tmp1;
        tmp1 = get1(regs.IY + fetch());
        sub_A_bytereg(tmp1);
      }
    return(resGO);

    case 0x9C: // SBC A,HY
      sbc_A_bytereg(regs.iy.h);
    return(resGO);
    case 0x9D: // SBC A,LY
      sbc_A_bytereg(regs.iy.l);
    return(resGO);
    case 0x9E: // SBC A,(IY+dd)
      { unsigned char utmp;
        utmp = get1(regs.IY + fetch());
        sbc_A_bytereg(utmp);
      }
    return(resGO);

    case 0xA4: // AND HY
      and_A_bytereg(regs.iy.h);
    return(resGO);
    case 0xA5: // AND LY
      and_A_bytereg(regs.iy.l);
    return(resGO);
    case 0xA6: // AND (IY+dd)
      { unsigned char utmp;
        utmp = get1(regs.IY + fetch());
        and_A_bytereg(utmp);
      }
    return(resGO);

    case 0xAC: // XOR HY
      xor_A_bytereg(regs.iy.h);
    return(resGO);
    case 0xAD: // XOR LY
      xor_A_bytereg(regs.iy.l);
    return(resGO);
    case 0xAE: // XOR (IY+dd)
      { unsigned char utmp;
        utmp = get1(regs.IY + fetch());
        xor_A_bytereg(utmp);
      }
    return(resGO);

    case 0xB4: // OR HY
      or_A_bytereg(regs.iy.h);
    return(resGO);
    case 0xB5: // OR LY
      or_A_bytereg(regs.iy.l);
    return(resGO);
    case 0xB6: // OR (IY+dd)
      { unsigned char utmp;
        utmp = get1(regs.IY + fetch());
        or_A_bytereg(utmp);
      }
    return(resGO);

    case 0xBC: // CP HY
      cp_bytereg(regs.iy.h);
    return(resGO);
    case 0xBD: // CP LY
      cp_bytereg(regs.iy.l);
    return(resGO);
    case 0xBE: // CP (IY+dd)
      { unsigned char utmp;
        utmp = get1(regs.IY + fetch());
        cp_bytereg(utmp);
      }
    return(resGO);
  }
  return(resGO);
}

int
cl_z80::inst_fd(void)
{
  t_mem code;

  if (fetch(&code))
    return(resBREAKPOINT);

  switch (code)
    {
      case 0x21: // LD IY,nnnn
      case 0x22: // LD (nnnn),IY
      case 0x26: // LD HY,nn
      case 0x2A: // LD IY,(nnnn)
      case 0x2E: // LD LY,nn
      case 0x36: // LD (IY+dd),nn
      case 0x44: // LD B,HY
      case 0x45: // LD B,LY
      case 0x46: // LD B,(IY+dd)
      case 0x4C: // LD C,HY
      case 0x4D: // LD C,LY
      case 0x4E: // LD C,(IY+dd)
      case 0x54: // LD D,HY
      case 0x55: // LD D,LY
      case 0x56: // LD D,(IY+dd)
      case 0x5C: // LD E,H
      case 0x5D: // LD E,L
      case 0x5E: // LD E,(IY+dd)
      case 0x60: // LD HY,B
      case 0x61: // LD HY,C
      case 0x62: // LD HY,D
      case 0x63: // LD HY,E
      case 0x64: // LD HY,HY
      case 0x66: // LD H,(IY+dd)
      case 0x67: // LD HY,A
      case 0x68: // LD LY,B
      case 0x69: // LD LY,C
      case 0x6A: // LD LY,D
      case 0x6B: // LD LY,E
      case 0x6C: // LD LY,HY
      case 0x6D: // LD LY,LY
      case 0x6E: // LD L,(IY+dd)
      case 0x6F: // LD LY,A
      case 0x70: // LD (IY+dd),B
      case 0x71: // LD (IY+dd),C
      case 0x72: // LD (IY+dd),D
      case 0x73: // LD (IY+dd),E
      case 0x74: // LD (IY+dd),H
      case 0x75: // LD (IY+dd),L
      case 0x77: // LD (IY+dd),A
      case 0x7C: // LD A,HY
      case 0x7D: // LD A,LY
      case 0x7E: // LD A,(IY+dd)
      case 0xF9: // LD SP,IY
        return(inst_fd_ld(code));

      case 0x23: // INC IY
      case 0x24: // INC HY
      case 0x2C: // INC LY
      case 0x34: // INC (IY+dd)
        return(inst_fd_inc(code));
      {
        t_addr addr;
        addr = add_u16_disp(regs.IY,fetch());
        store1(addr, get1(addr)+1);
      }

      case 0x09: // ADD IY,BC
      case 0x19: // ADD IY,DE
      case 0x29: // ADD IY,IY
      case 0x39: // ADD IY,SP
      case 0x84: // ADD A,HY
      case 0x85: // ADD A,LY
      case 0x86: // ADD A,(IY)
        return(inst_fd_add(code));

      case 0x25: // DEC HY
      case 0x2B: // DEC IY
      case 0x2D: // DEC LY
      case 0x35: // DEC (IY+dd)
      return(inst_fd_dec(code));

      case 0x8C: // ADC A,HY
      case 0x8D: // ADC A,LY
      case 0x8E: // ADC A,(IY)
      case 0x94: // SUB HY
      case 0x95: // SUB LY
      case 0x96: // SUB (IY+dd)
      case 0x9C: // SBC A,HY
      case 0x9D: // SBC A,LY
      case 0x9E: // SBC A,(IY+dd)
      case 0xA4: // AND HY
      case 0xA5: // AND LY
      case 0xA6: // AND (IY+dd)
      case 0xAC: // XOR HY
      case 0xAD: // XOR LY
      case 0xAE: // XOR (IY+dd)
      case 0xB4: // OR HY
      case 0xB5: // OR LY
      case 0xB6: // OR (IY+dd)
      case 0xBC: // CP HY
      case 0xBD: // CP LY
      case 0xBE: // CP (IY+dd)
        return(inst_fd_misc(code));
      break;

      case 0xCB: // escape, IY prefix to CB commands
        return(inst_fdcb()); /* see inst_fdcb.cc */
      break;

      case 0xE1: // POP IY
        regs.IY = get2(regs.SP);
        regs.SP+=2;
      return(resGO);

      case 0xE3: // EX (SP),IY
        {
          TYPE_UWORD tempw;

          tempw = regs.IY;
          regs.IY = get1(regs.SP);
          store1(regs.SP, tempw);
        }
      return(resGO);

      case 0xE5: // PUSH IY
        push2(regs.IY);
      return(resGO);

      case 0xE9: // JP (IY)
        PC = get2(regs.IY);
      return(resGO);

      default:
      return(resINV_INST);
    }
  return(resINV_INST);
}

/* End of z80.src/inst_fd.cc */
