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

#define store2(addr, val) ram->set((t_addr) (addr), val)
#define store1(addr, val) ram->set((t_addr) (addr), val)
#define get1(addr) ram->get((t_addr) (addr))
#define get2(addr) (ram->get((t_addr) (addr)) | (ram->get((t_addr) (addr)) << 8) )
#define fetch2() (fetch() | (fetch() << 8))
#define push2(val) {regs.SP-=2; store2(regs.SP,(val));}
#define pop2(var) {var=get2(regs.SP),regs.SP+=2;}

/*
 * No Instruction
 * NOP
 * 0000 0000 0000 0000
 *----------------------------------------------------------------------------
 */

int
cl_z80::inst_nop(t_mem code)
{
  //ticks(4);
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
      //ticks(12);
      regs.BC = fetch2();
    break;
    case 2: // LD (BC),A
      // ticks(?);
      store1(regs.BC, regs.A);
    break;
    case 6: // LD B,nn
      // ticks(?);
      regs.bc.l = fetch();
    break;
    case 0xa: // LD A,(BC)
      // ticks(?);
      regs.A = get1(regs.BC);
    break;
    case 0x0e: // LD C,nn
      // ticks(?);
      regs.bc.h = fetch();
    break;


    case 0x31: // LD SP,nnnn
      // ticks(?);
      regs.SP = fetch2();
    break;
  }
  return(resGO);
}

int
cl_z80::inst_inc(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_dec(t_mem code)
{
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
  return(resGO);
}

int
cl_z80::inst_djnz(t_mem code)
{
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
  return(resGO);
}

int
cl_z80::inst_pop(t_mem code)
{
  return(resGO);
}

int
cl_z80::inst_jp(t_mem code)
{
  PC = fetch2();
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

/* End of z80.src/inst.cc */
