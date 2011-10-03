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
#include "r2kcl.h"
#include "z80mac.h"


/******** rabbit 2000 memory access helper functions *****************/
TYPE_UDWORD  rabbit_mmu::logical_addr_to_phys( TYPE_UWORD logical_addr ) {
  TYPE_UDWORD  phys_addr = logical_addr;
  unsigned     segnib = logical_addr >> 12;
  
  if (segnib >= 0xE000)
  {
    phys_addr += ((TYPE_UDWORD)xpc) << 12;
  }
  else if (segnib >= ((segsize >> 4) & 0x0F))
  {
    phys_addr += ((TYPE_UDWORD)stackseg) << 12;    
  }
  else if (segnib >= (segsize & 0x0F))
  {
    phys_addr += ((TYPE_UDWORD)dataseg) << 12;    
  }
  
  return phys_addr;
}

void cl_r2k::store1( TYPE_UWORD addr, t_mem val ) {
  TYPE_UDWORD  phys_addr;
  
  if (mmu.io_flag == IOI) {
    if ((mmu.mmidr ^ 0x80) & 0x80)
      /* bit 7 = 0 --> use only 8-bits for internal I/O addresses */
      addr = addr & 0x0ff;
    
    if (addr == MMIDR) {
      mmu.mmidr = val;
      return;
    }
    
    if (addr == SADR) {
      /* serial A (console when using the rabbit programming cable) */
      putchar(val);
      fflush(stdout);
    }
    return;
  }
  if (mmu.io_flag == IOE) {
    /* I/O operation for external device (such as an ethernet controller) */
    return;
  }
  
  phys_addr = mmu.logical_addr_to_phys( addr );
  ram->set(phys_addr, val);
}

void cl_r2k::store2( TYPE_UWORD addr, TYPE_UWORD val ) {
  TYPE_UDWORD  phys_addr;
  
  if (mmu.io_flag == IOI) {
    /* I/O operation for on-chip device (serial ports, timers, etc) */
    return;
  }
  
  if (mmu.io_flag == IOE) {
    /* I/O operation for external device (such as an ethernet controller) */
    return;
  }
  
  phys_addr = mmu.logical_addr_to_phys( addr );
  
  ram->set(phys_addr,   val & 0xff);
  ram->set(phys_addr+1, (val >> 8) & 0xff);
}

TYPE_UBYTE  cl_r2k::get1( TYPE_UWORD addr ) {
  TYPE_UDWORD  phys_addr = mmu.logical_addr_to_phys( addr );
  
  if (mmu.io_flag == IOI) {
    /* stub for on-chip device I/O */
    return 0;
  }
  if (mmu.io_flag == IOE) {
    /* stub for external device I/O */
    return 0;
  }
  
  return ram->get(phys_addr);
}

TYPE_UWORD  cl_r2k::get2( TYPE_UWORD addr ) {
  TYPE_UDWORD phys_addr = mmu.logical_addr_to_phys( addr );
  TYPE_UWORD  l, h;
  
  if (mmu.io_flag == IOI) {
    /* stub for on-chip device I/O */
    return 0;
  }
  if (mmu.io_flag == IOE) {
    /* stub for external device I/O */
    return 0;
  }
  
  l = ram->get(phys_addr  );
  h = ram->get(phys_addr+1);
  
  return (h << 8) | l;
}

t_mem       cl_r2k::fetch1( void ) {
  return fetch( );
}

TYPE_UWORD  cl_r2k::fetch2( void ) {
  TYPE_UWORD  c1, c2;
  
  c1 = fetch( );
  c2 = fetch( );
  return (c2 << 8) | c1;
}

t_mem cl_r2k::fetch(void) {
  /*
   * Fetch without checking for breakpoint hit
   *
   * Used by bool cl_uc::fetch(t_mem *code) in sim.src/uc.cc
   * which does check for a breakpoint hit
   */
  
  TYPE_UDWORD phys_addr = mmu.logical_addr_to_phys( PC );
  ulong code;
  
  if (!rom)
    return(0);
  
  code= rom->read(phys_addr);
  PC = (PC + 1) & 0xffffUL;
  return(code);
}

