/*
 * Simulator of microcontrollers (inst.cc)
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

#define store2(addr, val) { ram->set((t_addr) (addr), val & 0xff); \
                            ram->set((t_addr) (addr+1), (val >> 8) & 0xff); }
#define store1(addr, val) ram->set((t_addr) (addr), val)
#define get1(addr) ram->get((t_addr) (addr))
#define get2(addr) (ram->get((t_addr) (addr)) | (ram->get((t_addr) (addr+1)) << 8) )
#define fetch2() (fetch() | (fetch() << 8))
#define fetch1() fetch()
#define push2(val) {regs.SP-=2; store2(regs.SP,(val));}
#define push1(val) {regs.SP-=1; store1(regs.SP,(val));}
#define pop2(var) {var=get2(regs.SP),regs.SP+=2;}
//#define pop1(var) {var=get1(regs.SP),regs.SP+=1;}

/*
 * No Instruction
 * NOP
 * 0000 0000 0000 0000
 *----------------------------------------------------------------------------
 */

int
cl_z80::inst_nop(t_mem code)
{
  return(resGO);
}

/*
 * Load Instruction
 * LD
 * 
 *----------------------------------------------------------------------------
 */

int
cl_z80::inst_ld(t_mem code)
{
  switch(code) {
    case 1:  // LD BC,nnnn
      regs.BC = fetch2();
    break;
    case 2: // LD (BC),A
      store1(regs.BC, regs.A);
    break;
    case 6: // LD B,nn
      regs.bc.l = fetch();
    break;
    case 0xa: // LD A,(BC)
      regs.A = get1(regs.BC);
    break;
    case 0x0e: // LD C,nn
      regs.bc.l = fetch();
    break;
    case 0x11: // LD DE,nnnn
      regs.DE = fetch2();
    break;
    case 0x12: // LD DE,A
      regs.DE = regs.A;
    break;
    case 0x16: // LD D,nn
      regs.de.h = fetch();
    break;
    case 0x1A: // LD A,DE
      regs.A = regs.DE;
    break;
    case 0x1E: // LD E,nn
      regs.de.l = fetch();
    break;
    case 0x21: // LD HL,nnnn
      regs.HL = fetch2();
    break;
    case 0x22: // LD (nnnn),HL
      store2(fetch2(), regs.HL);
    break;
    case 0x26: // LD H,nn
      regs.hl.h = fetch();
    break;
    case 0x2A: // LD HL,(nnnn)
      regs.HL = fetch2();
    break;
    case 0x2E: // LD L,nn
      regs.hl.l = fetch();
    break;
    case 0x31: // LD SP,nnnn
      regs.SP = fetch2();
    break;
    case 0x32: // LD (nnnn),A
      store2(fetch2(), regs.A);
    break;
    case 0x36: // LD (HL),nn
      store1(regs.HL, fetch());
    break;
    case 0x3A: // LD A,(nnnn)
      regs.A = get2(fetch2());
    break;
    case 0x3E: // LD A,nn
      regs.A = fetch();
    break;
    case 0x40: // LD B,B
    break;
    case 0x41: // LD B,C
      regs.bc.h = regs.bc.l;
    break;
    case 0x42: // LD B,D
      regs.bc.h = regs.de.h;
    break;
    case 0x43: // LD B,E
      regs.bc.h = regs.de.l;
    break;
    case 0x44: // LD B,H
      regs.bc.h = regs.hl.h;
    break;
    case 0x45: // LD B,L
      regs.bc.h = regs.hl.l;
    break;
    case 0x46: // LD B,(HL)
      regs.bc.h = get1(regs.HL);
    break;
    case 0x47: // LD B,A
      regs.bc.h = regs.A;
    break;
    case 0x48: // LD C,B
      regs.bc.l = regs.bc.h;
    break;
    case 0x49: // LD C,C
    break;
    case 0x4A: // LD C,D
      regs.bc.l = regs.de.h;
    break;
    case 0x4B: // LD C,E
      regs.bc.l = regs.de.l;
    break;
    case 0x4C: // LD C,H
      regs.bc.l = regs.hl.h;
    break;
    case 0x4D: // LD C,L
      regs.bc.l = regs.hl.l;
    break;
    case 0x4E: // LD C,(HL)
      regs.bc.l = get1(fetch());
    break;
    case 0x4F: // LD C,A
      regs.bc.l = regs.A;
    break;
    case 0x50: // LD D,B
      regs.de.h = regs.bc.h;
    break;
    case 0x51: // LD D,C
      regs.de.h = regs.bc.l;
    break;
    case 0x52: // LD D,D
    break;
    case 0x53: // LD D,E
      regs.de.h = regs.de.l;
    break;
    case 0x54: // LD D,H
      regs.de.h = regs.hl.h;
    break;
    case 0x55: // LD D,L
      regs.de.h = regs.hl.l;
    break;
    case 0x56: // LD D,(HL)
      regs.de.h = get1(regs.HL);
    break;
    case 0x57: // LD D,A
      regs.de.l = regs.A;
    break;
    case 0x58: // LD E,B
      regs.de.l = regs.bc.h;
    break;
    case 0x59: // LD E,C
      regs.de.l = regs.bc.l;
    break;
    case 0x5A: // LD E,D
      regs.de.l = regs.de.h;
    break;
    case 0x5B: // LD E,E
    break;
    case 0x5C: // LD E,H
      regs.de.l = regs.hl.h;
    break;
    case 0x5D: // LD E,L
      regs.de.l = regs.hl.l;
    break;
    case 0x5E: // LD E,(HL)
      regs.de.l = get1(regs.HL);
    break;
    case 0x5F: // LD E,A
      regs.de.l = regs.A;
    break;
    case 0x60: // LD H,B
      regs.hl.h = regs.bc.h;
    break;
    case 0x61: // LD H,C
      regs.hl.h = regs.bc.l;
    break;
    case 0x62: // LD H,D
      regs.hl.h = regs.de.h;
    break;
    case 0x63: // LD H,E
      regs.hl.h = regs.de.l;
    break;
    case 0x64: // LD H,H
      regs.hl.h = regs.hl.h;
    break;
    case 0x65: // LD H,L
      regs.hl.h = regs.hl.l;
    break;
    case 0x66: // LD H,(HL)
      regs.hl.h = get1(regs.HL);
    break;
    case 0x67: // LD H,A
      regs.hl.h = regs.A;
    break;
    case 0x68: // LD L,B
      regs.hl.l = regs.bc.h;
    break;
    case 0x69: // LD L,C
      regs.hl.l = regs.bc.l;
    break;
    case 0x6A: // LD L,D
      regs.hl.l = regs.de.h;
    break;
    case 0x6B: // LD L,E
      regs.hl.l = regs.de.l;
    break;
    case 0x6C: // LD L,H
      regs.hl.l = regs.hl.h;
    break;
    case 0x6D: // LD L,L
      regs.hl.l = regs.hl.l;
    break;
    case 0x6E: // LD L,(HL)
      regs.hl.l = get1(regs.HL);
    break;
    case 0x6F: // LD L,A
      regs.hl.l = regs.A;
    break;
    case 0x70: // LD (HL),B
      store1(regs.HL, regs.bc.h);
    break;
    case 0x71: // LD (HL),C
      store1(regs.HL, regs.bc.l);
    break;
    case 0x72: // LD (HL),D
      store1(regs.HL, regs.de.h);
    break;
    case 0x73: // LD (HL),E
      store1(regs.HL, regs.de.l);
    break;
    case 0x74: // LD (HL),H
      store1(regs.HL, regs.hl.h);
    break;
    case 0x75: // LD (HL),L
      store1(regs.HL, regs.hl.l);
    break;
    case 0x76: // LD (HL),A
      store1(regs.HL, regs.de.h);
    break;
    case 0x78: // LD A,B
      regs.A = regs.bc.h;
    break;
    case 0x79: // LD A,C
      regs.A = regs.bc.l;
    break;
    case 0x7A: // LD A,D
      regs.A = regs.de.h;
    break;
    case 0x7B: // LD A,E
      regs.A = regs.de.l;
    break;
    case 0x7C: // LD A,H
      regs.A = regs.hl.h;
    break;
    case 0x7D: // LD A,L
      regs.A = regs.hl.l;
    break;
    case 0x7E: // LD A,(HL)
      regs.A = get1(regs.HL);
    break;
    case 0x7F: // LD A,A
    break;
    case 0xF9: // LD SP,HL
      regs.SP = regs.HL;
    break;
  }
  return(resGO);
}

