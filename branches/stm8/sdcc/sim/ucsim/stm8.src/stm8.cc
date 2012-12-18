/*
 * Simulator of microcontrollers (stm8.cc)
 *
 * some stm8 code base from Karl Bongers karl@turbobit.com
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

#include <stdarg.h> /* for va_list */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "i_string.h"

// prj
#include "pobjcl.h"

// sim
#include "simcl.h"

// local
#include "stm8cl.h"
#include "glob.h"
#include "regsstm8.h"
#include "stm8mac.h"

#define uint32 t_addr
#define uint8 unsigned char

/*******************************************************************/


/*
 * Base type of STM8 controllers
 */

cl_stm8::cl_stm8(class cl_sim *asim):
  cl_uc(asim)
{
  type= CPU_STM8;
}

int
cl_stm8::init(void)
{
  cl_uc::init(); /* Memories now exist */

  xtal = 8000000;

  rom = address_space(MEM_ROM_ID);
//  ram = mem(MEM_XRAM);
  ram = rom;

  // zero out ram(this is assumed in regression tests)
  for (int i=0x0; i<0x8000; i++) {
    ram->set((t_addr) i, 0);
  }

  return(0);
}


void
cl_stm8::reset(void)
{
  cl_uc::reset();

  regs.SP = 0x7ff;
  regs.A = 0;
  regs.X = 0;
  regs.Y = 0;
  regs.CC = 0x00;
  regs.VECTOR = 1;

}


const char *
cl_stm8::id_string(void)
{
  return("unspecified STM8");
}


/*
 * Making elements of the controller
 */
/*
t_addr
cl_stm8::get_mem_size(enum mem_class type)
{
  switch(type)
    {
    case MEM_ROM: return(0x10000);
    case MEM_XRAM: return(0x10000);
    default: return(0);
    }
 return(cl_uc::get_mem_size(type));
}
*/
void
cl_stm8::mk_hw_elements(void)
{
  //class cl_base *o;
  /* t_uc::mk_hw() does nothing */
}