/******** start rabbit 2000 specific codes *****************/
int cl_r2k::inst_add_sp_d(t_mem code) {
  regs.SP = add_u16_disp(regs.SP, fetch());
  return(resGO);
}

int cl_r2k::inst_altd(t_mem code) {
  // stub
  return(resGO);
}

int
cl_r2k::inst_r2k_ld(t_mem code)
{
  /* 0xC4  ld hl,(sp+n)
   * 0xD4  ld (sp+n),hl
   * 0xE4  ld hl,(ix+d)
   *   DD E4 = ld hl,(hl+d)   [note: (hl+d) breaks the normal prefix pattern]
   *   FD E4 = ld hl,(iy+d)
   * 0xF4  ld (ix+d),hl
   *   DD F4 = ld (hl+d),hl
   *   FD F4 = ld (iy+d),hl
   */
  switch(code) {
  case 0xC4:  regs.HL = get2( add_u16_disp(regs.SP, fetch()) ); break;
  case 0xD4:  store2( add_u16_disp(regs.SP, fetch()), regs.HL ); break;
  case 0xE4:  regs.HL = get2( add_u16_disp(regs.IX, fetch()) ); break;
  case 0xF4:  store2( add_u16_disp(regs.IX, fetch()), regs.HL ); break;
  default:
    return(resINV_INST);
  }
  
  return(resGO);
}

int cl_r2k::inst_ljp(t_mem code) {
  TYPE_UWORD  mn;
  
  mn = fetch2();  /* don't clobber PC before the fetch for xmem page */
  mmu.xpc = fetch1();
  PC = mn;
  
  return(resGO);
}

int cl_r2k::inst_lcall(t_mem code) {
  TYPE_UWORD  mn;
  
  push1(mmu.xpc);
  push2(PC+2);
  
  mn = fetch2();  /* don't clobber PC before the fetch for xmem page */
  mmu.xpc = fetch1();
  PC = mn;
  
  return(resGO);
}

int cl_r2k::inst_mul(t_mem code) {
  long m = (long)(regs.BC & 0x7fff) * (long)(regs.DE & 0x7fff);
  m = ((regs.BC & 0x8000) ^ (regs.DE & 0x8000)) ? -m : m;
  regs.BC = ((unsigned long)(m) & 0xffff);
  regs.HL = m / (1 << 15);
  return(resGO);
}

int cl_r2k::inst_rl_de(t_mem code) {
return(resINV_INST);
  return(resGO);
}

int cl_r2k::inst_rr_de(t_mem code) {
return(resINV_INST);
  return(resGO);
}

int cl_r2k::inst_rr_hl(t_mem code) {
return(resINV_INST);
  return(resGO);
}


int
cl_r2k::inst_rst(t_mem code)
{
  switch(code) {
    case 0xC7: // RST 0
      push2(PC+2);
      PC = 0x0;
    break;
    case 0xCF: // RST 8
      return(resINV_INST);
    
    case 0xD7: // RST 10H
      push2(PC+2);
      PC = 0x10;
    break;
    case 0xDF: // RST 18H
      push2(PC+2);
      PC = 0x18;
    break;
    case 0xE7: // RST 20H
      push2(PC+2);
      PC = 0x20;
    break;
    case 0xEF: // RST 28H
      //PC = 0x28;
      switch (regs.A) {
        case 0:
          return(resBREAKPOINT);
//          ::exit(0);
        break;

        case 1:
          //printf("PUTCHAR-----> %xH\n", regs.hl.l);
          putchar(regs.hl.l);
          fflush(stdout);
        break;
      }
    break;
    case 0xF7: // RST 30H
      return(resINV_INST);  // opcode is used for MUL on rabbit 2000+
    break;
    case 0xFF: // RST 38H
      push2(PC+2);
      PC = 0x38;
    break;
    default:
      return(resINV_INST);
    break;
  }
  return(resGO);
}