int
cl_z80::inst_inc(t_mem code)
{
  switch(code) {
    case 0x03: // INC BC
      ++regs.BC;
    break;
    case 0x04: // INC B
      ++regs.bc.h;
    break;
    case 0x0C: // INC C
      ++regs.bc.l;
    break;
    case 0x13: // INC DE
      ++regs.DE;
    break;
    case 0x14: // INC D
      ++regs.de.h;
    break;
    case 0x1C: // INC E
      ++regs.de.l;
    break;
    case 0x23: // INC HL
      ++regs.HL;
    break;
    case 0x24: // INC H
      ++regs.hl.h;
    break;
    case 0x2C: // INC L
      ++regs.hl.l;
    break;
    case 0x33: // INC SP
      ++regs.SP;
    break;
    case 0x34: // INC (HL)
      store1(regs.HL, get1(regs.HL)+1);
    break;
    case 0x3C: // INC A
      ++regs.A;
    break;
  }
  return(resGO);
}

int
cl_z80::inst_dec(t_mem code)
{
  switch(code) {
    case 0x05: // DEC B
      --regs.bc.h;
    break;
    case 0x0B: // DEC BC
      --regs.BC;
    break;
    case 0x0D: // DEC C
      --regs.bc.l;
    break;
    case 0x15: // DEC D
      --regs.de.h;
    break;
    case 0x1B: // DEC DE
      --regs.DE;
    break;
    case 0x1D: // DEC E
      --regs.de.l;
    break;
    case 0x25: // DEC H
      --regs.hl.h;
    break;
    case 0x2B: // DEC HL
      --regs.HL;
    break;
    case 0x2D: // DEC L
      --regs.hl.l;
    break;
    case 0x35: // DEC (HL)
      store1(regs.HL, get1(regs.HL)-1);
    break;
    case 0x3B: // DEC SP
      --regs.SP;
    break;
    case 0x3D: // DEC A
      --regs.A;
    break;
  }
  return(resGO);
}

