/*
 * Simulator of microcontrollers (inst.cc)
 *
 * stm8 code base from Vaclav Peroutka vaclavpe@users.sourceforge.net
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
#include "stdio.h"
#include <stdlib.h>

// local
#include "stm8cl.h"
#include "regsstm8.h"
#include "stm8mac.h"



int
cl_stm8::fetchea(t_mem code, unsigned char prefix)
{
  switch ((code >> 4) & 0x0f) {
    case 0x1: return (unsigned char)fetch()+regs.SP;  // SP index
	
    case 0xb: return fetch(); // Direct
	
    case 0xc:
	  if ( 0 == prefix) {
		return fetch2();
	  } else if ( 0x72 == prefix) {
	    return get2(fetch2());
	  } else if ( 0x92 == prefix) {
	    return get2(fetch());
	  } else {
	    return( resHALT);
	  }

    case 0xd:
	  if ( 0 == prefix) {
        return fetch2()+regs.X; // X reg
	  } else if ( 0x72 == prefix) {
	    return get2(fetch2()+regs.X);
	  } else if ( 0x90 == prefix) {
        return fetch2()+regs.Y; // Y reg
	  } else if ( 0x91 == prefix) {
	    return get2((unsigned char)fetch()+regs.Y);
	  } else if ( 0x92 == prefix) {
	    return get2((unsigned char)fetch()+regs.X);
	  } else {
	    return( resHALT);
	  }

    case 0xe:
	  if ( 0 == prefix) {
		return (unsigned char)fetch()+regs.X;  // X index
	  } else if ( 0x90 == prefix) {
	    return (unsigned char)fetch()+regs.Y;  // Y index
	  } else {
	    return( resHALT);
	  }

    case 0xf:
	  if ( 0 == prefix) {
		return regs.X;  // X index
	  } else if ( 0x90 == prefix) {
	    return regs.Y;  // Y index
	  } else {
	    return( resHALT);
	  }

    default:
      return(resHALT);
  }
}

int
cl_stm8::inst_adc(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;
  int carryin = (regs.CC & BIT_C)!=0;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 + operand2 + carryin;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));

  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_add(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 + operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));
  
  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_and(t_mem code, unsigned char prefix)
{
  regs.A = regs.A & OPERAND(code, prefix);
  FLAG_CLEAR(BIT_V);
  FLAG_NZ(regs.A);
  return(resGO);
}

int
cl_stm8::inst_bccmbcpl(t_mem code, unsigned char prefix)
{
  int ea = fetch2();
  unsigned char dbyte = get1( ea);
  
  if (code & 0x01)  { // bccm  
	char pos = (code - 0x11) >> 1;
	dbyte = dbyte & (~(1<<pos));
	if (regs.CC & BIT_C) {
		dbyte |= (1<<pos);
	}
	
  } else { // bcpl
 	char pos = (code - 0x10) >> 1;
	dbyte = dbyte ^ (1<<pos);
	
  }
  
  store1(ea, dbyte);
  return(resGO);
}

int
cl_stm8::inst_bcp(t_mem code, unsigned char prefix)
{
  uchar operand = regs.A & OPERAND(code, prefix);
  FLAG_CLEAR(BIT_V);
  FLAG_NZ(operand);
  return(resGO);
}

int
cl_stm8::inst_bresbset(t_mem code, unsigned char prefix)
{
  int ea = fetch2();
  unsigned char dbyte = get1( ea);
  
  if (code & 0x01) { // bres  
	char pos = (code - 0x11) >> 1;
	dbyte = dbyte & (~(1<<pos));
	
  } else { // bset
 	char pos = (code - 0x10) >> 1;
	dbyte = dbyte | (1<<pos);
	
  }
  
  store1(ea, dbyte);
  return(resGO);
}

int
cl_stm8::inst_btjfbtjt(t_mem code, unsigned char prefix)
{
  int ea = fetch2();
  unsigned char dbyte = get1( ea);
  char reljump = fetch();
  
  if (code & 0x01) { // btjf  
	char pos = (code - 0x01) >> 1;
	if(!( dbyte & (1<<pos))) {
		PC += reljump;
	}
	
  } else { // btjt
 	char pos = (code - 0x00) >> 1;
	if ( dbyte & (1<<pos)) {
		PC += reljump;
	}
	
  }
  
  return(resGO);
}

int
cl_stm8::inst_call(t_mem code, unsigned char prefix)
{
  int newPC = fetchea(code, prefix);
  push2(PC);
  PC = newPC;

  return(resGO);
}

int
cl_stm8::inst_callr(t_mem code, unsigned char prefix)
{
  char newPC = fetch1();
  push2(PC);
  PC += newPC;

  return(resGO);
}

int
cl_stm8::inst_clr(t_mem code, unsigned char prefix)
{
  int ea = 0xffff;
  uchar operand;
  
  operand = 0;
  FLAG_CLEAR (BIT_V);
  FLAG_CLEAR (BIT_N);
  FLAG_SET (BIT_Z);

  if ((code == 0x4f) && (prefix == 0x00))
    regs.A = operand;
  else {
    ea = fetchea(code,prefix);
    store1(ea, operand);
  }
  return(resGO);
}

int
cl_stm8::inst_cp(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 - operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);

  return(resGO);
}

int
cl_stm8::inst_cpw(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;
  
  switch (prefix) {
	case 0x00:
	
	case 0x72:
	case 0x90:
	case 0x91:
	case 0x92:
	default: break;
  }

  //if () {
	//operand1 = regs.X;
  //} else {
    //operand1 = regs.Y;
  //}
  operand2 = OPERAND(code, prefix);
  result = operand1 - operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);

  return(resGO);
}

int
cl_stm8::inst_cpl(t_mem code, unsigned char prefix)
{
  int result, dstbyte;
  int ea = 0xffff;

  dstbyte = OPERAND(code, prefix);
  result = dstbyte ^ 0xff;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_C, 1);
  FLAG_ASSIGN (BIT_N, 0x80 &  result);
  
  if ((code == 0x43) && (prefix == 0x00))
    regs.A = result;
  else {
    ea = fetchea(code,prefix);
    store1(ea, result);
  }
  return(resGO);
}

int
cl_stm8::inst_dec(t_mem code, unsigned char prefix)
{
  int result, dstbyte;
  int ea = 0xffff;

  dstbyte = OPERAND(code, prefix);
  result = dstbyte - 1;
  FLAG_NZ (result);
  //FLAG_ASSIGN (BIT_C, 1);
  FLAG_ASSIGN (BIT_N, 0x80 &  result);
  FLAG_ASSIGN (BIT_V, 0x80 & (dstbyte ^ result ^ (result >>1)));
  
  if ((code == 0x43) && (prefix == 0x00))
    regs.A = result;
  else {
    ea = fetchea(code,prefix);
    store1(ea, result);
  }
  return(resGO);
}

/////// nove
int
cl_stm8::inst_xor(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 ^ operand2;
  FLAG_NZ (result);
  
  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_trap(t_mem code, unsigned char prefix)
{
	// store to stack
	push2( PC);
	push1( 0x00); //extended PC
	push2( regs.Y);
	push2( regs.X);
	push1( regs.A);
	push1( regs.CC);
	// set I1 and I0 status bits
	FLAG_SET(BIT_I1);
	FLAG_SET(BIT_I0);
	// get TRAP address
	PC = get1(0x8004);
	if (PC == 0x82) { // this is reserved opcode for vector table
		regs.VECTOR = 0;
		PC = get1(0x8005)*(1<<16);
		PC += get2(0x8006);
		return(resGO);
	} else {
		return( resERROR);
	}
}


int
cl_stm8::inst_tnzw(t_mem code, unsigned char prefix)
{
  unsigned int operand;
  
  if ((code == 0x5d) and (prefix == 0x00))
    operand = regs.X;
  else if ((code == 0x5d) and (prefix == 0x90))
    operand = regs.Y;
  else {
    return( resERROR);
  }

  FLAG_ASSIGN (BIT_N, 0x8000 & operand);
  FLAG_ASSIGN (BIT_Z, operand ^ 0xffff);

  return(resGO);
}

int
cl_stm8::inst_tnz(t_mem code, unsigned char prefix)
{
  int operand;

  operand = OPERAND(code, prefix);
  FLAG_NZ (operand);

  return(resGO);
}

int
cl_stm8::inst_swapw(t_mem code, unsigned char prefix)
{
  unsigned int operand;
  
  if ((code == 0x5e) and (prefix == 0x00)) {
    operand = regs.X;
	regs.X <<= 8;
	regs.X |= (operand >> 8);
    operand = regs.X;
  } else if ((code == 0x5e) and (prefix == 0x90)) {
    operand = regs.Y;
	regs.Y <<= 8;
	regs.Y |= (operand >> 8);
    operand = regs.Y;

  } else {
    return( resERROR);
  }

  FLAG_ASSIGN (BIT_N, 0x8000 & operand);
  FLAG_ASSIGN (BIT_Z, operand ^ 0xffff);

  return(resGO);
}

int
cl_stm8::inst_swap(t_mem code, unsigned char prefix)
{
  int operand, swp;

  operand = OPERAND(code, prefix);
  swp = operand >> 8;
  operand <<=8;
  operand |= swp;
  
  FLAG_NZ (operand);
  store1(fetchea(code,prefix), operand);
  
  return(resGO);
}

int
cl_stm8::inst_subw(t_mem code, unsigned char prefix)
{
  int operand;

  if ((code == 0x1d) and (prefix == 0x00)) {
	regs.X = regs.X - fetch2();
	operand = regs.X;
  } else if ((code == 0xa2) and (prefix == 0x72)) {
	regs.Y = regs.Y - fetch2();
	operand = regs.Y;
  } else if ((code == 0xb0) and (prefix == 0x72)) {
	regs.X = regs.X - get2(fetch2());
	operand = regs.X;
  } else if ((code == 0xf0) and (prefix == 0x72)) {
	regs.X = regs.X - get2(regs.SP+fetch1());
	operand = regs.X;
  } else if ((code == 0xb2) and (prefix == 0x72)) {
	regs.Y = regs.Y - get2(fetch2());
	operand = regs.Y;
  } else if ((code == 0xf2) and (prefix == 0x72)) {
	regs.Y = regs.Y - get2(regs.SP+fetch1());
	operand = regs.Y;
  } else {
	return(resERROR);
  }


  
  FLAG_ASSIGN (BIT_N, 0x8000 & operand);
  FLAG_ASSIGN (BIT_Z, operand ^ 0xffff);
  
  return(resGO);
}

int
cl_stm8::inst_sub(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 - operand2;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));
  
  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_shiftw(t_mem code, unsigned char prefix)
{
	if ((code == 0x54) and (prefix == 0x00)) { // srlw X
		FLAG_ASSIGN (BIT_C, 0x1 & regs.X);
		regs.X >>= 1;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
		FLAG_ASSIGN (BIT_Z, regs.X ^ 0xffff);
	} else if ((code == 0x54) and (prefix == 0x90)) { // srlw Y
		FLAG_ASSIGN (BIT_C, 0x1 & regs.Y);
		regs.Y >>= 1;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
		FLAG_ASSIGN (BIT_Z, regs.Y ^ 0xffff);
	} else if ((code == 0x57) and (prefix == 0x00)) { // sraw X
		FLAG_ASSIGN (BIT_C, 0x1 & regs.X);
		regs.X >>= 1;
		if (regs.X & 0x4000) regs.X |= 0x8000;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
		FLAG_ASSIGN (BIT_Z, regs.X ^ 0xffff);
	} else if ((code == 0x57) and (prefix == 0x90)) { // sraw Y
		FLAG_ASSIGN (BIT_C, 0x1 & regs.Y);
		regs.Y >>= 1;
		if (regs.Y & 0x4000) regs.Y |= 0x8000;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
		FLAG_ASSIGN (BIT_Z, regs.Y ^ 0xffff);
	} else if ((code == 0x58) and (prefix == 0x00)) { // sllw X
		FLAG_ASSIGN (BIT_C, 0x8000 & regs.X);
		regs.X <<= 1;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
		FLAG_ASSIGN (BIT_Z, regs.X ^ 0xffff);
	} else if ((code == 0x58) and (prefix == 0x90)) { // sllw Y
		FLAG_ASSIGN (BIT_C, 0x8000 & regs.Y);
		regs.Y <<= 1;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
		FLAG_ASSIGN (BIT_Z, regs.Y ^ 0xffff);
	} else {
		return( resERROR);
	}	

  return(resGO);
}

int
cl_stm8::inst_srl(t_mem code, unsigned char prefix)
{
  int operand;

  operand = OPERAND(code, prefix);
  FLAG_ASSIGN (BIT_C, 0x01 & operand);
  
  operand >>= 1;
  
  FLAG_NZ (operand);
  store1(fetchea(code,prefix), operand);
  
  return(resGO);
}

int
cl_stm8::inst_sra(t_mem code, unsigned char prefix)
{
  int operand;

  operand = OPERAND(code, prefix);
  FLAG_ASSIGN (BIT_C, 0x01 & operand);
  
  operand >>= 1;
  if (operand & 0x40) operand |= 0x80;
		
  FLAG_NZ (operand);
  store1(fetchea(code,prefix), operand);
  
  return(resGO);
}

int
cl_stm8::inst_sll(t_mem code, unsigned char prefix)
{
  int operand;

  operand = OPERAND(code, prefix);
  FLAG_ASSIGN (BIT_C, 0x80 & operand);
  
  operand <<= 1;
  
  FLAG_NZ (operand);
  store1(fetchea(code,prefix), operand);
  
  return(resGO);
}

int
cl_stm8::inst_sbc(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 - operand2;
  if (regs.CC & BIT_C) result--;
  
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
  FLAG_ASSIGN (BIT_C, 0x100 & result);
  FLAG_ASSIGN (BIT_H, 0x10 & (operand1 ^ operand2 ^ result));
  
  regs.A = result & 0xff;
  return(resGO);
}

int
cl_stm8::inst_rotw(t_mem code, unsigned char prefix)
{
	unsigned int tmp;

	if ((code == 0x01) and (prefix == 0x00)) { // rrwa X,A
		tmp = regs.X;
		regs.X >>= 8;
		regs.X |= (regs.A << 8);
		regs.A = tmp & 0xff;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
		FLAG_ASSIGN (BIT_Z, regs.X ^ 0xffff);
	} else if ((code == 0x01) and (prefix == 0x90)) { // rrwa Y,A
		tmp = regs.Y;
		regs.Y >>= 8;
		regs.Y |= (regs.A << 8);
		regs.A = tmp & 0xff;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
		FLAG_ASSIGN (BIT_Z, regs.Y ^ 0xffff);

	} else if ((code == 0x02) and (prefix == 0x00)) { // rlwa X,A
		tmp = regs.X;
		regs.X <<= 8;
		regs.X |= regs.A ;
		regs.A = tmp >> 8;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
		FLAG_ASSIGN (BIT_Z, regs.X ^ 0xffff);
	} else if ((code == 0x02) and (prefix == 0x90)) { // rlwa Y,A
		tmp = regs.Y;
		regs.Y <<= 8;
		regs.Y |= regs.A ;
		regs.A = tmp >> 8;
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
		FLAG_ASSIGN (BIT_Z, regs.Y ^ 0xffff);

	} else if ((code == 0x56) and (prefix == 0x00)) { // rrcw X
		tmp = regs.X;
		regs.X >>= 1;
		if ( regs.CC & BIT_C) regs.X |= 0x8000;
		FLAG_ASSIGN (BIT_C, 0x01 & tmp);
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
		FLAG_ASSIGN (BIT_Z, regs.X ^ 0xffff);
	} else if ((code == 0x56) and (prefix == 0x90)) { // rrcw Y
		tmp = regs.Y;
		regs.Y >>= 1;
		if ( regs.CC & BIT_C) regs.Y |= 0x8000;
		FLAG_ASSIGN (BIT_C, 0x01 & tmp);
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
		FLAG_ASSIGN (BIT_Z, regs.Y ^ 0xffff);
	} else if ((code == 0x56) and (prefix == 0x00)) { // rlcw X
		tmp = regs.X;
		regs.X <<= 1;
		if ( regs.CC & BIT_C) regs.X |= 0x0001;
		FLAG_ASSIGN (BIT_C, 0x8000 & tmp);
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
		FLAG_ASSIGN (BIT_Z, regs.X ^ 0xffff);
	} else if ((code == 0x56) and (prefix == 0x90)) { // rlcw Y
		tmp = regs.Y;
		regs.Y <<= 1;
		if ( regs.CC & BIT_C) regs.Y |= 0x0001;
		FLAG_ASSIGN (BIT_C, 0x8000 & tmp);
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
		FLAG_ASSIGN (BIT_Z, regs.Y ^ 0xffff);

	} else {
		return( resERROR);
	}	

  return(resGO);
}

int
cl_stm8::inst_rrc(t_mem code, unsigned char prefix)
{
  int operand, tmp;

  operand = OPERAND(code, prefix);

  tmp = operand;
  operand >>= 1;
  if ( regs.CC & BIT_C) operand |= 0x80;
  
  FLAG_ASSIGN (BIT_C, 0x01 & tmp);
  FLAG_NZ (operand);
  store1(fetchea(code,prefix), operand);
  
  return(resGO);
}

int
cl_stm8::inst_rlc(t_mem code, unsigned char prefix)
{
  int operand, tmp;

  operand = OPERAND(code, prefix);
  FLAG_ASSIGN (BIT_C, 0x80 & operand);
  tmp = operand;
  operand <<= 1;
  if ( regs.CC & BIT_C) operand |= 0x01;
  
  FLAG_ASSIGN (BIT_C, 0x80 & tmp);
  FLAG_NZ (operand);
  store1(fetchea(code,prefix), operand);
  
  return(resGO);
}

int
cl_stm8::inst_or(t_mem code, unsigned char prefix)
{
  int result, operand1, operand2;

  operand1 = regs.A;
  operand2 = OPERAND(code, prefix);
  result = operand1 | operand2;
  FLAG_NZ (result);
  
  regs.A = result & 0xff;
  return(resGO);
}


int
cl_stm8::inst_negw(t_mem code, unsigned char prefix)
{
	if ((code == 0x50) and (prefix == 0x00)) { // srlw X
		FLAG_ASSIGN (BIT_C, regs.X);
		if (regs.X == 0x8000) {
			FLAG_SET (BIT_V);
		} else {
			FLAG_CLEAR (BIT_V);
		}

		regs.X ^= 0xffff;
		regs.X += 1;
		
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
		FLAG_ASSIGN (BIT_Z, regs.X ^ 0xffff);
	} else if ((code == 0x50) and (prefix == 0x90)) { // srlw Y
		FLAG_ASSIGN (BIT_C, regs.Y);
		if (regs.Y == 0x8000) {
			FLAG_SET (BIT_V);
		} else {
			FLAG_CLEAR (BIT_V);
		}

		regs.Y ^= 0xffff;
		regs.Y += 1;
		
		FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
		FLAG_ASSIGN (BIT_Z, regs.Y ^ 0xffff);

	} else {
		return( resERROR);
	}	

  return(resGO);
}

int
cl_stm8::inst_neg(t_mem code, unsigned char prefix)
{
  int operand, tmp;

  operand = OPERAND(code, prefix);
  FLAG_ASSIGN (BIT_C, operand);
  if (operand == 0x80) {
	FLAG_SET (BIT_V);
  } else {
	FLAG_CLEAR (BIT_V);
  }

  tmp = operand;
  operand ^= 0xff;
  operand++;
  
  FLAG_NZ (operand);
  store1(fetchea(code,prefix), operand);
  
  return(resGO);
}

int
cl_stm8::inst_ldw(t_mem code, unsigned char prefix)
{
	if ((code == 0xae) && (prefix == 0x00)) {
		regs.X = get2(PC);
		PC += 2;
		return( resGO);
	} else 	if ((code == 0xae) && (prefix == 0x90)) {
		regs.Y = get2(PC);
		PC += 2;
		return( resGO);
	}

  
  return(resHALT);
}

int
cl_stm8::inst_lddst(t_mem code, unsigned char prefix)
{

  return(resHALT);
}

int
cl_stm8::inst_lda(t_mem code, unsigned char prefix)
{

  return(resHALT);
}

int
cl_stm8::inst_jr(t_mem code, unsigned char prefix)
{
  bool taken;
  signed char ofs;
  unsigned char maskedP;
  
  if (prefix ==0x00) {
    switch ((code>>1) & 7) {
      case 0: // JRA/JRT
        taken = 1;
        break;
      case 1: // JRUGT
        taken = !(regs.CC & (BIT_C | BIT_Z));
        break;
      case 2: // JRUGE
        taken = !(regs.CC & BIT_C);
        break;
      case 3: // JRNE
        taken = !(regs.CC & BIT_Z);
        break;
      case 4: // JRNV
        taken = !(regs.CC & BIT_V);
        break;
      case 5: // JRPL
        taken = !(regs.CC & BIT_N);
        break;
      case 6: // JRSGT - Z or (N xor V) = 0
        maskedP = regs.CC & (BIT_N | BIT_V | BIT_Z);
        taken = !maskedP || (maskedP == (BIT_N | BIT_V));
        break;
      case 7: // JRSGE - N xor V = 0
        maskedP = regs.CC & (BIT_N | BIT_V);
        taken = !maskedP || (maskedP == (BIT_N | BIT_V));
      default:
        return(resHALT);
    } 
  }
  else if (prefix==0x90) {
    switch ((code>>1) & 7) {
      case 4: // JRNH
         taken = !(regs.CC & BIT_H);
       break;
      case 6: // JRNM - no int mask
        taken = !(regs.CC & (BIT_I1|BIT_I0));
        break;
      case 7: // JRIL - interrupt low - no means to test this ???
        taken = 0; 
      default:
        return(resHALT);
    }
  }
  else
    return(resHALT);
  
  if (code & 1)
    taken = ! taken;
  
  ofs = fetch();
  if (taken)
    PC += ofs;

  return(resGO);
}

int
cl_stm8::inst_jp(t_mem code, unsigned char prefix)
{
  int newPC = fetchea(code, prefix);

  PC = newPC;

  return(resGO);
}

int
cl_stm8::inst_inc(t_mem code, unsigned char prefix)
{
  int result, dstbyte;
  int ea = 0xffff;

  dstbyte = OPERAND(code, prefix);
  result = dstbyte + 1;
  FLAG_NZ (result);
  FLAG_ASSIGN (BIT_N, 0x80 &  result);
  FLAG_ASSIGN (BIT_V, 0x80 & (dstbyte ^ result ^ (result >>1)));
  
  if ((code == 0x4c) && (prefix == 0x00))
    regs.A = result;
  else {
    ea = fetchea(code,prefix);
    store1(ea, result);
  }
  return(resGO);
}

//int
//cl_stm8::inst_nop(t_mem code, bool prefix)
//{
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_transfer(t_mem code, bool prefix)
//{
//  int hx;
//  
//  switch (code) {
//    case 0x84: // TAP
//      regs.P = regs.A | 0x60;
//      break;
//    case 0x85: // TPA
//      regs.A = regs.P | 0x60;
//      break;
//    case 0x97: // TAX
//      regs.X = regs.A;
//      break;
//    case 0x9f: // TXA
//      regs.A = regs.X;
//      break;
//    case 0x94: // TXS
//      hx = (regs.H << 8) | regs.X;
//      regs.SP = (hx - 1) & 0xffff;
//      break;
//    case 0x95: // TSX
//      hx = regs.SP +1;
//      regs.H = (hx >> 8) & 0xff;
//      regs.X = hx & 0xff;
//      break;
//    default:
//      return(resHALT);
//  }
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_setclearflags(t_mem code, bool prefix)
//{
//  switch (code) {
//    case 0x98:
//      regs.P &= ~BIT_C;
//      break;
//    case 0x99:
//      regs.P |= BIT_C;
//      break;
//    case 0x9a:
//      regs.P &= ~BIT_I;
//      break;
//    case 0x9b:
//      regs.P |= BIT_I;
//      break;
//    default:
//      return(resHALT);
//  }
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_rsp(t_mem code, bool prefix)
//{
//  regs.SP = 0x00ff;
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_nsa(t_mem code, bool prefix)
//{
//  regs.A = ((regs.A & 0xf0)>>4) | ((regs.A & 0x0f)<<4);
//  return(resGO);
//}
//
//
//
//int
//cl_stm8::inst_lda(t_mem code, bool prefix)
//{
//  regs.A = OPERAND(code, prefix);
//  FLAG_CLEAR(BIT_V);
//  FLAG_NZ(regs.A);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_ldx(t_mem code, bool prefix)
//{
//  regs.X = OPERAND(code, prefix);
//  FLAG_CLEAR(BIT_V);
//  FLAG_NZ(regs.X);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_sta(t_mem code, bool prefix)
//{
//  int ea = fetchea(code, prefix);
//
//  //fprintf (stdout, "ea = 0x%04x\n", ea);
//    
//  FLAG_CLEAR(BIT_V);
//  FLAG_NZ(regs.A);
//  store1(ea, regs.A);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_stx(t_mem code, bool prefix)
//{
//  int ea = fetchea(code, prefix);
//
//  FLAG_CLEAR(BIT_V);
//  FLAG_NZ(regs.X);
//  store1(ea, regs.X);
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_sub(t_mem code, bool prefix)
//{
//  int result, operand1, operand2;
//
//  operand1 = regs.A;
//  operand2 = OPERAND(code, prefix);
//  result = operand1 - operand2;
//  FLAG_NZ (result);
//  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
//  FLAG_ASSIGN (BIT_C, 0x100 & result);
//
//  regs.A = result & 0xff;
//  return(resGO);
//}
//
//int
//cl_stm8::inst_sbc(t_mem code, bool prefix)
//{
//  int result, operand1, operand2;
//  int carryin = (regs.P & BIT_C)!=0;
//
//  operand1 = regs.A;
//  operand2 = OPERAND(code, prefix);
//  result = operand1 - operand2 - carryin;
//  FLAG_NZ (result);
//  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
//  FLAG_ASSIGN (BIT_C, 0x100 & result);
//
//  regs.A = result & 0xff;
//  return(resGO);
//}
//
//int
//cl_stm8::inst_cmp(t_mem code, bool prefix)
//{
//  int result, operand1, operand2;
//
//  operand1 = regs.A;
//  operand2 = OPERAND(code, prefix);
//  result = operand1 - operand2;
//  FLAG_NZ (result);
//  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
//  FLAG_ASSIGN (BIT_C, 0x100 & result);
//
//  return(resGO);
//}
//
//int
//cl_stm8::inst_cpx(t_mem code, bool prefix)
//{
//  int result, operand1, operand2;
//
//  operand1 = regs.X;
//  operand2 = OPERAND(code, prefix);
//  result = operand1 - operand2;
//  FLAG_NZ (result);
//  FLAG_ASSIGN (BIT_V, 0x80 & (operand1 ^ operand2 ^ result ^ (result >>1)));
//  FLAG_ASSIGN (BIT_C, 0x100 & result);
//
//  return(resGO);
//}
//
//int
//cl_stm8::inst_jmp(t_mem code, bool prefix)
//{
//  PC = fetchea(code, prefix);
//
//  return(resGO);
//}
//
//int
//cl_stm8::inst_jsr(t_mem code, bool prefix)
//{
//  int newPC = fetchea(code, prefix);
//  
//  push2(PC);
//  PC = newPC;
//
//  return(resGO);
//}
//
//int
//cl_stm8::inst_bsr(t_mem code, bool prefix)
//{
//  signed char ofs = fetch();
//  
//  push2(PC);
//  PC += ofs;
//
//  return(resGO);
//}
//
//int
//cl_stm8::inst_ais(t_mem code, bool prefix)
//{
//  regs.SP = regs.SP + (signed char)fetch();
//  return(resGO);
//}
//
//int
//cl_stm8::inst_aix(t_mem code, bool prefix)
//{
//  int hx = (regs.H << 8) | (regs.X);
//  hx += (signed char)fetch();
//  regs.H = (hx >> 8) & 0xff;
//  regs.X = hx & 0xff;
//  return(resGO);
//}
//
//int
//cl_stm8::inst_and(t_mem code, bool prefix)
//{
//  regs.A = regs.A & OPERAND(code, prefix);
//  FLAG_CLEAR(BIT_V);
//  FLAG_NZ(regs.A);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_bit(t_mem code, bool prefix)
//{
//  uchar operand = regs.A & OPERAND(code, prefix);
//  FLAG_CLEAR(BIT_V);
//  FLAG_NZ(operand);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_ora(t_mem code, bool prefix)
//{
//  regs.A = regs.A | OPERAND(code, prefix);
//  FLAG_CLEAR(BIT_V);
//  FLAG_NZ(regs.A);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_eor(t_mem code, bool prefix)
//{
//  regs.A = regs.A ^ OPERAND(code, prefix);
//  FLAG_CLEAR(BIT_V);
//  FLAG_NZ(regs.A);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_asr(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  FLAG_ASSIGN (BIT_C, operand & 1);
//  operand = (operand >> 1) | (operand & 0x80);
//  FLAG_NZ (operand);
//  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_lsr(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  FLAG_ASSIGN (BIT_C, operand & 1);
//  operand = (operand >> 1) & 0x7f;
//  FLAG_NZ (operand);
//  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_lsl(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  FLAG_ASSIGN (BIT_C, operand & 0x80);
//  operand = (operand << 1);
//  FLAG_NZ (operand);
//  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_rol(t_mem code, bool prefix)
//{
//  uchar c = (regs.P & BIT_C)!=0;
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  FLAG_ASSIGN (BIT_C, operand & 0x80);
//  operand = (operand << 1) | c;
//  FLAG_NZ (operand);
//  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_ror(t_mem code, bool prefix)
//{
//  uchar c = (regs.P & BIT_C)!=0;
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  FLAG_ASSIGN (BIT_C, operand & 1);
//  operand = (operand >> 1) | (c << 7);
//  FLAG_NZ (operand);
//  FLAG_ASSIGN (BIT_V, ((regs.P & BIT_C)!=0) ^ ((regs.P & BIT_N)!=0));
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_inc(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  operand++;
//  FLAG_NZ (operand);
//  FLAG_ASSIGN (BIT_V, operand == 0x80);
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_dec(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  operand--;
//  FLAG_NZ (operand);
//  FLAG_ASSIGN (BIT_V, operand == 0x7f);
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//  return(resGO);
//}
//
//int
//cl_stm8::inst_dbnz(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  signed char ofs;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  operand--;
//  FLAG_NZ (operand);
//  FLAG_ASSIGN (BIT_V, operand == 0x7f);
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//
//  ofs = fetch();
//  if (operand)
//    PC += ofs;
//
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_tst(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  FLAG_NZ (operand);
//  FLAG_CLEAR (BIT_V);
//
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_clr(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  
//  operand = 0;
//  FLAG_CLEAR (BIT_V);
//  FLAG_CLEAR (BIT_N);
//  FLAG_SET (BIT_Z);
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    ea = fetchea(code,prefix);
//    store1(ea, operand);
//  }
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_clrh(t_mem code, bool prefix)
//{
//  FLAG_CLEAR (BIT_V);
//  FLAG_CLEAR (BIT_N);
//  FLAG_SET (BIT_Z);
//  regs.H = 0;
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_com(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  operand = ~operand;
//  FLAG_SET (BIT_C);
//  FLAG_NZ (operand);
//  FLAG_CLEAR (BIT_V);
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_neg(t_mem code, bool prefix)
//{
//  int ea = 0xffff;
//  uchar operand;
//  
//  if ((code & 0xf0) == 0x40)
//    operand = regs.A;
//  else if ((code & 0xf0) == 0x50)
//    operand = regs.X;
//  else {
//    ea = fetchea(code,prefix);
//    operand = get1(ea);
//  }
//
//  FLAG_ASSIGN (BIT_V, operand==0x80);
//  FLAG_ASSIGN (BIT_C, operand!=0x00);
//  operand = -operand;
//  FLAG_NZ (operand);
//
//  if ((code & 0xf0) == 0x40)
//    regs.A = operand;
//  else if ((code & 0xf0) == 0x50)
//    regs.X = operand;
//  else {
//    store1(ea, operand);
//  }
//  return(resGO);
//}
//
//
//
//int
//cl_stm8::inst_pushpull(t_mem code, bool prefix)
//{
//  switch (code) {
//    case 0x86:
//      pop1(regs.A);
//      break;
//    case 0x87:
//      push1(regs.A);
//      break;
//    case 0x88:
//      pop1(regs.X);
//      break;
//    case 0x89:
//      push1(regs.X);
//      break;
//    case 0x8a:
//      pop1(regs.H);
//      break;
//    case 0x8b:
//      push1(regs.H);
//      break;
//    default:
//      return(resHALT);
//  }
//  return(resGO);
//}
//
//
//
//
//int
//cl_stm8::inst_stop(t_mem code, bool prefix)
//{
//  FLAG_CLEAR (BIT_I);
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_wait(t_mem code, bool prefix)
//{
//  FLAG_CLEAR (BIT_I);
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_daa(t_mem code, bool prefix)
//{
//  uchar lsn, msn;
//
//  lsn = regs.A & 0xf;
//  msn = (regs.A >> 4) & 0xf;
//  if (regs.P & BIT_H) {
//    lsn += 16;
//    msn = (msn-1) & 0xf;
//  }
//  if (regs.P & BIT_C)
//    msn += 16;
//
//  FLAG_CLEAR (BIT_C);
//  while (lsn>9) {
//    lsn -= 10;
//    msn++;
//  }
//  if (msn>9) {
//    msn = msn % 10;
//    FLAG_SET (BIT_C);
//  }
//
//  return(resGO);
//}
//
//int
//cl_stm8::inst_mul(t_mem code, bool prefix)
//{
//  int result = regs.A * regs.X;
//  regs.A = result & 0xff;
//  regs.X = (result >> 8) & 0xff;
//  FLAG_CLEAR (BIT_C);
//  FLAG_CLEAR (BIT_H);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_div(t_mem code, bool prefix)
//{
//  unsigned int dividend = (regs.H << 8) | regs.A;
//  unsigned int quotient;
//
//  if (regs.X) {
//    quotient = dividend / (unsigned int)regs.X;
//    if (quotient<=0xff) {
//      regs.A = quotient;
//      regs.H = dividend % regs.X;
//      FLAG_CLEAR (BIT_C);
//      FLAG_ASSIGN (BIT_Z, quotient==0);
//    }
//    else
//      FLAG_SET (BIT_C);  // overflow
//  } else
//    FLAG_SET (BIT_C);    // division by zero
//
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_condbranch(t_mem code, bool prefix)
//{
//  bool taken;
//  signed char ofs;
//  unsigned char maskedP;
//  
//  if ((code & 0xf0)==0x20) {
//    switch ((code>>1) & 7) {
//      case 0: // BRA
//        taken = 1;
//        break;
//      case 1: // BHI
//        taken = !(regs.P & (BIT_C | BIT_Z));
//        break;
//      case 2: // BCC
//        taken = !(regs.P & BIT_C);
//        break;
//      case 3: // BNE
//        taken = !(regs.P & BIT_Z);
//        break;
//      case 4: // BHCC
//        taken = !(regs.P & BIT_H);
//        break;
//      case 5: // BPL
//        taken = !(regs.P & BIT_N);
//        break;
//      case 6: // BMC
//        taken = !(regs.P & BIT_I);
//        break;
//      case 7: // BIL
//        taken = 0; // TODO: should read simulated IRQ# pin
//      default:
//        return(resHALT);
//    } 
//  }
//  else if ((code & 0xf0)==0x90) {
//    switch ((code>>1) & 7) {
//      case 0: // BGE
//        maskedP = regs.P & (BIT_N | BIT_V);
//        taken = !maskedP || (maskedP == (BIT_N | BIT_V));
//        break;
//      case 1: // BGT
//        maskedP = regs.P & (BIT_N | BIT_V | BIT_Z);
//        taken = !maskedP || (maskedP == (BIT_N | BIT_V));
//        break;
//      default:
//        return(resHALT);
//    }
//  }
//  else
//    return(resHALT);
//  
//  if (code & 1)
//    taken = ! taken;
//  
//  ofs = fetch();
//  if (taken)
//    PC += ofs;
//
//  return(resGO);
//}
//
//int
//cl_stm8::inst_bitsetclear(t_mem code, bool prefix)
//{
//  uchar bit = (code >> 1) & 7;
//  int ea = fetchea(code, prefix);
//  uchar operand = get1(ea);
//
//  if (code & 1)
//    operand &= ~(1 << bit);
//  else
//    operand |= (1 << bit);
//  store1(ea, operand);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_bittestsetclear(t_mem code, bool prefix)
//{
//  uchar bit = (code >> 1) & 7;
//  int ea = fetchea(code, prefix);
//  uchar operand = get1(ea);
//  signed char ofs;
//  bool taken;
//  
//  if (code & 1)
//    taken = operand & (1 << bit);
//  else
//    taken = !(operand & (1 << bit));
//
//  ofs = fetch();
//  if (taken)
//    PC += ofs;
//
//  FLAG_ASSIGN (BIT_C, operand & (1 << bit));
//  return(resGO);
//}
//
//int
//cl_stm8::inst_cbeq(t_mem code, bool prefix)
//{
//  int ea;
//  uchar operand1, operand2;
//  signed char ofs;
//    
//  if ((code & 0xf0) == 0x40) {
//    operand1 = regs.A;
//    operand2 = fetch();
//  }
//  else if ((code & 0xf0) == 0x50) {
//    operand1 = regs.X;
//    operand2 = fetch();
//  }
//  else {
//    ea = fetchea(code,prefix);
//    operand1 = get1(ea);
//    operand2 = regs.A;
//  }
//
//  ofs = fetch();
//  if (operand1==operand2)
//    PC += ofs;  
//
//  if (code==0x71)
//    incx();
//    
//  return(resGO);
//}
//
//int
//cl_stm8::inst_rti(t_mem code, bool prefix)
//{
//  pop1(regs.P);
//  regs.P |= 0x60;
//  pop1(regs.A);
//  pop1(regs.X);
//  pop2(PC);
//  
//  return(resGO);
//}
//
//int
//cl_stm8::inst_rts(t_mem code, bool prefix)
//{
//  pop2(PC);
//  
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_mov(t_mem code, bool prefix)
//{
//  int ea;
//  uchar operand;
//  bool aix;
//  int hx = (regs.H << 8) | (regs.X);
//  
//  switch (code) {
//    case 0x4e:	//mov opr8a,opr8a
//      operand = get1(fetch());
//      ea = fetch();
//      aix = 0;
//      break;
//    case 0x5e:	//mov opr8a,x+
//      operand = get1(fetch());
//      ea = hx;
//      aix = 1;
//      break;
//    case 0x6e:	//mov #opr8i,opr8a
//      operand = fetch();
//      ea = fetch();
//      aix = 0;
//      break;
//    case 0x7e:	//mov x+,opr8a
//      operand = get1(hx);
//      ea = fetch();
//      aix = 1;
//      break;
//    default:
//      return(resHALT);
//  }
//  
//  store1(ea, operand);
//  if (aix)
//    incx();
//
//  FLAG_NZ(operand);
//  FLAG_CLEAR(BIT_V);
//    
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_sthx(t_mem code, bool prefix)
//{
//  int ea = fetch1();
//  
//  store1(ea, regs.H);
//  store1((ea+1) & 0xffff, regs.X);
//
//  FLAG_CLEAR(BIT_V);
//  FLAG_ASSIGN(BIT_N, regs.H & 0x80);
//  FLAG_ASSIGN(BIT_Z, !regs.X && !regs.H);
//  return(resGO);
//}
//
//int
//cl_stm8::inst_ldhx(t_mem code, bool prefix)
//{
//  int ea;
//  
//  if (code == 0x45) {
//    regs.H = fetch();
//    regs.X = fetch();
//  }
//  else if (code == 0x55) {
//    ea = fetch();
//    regs.H = get1(ea);
//    regs.X = get1(ea+1);
//  }
//  else
//    return(resHALT);
//  
//  FLAG_CLEAR(BIT_V);
//  FLAG_ASSIGN(BIT_N, regs.H & 0x80);
//  FLAG_ASSIGN(BIT_Z, !regs.X && !regs.H);
//  return(resGO);
//}
//
//
//int
//cl_stm8::inst_cphx(t_mem code, bool prefix)
//{
//  int ea;
//  int hx;
//  int operand;
//  int result;
//  
//  if (code == 0x65) {
//    operand = fetch2();
//  }
//  else if (code == 0x75) {
//    ea = fetch();
//    operand = (get1(ea) << 8) | get1(ea+1);
//  }
//  else
//    return(resHALT);
//
//  hx = (regs.H << 8) | regs.X;
//  result = hx-operand;
//
//  FLAG_ASSIGN (BIT_V, 0x8000 & (hx ^ operand ^ result ^ (result>>1)));
//  FLAG_ASSIGN (BIT_C, 0x10000 & result);
//  FLAG_ASSIGN(BIT_N, result & 0x8000);
//  FLAG_ASSIGN(BIT_Z, !(result & 0xffff));
//                              
//  return(resGO);
//}
//
//int
//cl_stm8::inst_swi(t_mem code, bool prefix)
//{
//  push2(PC);
//  push1(regs.X);
//  push1(regs.A);
//  push1(regs.P);
//  FLAG_CLEAR(BIT_I);
//  
//  PC = get2(0xfffc);
//
//  return(resGO);
//}
//

/* End of stm8.src/inst.cc */