void
cl_stm8::make_memories(void)
{
  class cl_address_space *as;

  as= new cl_address_space("rom", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip= new cl_memory_chip("rom_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("rom"), chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
}


/*
 * Help command interpreter
 */

struct dis_entry *
cl_stm8::dis_tbl(void)
{
  return(disass_stm8);
}

/*struct name_entry *
cl_stm8::sfr_tbl(void)
{
  return(0);
}*/

/*struct name_entry *
cl_stm8::bit_tbl(void)
{
  //FIXME
  return(0);
}*/

int
cl_stm8::inst_length(t_addr addr)
{
  int len = 0;

  get_disasm_info(addr, &len, NULL, NULL);

  return len;
}

int
cl_stm8::inst_branch(t_addr addr)
{
  int b;

  get_disasm_info(addr, NULL, &b, NULL);

  return b;
}

int
cl_stm8::longest_inst(void)
{
  return 5;
}


const char *
cl_stm8::get_disasm_info(t_addr addr,
                        int *ret_len,
                        int *ret_branch,
                        int *immed_offset)
{
  const char *b = NULL;
  uint code;
  int len = 0;
  int immed_n = 0;
  int i;
  int start_addr = addr;
  struct dis_entry *dis_e;

  code= get_mem(MEM_ROM_ID, addr++);
  dis_e = NULL;

  switch(code) {
	/* here will be all the prefixes for STM8 */
	case 0x72 :
	  code= get_mem(MEM_ROM_ID, addr++);
      i= 0;
      while ((code & disass_stm8_72[i].mask) != disass_stm8_72[i].code &&
        disass_stm8_72[i].mnemonic)
        i++;
      dis_e = &disass_stm8_72[i];
      b= disass_stm8_72[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8_72[i].length + 1);
    break;
	  
	case 0x90 :
	  code= get_mem(MEM_ROM_ID, addr++);
      i= 0;
      while ((code & disass_stm8_90[i].mask) != disass_stm8_90[i].code &&
        disass_stm8_90[i].mnemonic)
        i++;
      dis_e = &disass_stm8_90[i];
      b= disass_stm8_90[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8_90[i].length + 1);
    break;
	  
	case 0x91 :
	  code= get_mem(MEM_ROM_ID, addr++);
      i= 0;
      while ((code & disass_stm8_91[i].mask) != disass_stm8_91[i].code &&
        disass_stm8_91[i].mnemonic)
        i++;
      dis_e = &disass_stm8_91[i];
      b= disass_stm8_91[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8_91[i].length + 1);
    break;
	  
	case 0x92 :
	  code= get_mem(MEM_ROM_ID, addr++);
      i= 0;
      while ((code & disass_stm8_92[i].mask) != disass_stm8_92[i].code &&
        disass_stm8_92[i].mnemonic)
        i++;
      dis_e = &disass_stm8_92[i];
      b= disass_stm8_92[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8_92[i].length + 1);
    break;
	  
	
    default:
      i= 0;
      while ((code & disass_stm8[i].mask) != disass_stm8[i].code &&
             disass_stm8[i].mnemonic)
        i++;
      dis_e = &disass_stm8[i];
      b= disass_stm8[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8[i].length);
    break;
  }

  if (ret_branch) {
    *ret_branch = dis_e->branch;
  }

  if (immed_offset) {
    if (immed_n > 0)
         *immed_offset = immed_n;
    else *immed_offset = (addr - start_addr);
  }

  if (len == 0)
    len = 1;

  if (ret_len)
    *ret_len = len;

  return b;
}

const char *
cl_stm8::disass(t_addr addr, const char *sep)
{
  char work[256], temp[20];
  const char *b;
  char *buf, *p, *t;
  int len = 0;
  int immed_offset = 0;

  p= work;

  b = get_disasm_info(addr, &len, NULL, &immed_offset);

  if (b == NULL) {
    buf= (char*)malloc(30);
    strcpy(buf, "UNKNOWN/INVALID");
    return(buf);
  }

  while (*b)
    {
      if (*b == '%')
        {
          b++;
          switch (*(b++))
            {
            case 's': // s    signed byte immediate
              sprintf(temp, "#%d", (char)get_mem(MEM_ROM_ID, addr+immed_offset));
              ++immed_offset;
              break;
            case 'e': // e    extended 24bit immediate operand
              sprintf(temp, "#0x%06lx",
                 (ulong)((get_mem(MEM_ROM_ID, addr+immed_offset)<<16) |
                        (get_mem(MEM_ROM_ID, addr+immed_offset+1)<<8) |
                        (get_mem(MEM_ROM_ID, addr+immed_offset+2))) );
              ++immed_offset;
              ++immed_offset;
              ++immed_offset;
              break;
            case 'w': // w    word immediate operand
              sprintf(temp, "#0x%04x",
                 (uint)((get_mem(MEM_ROM_ID, addr+immed_offset)<<8) |
                        (get_mem(MEM_ROM_ID, addr+immed_offset+1))) );
              ++immed_offset;
              ++immed_offset;
              break;
            case 'b': // b    byte immediate operand
              sprintf(temp, "#0x%02x", (uint)get_mem(MEM_ROM_ID, addr+immed_offset));
              ++immed_offset;
              break;
            case 'x': // x    extended addressing
              sprintf(temp, "0x%04x",
                 (uint)((get_mem(MEM_ROM_ID, addr+immed_offset)<<8) |
                        (get_mem(MEM_ROM_ID, addr+immed_offset+1))) );
              ++immed_offset;
              ++immed_offset;
              break;
            case 'd': // d    direct addressing
              sprintf(temp, "*0x%02x", (uint)get_mem(MEM_ROM_ID, addr+immed_offset));
              ++immed_offset;
              break;
            case '3': // 3    24bit index offset
              sprintf(temp, "0x%06lx",
                 (ulong)((get_mem(MEM_ROM_ID, addr+immed_offset)<<16) |
                        (get_mem(MEM_ROM_ID, addr+immed_offset+1)<<8) |
                        (get_mem(MEM_ROM_ID, addr+immed_offset+2))) );
              ++immed_offset;
              ++immed_offset;
              ++immed_offset;
             break;
            case '2': // 2    word index offset
              sprintf(temp, "0x%04x",
                 (uint)((get_mem(MEM_ROM_ID, addr+immed_offset)<<8) |
                        (get_mem(MEM_ROM_ID, addr+immed_offset+1))) );
              ++immed_offset;
              ++immed_offset;
              break;
            case '1': // b    byte index offset
              sprintf(temp, "0x%02x", (uint)get_mem(MEM_ROM_ID, addr+immed_offset));
              ++immed_offset;
              break;
            case 'p': // b    byte index offset
              sprintf(temp, "0x%04x",
                 addr+immed_offset+1
                 +(char)get_mem(MEM_ROM_ID, addr+immed_offset));
              ++immed_offset;
              break;
            default:
              strcpy(temp, "?");
              break;
            }
          t= temp;
          while (*t)
            *(p++)= *(t++);
        }
      else
        *(p++)= *(b++);
    }
  *p= '\0';

  p= strchr(work, ' ');
  if (!p)
    {
      buf= strdup(work);
      return(buf);
    }
  if (sep == NULL)
    buf= (char *)malloc(6+strlen(p)+1);
  else
    buf= (char *)malloc((p-work)+strlen(sep)+strlen(p)+1);
  for (p= work, t= buf; *p != ' '; p++, t++)
    *t= *p;
  p++;
  *t= '\0';
  if (sep == NULL)
    {
      while (strlen(buf) < 6)
        strcat(buf, " ");
    }
  else
    strcat(buf, sep);
  strcat(buf, p);
  return(buf);
}


void
cl_stm8::print_regs(class cl_console_base *con)
{
  con->dd_printf("V-IHINZC  Flags= 0x%02x %3d %c  ",
                 regs.CC, regs.CC, isprint(regs.CC)?regs.CC:'.');
  con->dd_printf("A= 0x%02x %3d %c\n",
                 regs.A, regs.A, isprint(regs.A)?regs.A:'.');
  con->dd_printf("%c-%c%c%c%c%c%c  ",
                 (regs.CC&BIT_V)?'1':'0',
                 (regs.CC&BIT_I1)?'1':'0',
                 (regs.CC&BIT_H)?'1':'0',
                 (regs.CC&BIT_I0)?'1':'0',
                 (regs.CC&BIT_N)?'1':'0',
                 (regs.CC&BIT_Z)?'1':'0',
                 (regs.CC&BIT_C)?'1':'0');
  con->dd_printf("X= 0x%04x %3d %c    ",
                 regs.X, regs.X, isprint(regs.X)?regs.X:'.');
  con->dd_printf("Y= 0x%04x %3d %c\n",
                 regs.Y, regs.Y, isprint(regs.Y)?regs.Y:'.');
  con->dd_printf("SP= 0x%04x [SP+1]= %02x %3d %c\n",
                 regs.SP, ram->get(regs.SP+1), ram->get(regs.SP+1),
                 isprint(ram->get(regs.SP+1))?ram->get(regs.SP+1):'.');

  print_disass(PC, con);
}

/*
 * Execution
 */

int
cl_stm8::exec_inst(void)
{
  t_mem code;
  unsigned char cprefix; // prefix used for processing in functions

  if (regs.VECTOR) {
    PC = get1(0x8000);
	if (PC == 0x82) { // this is reserved opcode for vector table
		regs.VECTOR = 0;
		PC = get1(0x8001)*(1<<16);
		PC += get2(0x8002);
		return(resGO);
	} else {
		return( resERROR);
	}
  }

  if (fetch(&code)) {
	  printf("******************** break \n");
	  return(resBREAKPOINT);
  }
  tick(1);

  switch (code) { // get prefix
	case 0x72:
	case 0x90:
	case 0x91:
	case 0x92:
		cprefix = code;
		fetch(&code);
		break;
	default:
		cprefix = 0x00;
		break;
  }
	
  //printf("********************  switch; pc=0x%x, prefix = 0x%x, code = 0x%x\n",PC, cprefix, code);
  switch ((code >> 4) & 0xf) {
    case 0x0: 
		if ( 0x72 == cprefix) {
		  return(inst_btjfbtjt(code, cprefix));
		} 
		if ( 0x0f == code) { // clr ($xx,SP)
			unsigned char addr = get1( fetch1() + regs.SP);
			store1( addr, 0);
			return( resGO);
		}
		if ((code == 0x01) && ((cprefix == 0)||(cprefix ==0x90))) { //rrwa
			return(inst_rotw(code, cprefix));
		}
		if ((code == 0x02) && ((cprefix == 0)||(cprefix ==0x90))) { //rlwa
			return(inst_rotw(code, cprefix));
		}
		
		if ( 0x00 == code) { // neg ($xx,SP)
		}
		if ( 0x04 == code) { // srl ($xx,SP)
		}
		if ( 0x05 == code) { // rrc ($xx,SP)
		}
		if ( 0x07 == code) { // sra ($xx,SP)
		}
		if ( 0x08 == code) { // sll ($xx,SP)
		}
		if ( 0x09 == code) { // rlc ($xx,SP)
		}
		if ( 0x0d == code) { // tnz ($xx,SP)
		}
		if ( 0x0e == code) { // swap ($xx,SP)
		}
    case 0x1:
		if ((code == 0x1d) && (cprefix == 0x00)) {
			return( inst_subw(code, cprefix));
		}
		if ( 0x13 == code) {
			regs.X = get2( fetch1() + regs.SP);
			return( resGO);
		}
		if ( 0x90 == cprefix) {
		  return(inst_bccmbcpl(code, cprefix)); // copy C flag to memory
		} else if ( 0x72 == cprefix) {
		  return(inst_bresbset(code, cprefix));
		} 
		switch (code & 0xf) {
			case 0x0: return(inst_sub(code, cprefix)); // sub A,(shortoff,SP)
			case 0x1: return(inst_cp(code, cprefix)); // cp A,(shortoff,SP)
			case 0x2: return(inst_sbc(code, cprefix)); // sbc A,(shortoff,SP)
			case 0x3: return(inst_cpw(code, cprefix)); // cpw X,(shortoff,SP)
			case 0x8: return(inst_xor(code, cprefix));
			case 0x9: return(inst_adc(code, cprefix));
			case 0xa: return(inst_or(code, cprefix));
			case 0xb: return(inst_add(code, cprefix));
			case 0xc: regs.X = fetch2(); return( resGO); //addw X,#word

			default: return(resHALT);
		}
    case 0x2: 
		return(inst_jr(code, cprefix));
    case 0x3:
		if ((code == 0x31) && (cprefix == 0)) { // exg A,$xxxx
			unsigned int tmpi, addr;
			addr = fetch2();
			tmpi = get1(addr);
			store1(addr, regs.A);
			regs.A = tmpi;
			return( resGO);
		}

		if ((code == 0x3b) && (cprefix == 0x00)) {
			push1( get1(fetch2())); // push $xxxx
			return( resGO);
		}
		if ((code == 0x32) && (cprefix == 0x00)) {
			unsigned char tmpv;
			pop1(tmpv);
			store1(fetch2(),tmpv); // pop $xxxx
			return( resGO);
		}
		if ((code == 0x35) && (cprefix == 0x00)) { // mov
			unsigned int tmpaddr, tmpdat;
			tmpdat = fetch1();
			tmpaddr = fetch2();
			store1(tmpaddr, tmpdat);
			return( resGO);
		}
    case 0x4:
		if ((code == 0x4b) && (cprefix == 0x00)) {
			push1( fetch1()); // push #$xx
			return( resGO);
		}
		if ((code == 0x42) && (cprefix == 0x00)) {  // mul X,A
			regs.X = (regs.X & 0xff) * regs.A;
			FLAG_ASSIGN (BIT_C, 0); FLAG_ASSIGN (BIT_H, 0);
			return( resGO);
		}
		if ((code == 0x42) && (cprefix == 0x90)) {  // mul Y,A
			regs.Y = (regs.Y & 0xff) * regs.A;
			FLAG_ASSIGN (BIT_C, 0); FLAG_ASSIGN (BIT_H, 0);
			return( resGO);
		}
		if ((code == 0x45) && (cprefix == 0x00)) { // mov
			unsigned int tmpaddr, tmpdat;
			tmpaddr = fetch1();
			tmpdat = get1(fetch1());
			store1(tmpaddr, tmpdat);
			return( resGO);
		}
		if ((code == 0x41) && (cprefix == 0)) { // exg A,XL
			unsigned int tmpi;
			tmpi = regs.X;
			regs.X &= 0xff00;
			regs.X |= regs.A;
			regs.A = tmpi & 0xff;
			return( resGO);
		}

    case 0x5:
		if ((code == 0x55) && (cprefix == 0x00)) { // mov
			unsigned int tmpaddr, tmpdat;
			tmpaddr = fetch2();
			tmpdat = get1(fetch2());
			store1(tmpaddr, tmpdat);
			return( resGO);
		}
		if ((code == 0x5d) && ((cprefix == 0x00) || (cprefix == 0x90))) {
			return( inst_tnzw(code, cprefix));
		}
		if ((code == 0x5e) && ((cprefix == 0x00) || (cprefix == 0x90))) {
			return( inst_swapw(code, cprefix));
		}
		if (code == 0x5b) {
			regs.SP += fetch(); return( resGO);
		}
		if ((code == 0x5f) && (cprefix == 0)) {
			regs.X = 0; return( resGO);
		}
		if ((code == 0x52) && (cprefix == 0)) { // sub SP,#$cislo
			regs.SP -= fetch1(); return( resGO);
		}
		if ((code == 0x5f) && (cprefix ==0x90)) {
			regs.Y = 0; return( resGO);
		}
		if ((code == 0x53) && (cprefix == 0)) {
			regs.X ^= 0xffff;   FLAG_NZ (regs.X); FLAG_ASSIGN (BIT_C, 1);  FLAG_ASSIGN (BIT_N, 0x8000 &  regs.X); return( resGO);
		}
		if ((code == 0x53) && (cprefix ==0x90)) {
			regs.Y ^= 0xffff;   FLAG_NZ (regs.Y); FLAG_ASSIGN (BIT_C, 1);  FLAG_ASSIGN (BIT_N, 0x8000 &  regs.Y); return( resGO);
		}
		if ((code == 0x50) && ((cprefix == 0)||(cprefix ==0x90))) { //srlw
			return(inst_negw(code, cprefix));
		}
		if ((code == 0x54) && ((cprefix == 0)||(cprefix ==0x90))) { //srlw
			return(inst_shiftw(code, cprefix));
		}
		if ((code == 0x56) && ((cprefix == 0)||(cprefix ==0x90))) { //rrcw
			return(inst_rotw(code, cprefix));
		}
		if ((code == 0x59) && ((cprefix == 0)||(cprefix ==0x90))) { //rlcw
			return(inst_rotw(code, cprefix));
		}
		if ((code == 0x57) && ((cprefix == 0)||(cprefix ==0x90))) { //sraw
			return(inst_shiftw(code, cprefix));
		}
		if ((code == 0x58) && ((cprefix == 0)||(cprefix ==0x90))) { //sllw
			return(inst_shiftw(code, cprefix));
		}
		if ((code == 0x5a) && (cprefix == 0)) {
			regs.X -= 1;   FLAG_NZ (regs.X); FLAG_ASSIGN (BIT_N, 0x8000 &  regs.X);
			//ToDo FLAG_ASSIGN (BIT_V, 0x8000 & (operaxxnd1 ^ operand2 ^ result ^ (result >>1)));
			return( resGO);
		}
		if ((code == 0x5a) && (cprefix ==0x90)) {
			regs.Y -= 1;   FLAG_NZ (regs.Y); FLAG_ASSIGN (BIT_N, 0x8000 &  regs.Y);
			//ToDo FLAG_ASSIGN (BIT_V, 0x8000 & (operaxxnd1 ^ operand2 ^ result ^ (result >>1)));
			return( resGO);
		}
		if ((code == 0x5c) && (cprefix == 0)) {
			regs.X++;   FLAG_NZ (regs.X); FLAG_ASSIGN (BIT_N, 0x8000 &  regs.X);
			//ToDo FLAG_ASSIGN (BIT_V, 0x8000 & (operaxxnd1 ^ operand2 ^ result ^ (result >>1)));
			return( resGO);
		}
		if ((code == 0x5c) && (cprefix ==0x90)) {
			regs.Y++;   FLAG_NZ (regs.Y); FLAG_ASSIGN (BIT_N, 0x8000 &  regs.Y);
			//ToDo FLAG_ASSIGN (BIT_V, 0x8000 & (operaxxnd1 ^ operand2 ^ result ^ (result >>1)));
			return( resGO);
		}
		if ((code == 0x51) && (cprefix == 0)) { // exgw X,Y
			unsigned int tmpi;
			tmpi = regs.X;
			regs.X = regs.Y;
			regs.Y = tmpi;
			return( resGO);
		}
		switch (code & 0xf) {
			case 0xb: regs.SP += fetch(); return( resGO);
	  
			//default is processing in the next switch statement

		}
    case 0x6:
		if ((code == 0x61) && (cprefix == 0)) { // exg A,XL
			unsigned int tmpi;
			tmpi = regs.Y;
			regs.Y &= 0xff00;
			regs.Y |= regs.A;
			regs.A = tmpi & 0xff;
			return( resGO);
		}
		if ((code == 0x62) && (cprefix == 0)) { // div X,A
			unsigned int tmpi;
			
			FLAG_CLEAR(BIT_V);
			FLAG_CLEAR(BIT_H);
			FLAG_CLEAR(BIT_N);
			if (regs.A == 0) {
				//nastav C
				FLAG_SET(BIT_C);
			} else {
				FLAG_CLEAR(BIT_C);
				tmpi = regs.X%regs.A;
				regs.X = regs.X/regs.A; 
				regs.A = tmpi;
			}
			return( resGO);
		}
		if ((code == 0x62) && (cprefix ==0x90)) { // div Y,A
			unsigned int tmpi;
			
			FLAG_CLEAR(BIT_V);
			FLAG_CLEAR(BIT_H);
			FLAG_CLEAR(BIT_N);
			if (regs.A == 0) {
				//nastav C
				FLAG_SET(BIT_C);
			} else {
				FLAG_CLEAR(BIT_C);
				tmpi = regs.Y%regs.A;
				regs.Y = regs.Y/regs.A; 
				regs.A = tmpi;
			}
			return( resGO);
		}
		if ((code == 0x65) && (cprefix == 0)) { // divw X,Y
			unsigned int tmpi;

			FLAG_CLEAR(BIT_V);
			FLAG_CLEAR(BIT_H);
			FLAG_CLEAR(BIT_N);
			if (regs.Y == 0) {
				//nastav C
				FLAG_SET(BIT_C);
			} else {
				FLAG_CLEAR(BIT_C);
				
				tmpi = regs.X%regs.Y; 
				regs.X = regs.X/regs.Y;
				regs.Y = tmpi;
			}
			return( resGO);
		}
	
    case 0x7:
		switch (code & 0xf) {
			case 0x00: return(inst_neg(code, cprefix));
			case 0x03: return(inst_cpl(code, cprefix));
			case 0x04: return(inst_srl(code, cprefix));
			case 0x05: return(inst_rrc(code, cprefix));
			case 0x07: return(inst_sra(code, cprefix));
			case 0x08: return(inst_sll(code, cprefix));
			case 0x09: return(inst_rlc(code, cprefix));
			case 0x0b: regs.SP += fetch(); return( resGO);
			case 0x0c: return(inst_inc(code, cprefix));
			case 0x0d: return( inst_tnz(code, cprefix));
			case 0x0e: return( inst_swap(code, cprefix));
			case 0x0f: return(inst_clr(code, cprefix));
		}
		
    case 0x8:
		if (code == 0x8d) {
			int ptr = fetch2();
			push2(PC);
			push1(PC>>16);

			if (cprefix == 0x92) {
				PC = get3(ptr);
			} else {
				PC = (ptr << 8) + fetch();
			}
			return( resGO);
		}
		if (code == 0x8c) { // CCF
			regs.CC ^= BIT_C;
			return( resGO);
		}

		if (code == 0x81) { // RET
			pop2( PC);
			return( resGO);
		}
		if (code == 0x80) { // IRET
			pop1( regs.CC);
			pop1( regs.A);
			pop2( regs.X);
			pop2( regs.Y);
			pop1( PC); // extended PC is skipped
			pop2( PC);
			return( resGO);
		}
		if (code == 0x87) { // RETF
			pop1( PC); // extended PC is skipped
			pop2( PC);
			return( resGO);
		}
		if (((code == 0x8f)||(code == 0x8e)) && (cprefix == 0x00)) {
			PC -= 1; // WFI = Wait For Interrupt, HALT
			FLAG_SET(BIT_I1);
			FLAG_CLEAR(BIT_I0);
			return( resGO);
		}
		if ((code == 0x8f) && (cprefix == 0x72)) {
			PC -= 2; // WFE = Wait For Event
			return( resGO);
		}
		if ((code == 0x83) && (cprefix == 0x00)) { // TRAP
			return(inst_trap(code, cprefix));
		}
		if ((code == 0x88) && (cprefix == 0x00)) {
			push1( regs.A); // push A
			return( resGO);
		}
		if ((code == 0x8A) && (cprefix == 0x00)) {
			push1( regs.CC); // push CC
			return( resGO);
		}
		if ((code == 0x89) && (cprefix == 0x00)) {
			push2( regs.X); // pushw X
			return( resGO);
		}
		if ((code == 0x89) && (cprefix == 0x90)) {
			push2( regs.Y); // pushw Y
			return( resGO);
		}
		if ((code == 0x85) && (cprefix == 0x00)) {
			pop2( regs.X); // popw X
			return( resGO);
		}
		if ((code == 0x85) && (cprefix == 0x90)) {
			pop2( regs.Y); // popw Y
			return( resGO);
		}
 		if ((code == 0x84) && (cprefix == 0x00)) {
			pop1( regs.A); // pop A
			return( resGO);
		}
		if ((code == 0x86) && (cprefix == 0x00)) {
			pop1( regs.CC); // pop CC
			return( resGO);
		}
   case 0x9:
		if ((code == 0x99) && (cprefix == 0x00)) { // SCF
			FLAG_SET(BIT_C);
			return( resGO);
		}
		if ((code == 0x98) && (cprefix == 0x00)) { // RCF
			FLAG_CLEAR(BIT_C);
			return( resGO);
		}
		if ((code == 0x9a) && (cprefix == 0x00)) { // RIM - enable IRQ
			FLAG_SET(BIT_I1);
			FLAG_CLEAR(BIT_I0);
			return( resGO);
		}
		if ((code == 0x9b) && (cprefix == 0x00)) { // SIM - disable IRQ
			FLAG_SET(BIT_I1);
			FLAG_SET(BIT_I0);
			return( resGO);
		}
		if ((code == 0x9c) && (cprefix == 0x00)) { // RVF
			FLAG_CLEAR(BIT_V);
			return( resGO);
		}
		if ((code == 0x9d) && (cprefix == 0x00)) { // nop
			return( resGO);
		}
		if ((code == 0x97) && (cprefix == 0x00)) { // ld XL,A
			regs.X &= 0xff00;
			regs.X |= regs.A;
			FLAG_NZ (regs.A);
			return( resGO);
		}
		if ((code == 0x97) && (cprefix == 0x90)) { // ld YL,A
			regs.Y &= 0xff00;
			regs.Y |= regs.A;
			FLAG_NZ (regs.A);
			return( resGO);
		}
		if ((code == 0x9f) && (cprefix == 0x00)) { // ld A,XL
			regs.A = regs.X & 0xff;
			FLAG_NZ (regs.A);
			return( resGO);
		}
		if ((code == 0x9f) && (cprefix == 0x90)) { // ld A,YL
			regs.A = regs.Y & 0xff;
			FLAG_NZ (regs.A);
			return( resGO);
		}
		if ((code == 0x95) && (cprefix == 0x00)) { // ld XH,A
			regs.X &= 0x00ff;
			regs.X |= regs.A<<8;
			FLAG_NZ (regs.A);
			return( resGO);
		}
		if ((code == 0x95) && (cprefix == 0x90)) { // ld YH,A
			regs.Y &= 0xff;
			regs.Y |= regs.A<<8;
			FLAG_NZ (regs.A);
			return( resGO);
		}
		if ((code == 0x9E) && (cprefix == 0x00)) { // ld A,XH
			regs.A = regs.X >> 8;
			FLAG_NZ (regs.A);
			return( resGO);
		}
		if ((code == 0x9E) && (cprefix == 0x90)) { // ld A,YH
			regs.A = regs.Y >> 8;
			FLAG_NZ (regs.A);
			return( resGO);
		}
		

    case 0xa:
		if ((code == 0xa7) || (code == 0xaf)) { // ldf
			return( resHALT); // LDF unsupported now !!!! ToDo
		}
		if (code == 0xac)  { // jpf
			return( resHALT); // JPF unsupported now !!!! ToDo
		}
	
    case 0xb:
		if ((code == 0xbc) || (code == 0xbd)) { // ldf
			return( resHALT); // LDF unsupported now !!!! ToDo
		}
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
	  if (code == 0xad) {
		return(inst_callr(code, cprefix));
	  }
	  if (cprefix == 0x72) {
		if ((code == 0xf0) || (code == 0xf2) || (code == 0xa2)
			|| (code == 0xb0) || (code == 0xb2)) {
			return(inst_subw(code, cprefix));
		}
	    switch (code) { //addw
			case 0xa9 : regs.Y = fetch2(); return( resGO);
			case 0xb9 : regs.Y = get2(fetch2()); return( resGO);
			case 0xbb : regs.X = get2(fetch2()); return( resGO);
			case 0xf9 : regs.Y = get2(fetch() + regs.SP); return( resGO);
			case 0xfb : regs.X = get2(fetch() + regs.SP); return( resGO);
			//default is processing in the next switch statement
		}
	  }
      switch (code & 0xf) {
		case 0x0: return(inst_sub(code, cprefix));
		case 0x1: return(inst_cp(code, cprefix));
		case 0x2: return(inst_sbc(code, cprefix));
		case 0x3: return(inst_cpw(code, cprefix));
		case 0x4: return(inst_and(code, cprefix));
		case 0x5: return(inst_bcp(code, cprefix));
 		case 0x6: return(inst_lda(code, cprefix));
 		case 0x7: return(inst_lddst(code, cprefix));
        case 0x8: return(inst_xor(code, cprefix));
        case 0x9: return(inst_adc(code, cprefix));
		case 0xa: return(inst_or(code, cprefix));
        case 0xb: return(inst_add(code, cprefix));
        case 0xc: return(inst_jp(code, cprefix));
		case 0xd: return(inst_call(code, cprefix));
		case 0xe: return(inst_ldw(code, cprefix));
	  
        default: return(resHALT);
	  }
	  
    default:
		printf("************* bad code !!!!\n");
		return(resHALT);
  }

  /*if (PC)
    PC--;
  else
  PC= get_mem_size(MEM_ROM_ID)-1;*/
  PC= rom->inc_address(PC, -1);

  sim->stop(resINV_INST);
  return(resINV_INST);
}


/* End of stm8.src/stm8.cc */