int
cl_z80::inst_rlca(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_rrca(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_ex(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_add(t_mem code)
{
  switch(code) {
    case 0x09: // ADD HL,BC
      regs.HL += regs.BC;
    break;
    case 0x19: // ADD HL,DE
      regs.HL += regs.DE;
    break;
    case 0x29: // ADD HL,HL
      regs.HL += regs.HL;
    break;
    case 0x39: // ADD HL,SP
      regs.HL += regs.SP;
    break;
    case 0x80: // ADD A,B
      regs.A += regs.bc.h;
    break;
    case 0x81: // ADD A,C
      regs.A += regs.bc.l;
    break;
    case 0x82: // ADD A,D
      regs.A += regs.de.h;
    break;
    case 0x83: // ADD A,E
      regs.A += regs.de.l;
    break;
    case 0x84: // ADD A,H
      regs.A += regs.hl.h;
    break;
    case 0x85: // ADD A,L
      regs.A += regs.hl.l;
    break;
    case 0x86: // ADD A,(HL)
      regs.A += get1(regs.HL);
    break;
    case 0x87: // ADD A,A
      regs.A += regs.A;
    break;
    case 0xC6: // ADD A,nn
      regs.A += fetch();
    break;
  }

  return(resGO);
}

int
cl_z80::inst_djnz(t_mem code)
{
  signed char j;

  // 0x10: DJNZ dd

  j = fetch1();
  if ((--regs.bc.h != 0)) {
    PC += j;
  } else {
  }
  return(resGO);
}

int
cl_z80::inst_rra(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_rla(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_jr(t_mem code)
{
  signed char j;

  j = fetch1();
  switch(code) {
    case 0x18: // JR dd
      PC += j;
    break;
    case 0x20: // JR NZ,dd
      if (!(regs.F & BIT_Z)) {
        PC += j;
      } else {
      }
    break;
    case 0x28: // JR Z,dd
      if ((regs.F & BIT_Z)) {
        PC += j;
      } else {
      }
    break;
    case 0x30: // JR NC,dd
      if (!(regs.F & BIT_C)) {
        PC += j;
      } else {
      }
    break;
    case 0x38: // JR C,dd
      if ((regs.F & BIT_C)) {
        PC += j;
      } else {
      }
    break;
  }
  return(resGO);
}

int
cl_z80::inst_daa(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_cpl(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_scf(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_ccf(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_halt(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_adc(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_sbc(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_and(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_xor(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_or(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_cp(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_rst(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_ret(t_mem code)
{
  pop2(PC);
  return(resGO);
}

int
cl_z80::inst_call(t_mem code)
{
  push2(PC+2);
  PC = fetch2();

  return(resGO);
}

int
cl_z80::inst_out(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_push(t_mem code)
{
  switch(code) {
    case 0xC5: // PUSH BC
      push2(regs.BC);
    break;
    case 0xD5: // PUSH DE
      push2(regs.DE);
    break;
    case 0xE5: // PUSH HL
      push2(regs.HL);
    break;
    case 0xF5: // PUSH AF
      push1(regs.F);
      push1(regs.A);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_exx(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_in(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_sub(t_mem code)
{
  switch(code) {
    case 0x90: // SUB B
      regs.A -= regs.bc.h;
    break;
    case 0x91: // SUB C
      regs.A -= regs.bc.l;
    break;
    case 0x92: // SUB D
      regs.A -= regs.de.h;
    break;
    case 0x93: // SUB E
      regs.A -= regs.de.l;
    break;
    case 0x94: // SUB H
      regs.A -= regs.hl.h;
    break;
    case 0x95: // SUB L
      regs.A -= regs.hl.l;
    break;
    case 0x96: // SUB (HL)
      regs.A -= (get1(regs.HL));
    break;
    case 0x97: // SUB A
      regs.A = 0;
    break;
    case 0xD6: // SUB nn
      regs.A -= fetch();
    break;
  }
  return(resGO);
}

int
cl_z80::inst_pop(t_mem code)
{
  switch (code) {
    case 0xC1: // POP BC
      regs.BC = get2(regs.SP);
      regs.SP+=2;
    break;
    case 0xD1: // POP DE
      regs.DE = get2(regs.SP);
      regs.SP+=2;
    break;
    case 0xE1: // POP HL
      regs.HL = get2(regs.SP);
      regs.SP+=2;
    break;
    case 0xF1: // POP AF
      regs.A = get1(regs.SP++);
      regs.F = get1(regs.SP++);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_jp(t_mem code)
{
  int jnk;

  switch (code) {
    case 0xC2: // JP NZ,nnnn
      if (!(regs.F & BIT_Z)) {
        PC = fetch2();
      } else {
        jnk = fetch2();
      }
    break;
    case 0xC3: // JP nnnn
      PC = fetch2();
    break;
    case 0xCA: // JP Z,nnnn
      if (regs.F & BIT_Z) {
        PC = fetch2();
      } else {
        jnk = fetch2();
      }
    break;
    case 0xD2: // JP NC,nnnn
      if (!(regs.F & BIT_C)) {
        PC = fetch2();
      } else {
        jnk = fetch2();
      }
    break;
    case 0xDA: // JP C,nnnn
      if (regs.F & BIT_C) {
        PC = fetch2();
      } else {
        jnk = fetch2();
      }
    break;
    case 0xE2: // JP PO,nnnn
      if (regs.F & BIT_P) {
        PC = fetch2();
      } else {
        jnk = fetch2();
      }
    break;
    case 0xE9: // JP (HL)
      PC = get2(regs.HL);
    break;
  }
  return(resGO);
}

int
cl_z80::inst_di(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_ei(t_mem code)
{
  return(resGO);
}

/********************************************/
/**** start a inst_dd.cc file? **************/
/********************************************/

int
cl_z80::inst_dd_ld(t_mem code)
{
  switch (code) {
    case 0x21: // LD IX,nnnn
      regs.IX = fetch2();
    return(resGO);
    case 0x22: // LD (nnnn),IX
      store2(fetch2(), regs.IX);
    return(resGO);
    case 0x26: // LD HX,nn
      regs.hl.h = fetch1();
    return(resGO);
    case 0x2A: // LD IX,(nnnn)
      regs.IX = get2(fetch2());
    return(resGO);
    case 0x2E: // LD LX,nn
      regs.hl.l = fetch1();
    return(resGO);
    case 0x36: // LD (IX+dd),nn
      store1(regs.IX+fetch(), fetch());
    return(resGO);
    case 0x44: // LD B,HX
      regs.bc.h = regs.hl.h;
    return(resGO);
    case 0x45: // LD B,LX
      regs.bc.h = regs.hl.l;
    return(resGO);
    case 0x46: // LD B,(IX+dd)
      regs.bc.h = get1(regs.IX+fetch());
    return(resGO);
    case 0x4C: // LD C,HX
      regs.bc.l = regs.hl.h;
    return(resGO);
    case 0x4D: // LD C,LX
      regs.bc.l = regs.hl.l;
    return(resGO);
    case 0x4E: // LD C,(IX+dd)
      regs.bc.l = get1(regs.IX+fetch());
    return(resGO);
    case 0x54: // LD D,HX
      regs.de.h = regs.hl.h;
    return(resGO);
    case 0x55: // LD D,LX
      regs.de.h = regs.hl.l;
    return(resGO);
    case 0x56: // LD D,(IX+dd)
      regs.de.h = get1(regs.IX+fetch());
    return(resGO);
    case 0x5C: // LD E,H
      regs.de.l = regs.hl.h;
    return(resGO);
    case 0x5D: // LD E,L
      regs.de.l = regs.hl.l;
    return(resGO);
    case 0x5E: // LD E,(IX+dd)
      regs.de.l = get1(regs.IX+fetch());
    return(resGO);
    case 0x60: // LD HX,B
      regs.hl.h = regs.bc.h;
    return(resGO);
    case 0x61: // LD HX,C
      regs.hl.h = regs.bc.l;
    return(resGO);
    case 0x62: // LD HX,D
      regs.hl.h = regs.de.h;
    return(resGO);
    case 0x63: // LD HX,E
      regs.hl.h = regs.de.l;
    return(resGO);
    case 0x64: // LD HX,HX
    return(resGO);
    case 0x65: // LD HX,LX
      regs.hl.h = regs.hl.l;
    return(resGO);
    case 0x66: // LD H,(IX+dd)
      regs.hl.h = get1(regs.IX+fetch());
    return(resGO);
    case 0x67: // LD HX,A
      regs.hl.h = regs.A;
    return(resGO);
    case 0x68: // LD LX,B
      regs.hl.l = regs.bc.h;
    return(resGO);
    case 0x69: // LD LX,C
      regs.hl.l = regs.bc.l;
    return(resGO);
    case 0x6A: // LD LX,D
      regs.hl.l = regs.de.h;
    return(resGO);
    case 0x6B: // LD LX,E
      regs.hl.l = regs.de.l;
    return(resGO);
    case 0x6C: // LD LX,HX
      regs.hl.l = regs.hl.h;
    return(resGO);
    case 0x6D: // LD LX,LX
    return(resGO);
    case 0x6E: // LD L,(IX+dd)
      regs.hl.l = get1(regs.IX+fetch());
    return(resGO);
    case 0x6F: // LD LX,A
      regs.hl.l = regs.A;
    return(resGO);
    case 0x70: // LD (IX+dd),B
      store1(regs.IX+fetch(), regs.bc.h);
    return(resGO);
    case 0x71: // LD (IX+dd),C
      store1(regs.IX+fetch(), regs.bc.l);
    return(resGO);
    case 0x72: // LD (IX+dd),D
      store1(regs.IX+fetch(), regs.de.h);
    return(resGO);
    case 0x73: // LD (IX+dd),E
      store1(regs.IX+fetch(), regs.de.l);
    return(resGO);
    case 0x74: // LD (IX+dd),H
      store1(regs.IX+fetch(), regs.hl.h);
    return(resGO);
    case 0x75: // LD (IX+dd),L
      store1(regs.IX+fetch(), regs.hl.l);
    return(resGO);
    case 0x77: // LD (IX+dd),A
      store1(regs.IX+fetch(), regs.A);
    return(resGO);
    case 0x7C: // LD A,HX
      regs.A = regs.hl.h;
    return(resGO);
    case 0x7D: // LD A,LX
      regs.A = regs.hl.l;
    return(resGO);
    case 0x7E: // LD A,(IX+dd)
      regs.A = get1(regs.IX+fetch());
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
  switch (code) {
    case 0x09: // ADD IX,BC
      regs.IX += regs.BC;
      return(resGO);
    case 0x19: // ADD IX,DE
      regs.IX += regs.DE;
      return(resGO);
    case 0x29: // ADD IX,IX
      regs.IX += regs.IX;
      return(resGO);
    case 0x39: // ADD IX,SP
      regs.IX += regs.SP;
    return(resGO);
    case 0x84: // ADD A,HX
      regs.A += regs.IX;
      return(resGO);
    case 0x85: // ADD A,LX
      regs.A += regs.IX;
      return(resGO);
    case 0x86: // ADD A,(IX)
      regs.A += get2(regs.IX);
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
      ++regs.hl.h;
    break;
    case 0x34: // INC (IX+dd)
      {
        t_addr addr;
        addr = regs.IX+fetch();
        store1(addr, get1(addr)+1);
      }
    break;
  }
}

int
cl_z80::inst_dd_dec(t_mem code)
{
  switch(code) {
    case 0x25: // DEC HX
      --regs.hl.h;
    break;
    case 0x2B: // DEC IX
      --regs.IX;
    break;
    case 0x2C: // INC LX
      ++regs.hl.l;
    break;
    case 0x2D: // DEC LX
      --regs.hl.l;
    break;
    case 0x35: // DEC (IX+dd)
      {
        t_addr addr;
        addr = regs.IX+fetch();
        store1(addr, get1(addr)-1);
      }
    break;
  }
}

int
cl_z80::inst_dd(void)
{
  t_mem code;

  if (fetch(&code))
    return(resBREAKPOINT);
  tick(1);
  switch (code)
    {
      case 0xe5: return(inst_dd_push(code));

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
      case 0x34: // INC (IX+dd)
        return(inst_dd_inc(code));

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
      case 0x2C: // INC LX
      case 0x2D: // DEC LX
      case 0x35: // DEC (IX+dd)
      return(inst_dd_dec(code));
    }
  if (PC)
    PC--;
  else
    PC= get_mem_size(MEM_ROM)-1;
  return(resINV_INST);
}

/* End of z80.src/inst.cc */
