/*
 * Simulator of microcontrollers (z80.cc)
 * 
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * by Karl Bongers, added code into ucSim from rrgb.
 * by Michael Hope <michaelh@juju.net.nz> into rrgb.
 * cxmon (C) 1997-2000 Christian Bauer, Marc Hellwig
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
#include "z80cl.h"
#include "glob.h"
#include "regsz80.h"

#define uint32 t_addr
#define uint8 unsigned char
#define int8 char
class cl_z80 *our_class;
#define mon_read_byte(_adr) ((uchar)our_class->get_mem(MEM_ROM, _adr))

/*******************************************************************/
// following mostly ripped from MH's disasm.cc code

// Addressing modes
enum {
	A_IMPL,
	A_IMM8,		// xx
	A_IMM16,	// xxxx
	A_ABS8,		// (xx)
	A_ABS16,	// (xxxx)
	A_REL,		// relative
	A_A,		// a
	A_HL,		// hl or ix or iy
	A_SP,		// sp
	A_REG1,		// 8-bit register (bits 0..2 of opcode) or (hl)/(ix+d)/(iy+d)
	A_REG1X,	// 8-bit register (bits 0..2 of opcode) or (hl)/(ix+d)/(iy+d), don't substitute h or l on prefixes
	A_REG2,		// 8-bit register (bits 3..5 of opcode) or (hl)/(ix+d)/(iy+d)
	A_REG2X,	// 8-bit register (bits 3..5 of opcode) or (hl)/(ix+d)/(iy+d), don't substitute h or l on prefixes
	A_REG3,		// 16-bit register (bits 4..5 of opcode) bc/de/hl/sp
	A_REG4,		// 16-bit register (bits 4..5 of opcode) bc/de/hl/af
	A_COND,		// condition code (bits 3..5 of opcode)
	A_COND2,	// condition code (bits 3..4 of opcode)
	A_BIT,		// bit number (bits 3..5 of opcode)
	A_RST,		// restart
	A_BC_IND,	// (bc)
	A_DE_IND,	// (de)
	A_HL_IND,	// (hl) or (ix) or (iy)
	A_SP_IND,	// (sp)
	A_DE_HL,	// de,hl
	A_AF_AF,	// af,af'
};

// Mnemonics
enum {
	M_ADC, M_ADD, M_AND, M_BIT, M_CALL, M_CCF, M_CP, M_CPD, M_CPDR, M_CPI,
	M_CPIR, M_CPL, M_DAA, M_DEC, M_DI, M_DJNZ, M_EI, M_EX, M_EXX, M_HALT,
	M_IM0, M_IM1, M_IM2, M_IN, M_INC, M_IND, M_INDR, M_INI, M_INIR, M_JP,
	M_JR, M_LD, M_LDD, M_LDDR, M_LDI, M_LDIR, M_NEG, M_NOP, M_OR, M_OTDR,
	M_OTIR, M_OUT, M_OUTD, M_OUTI, M_POP, M_PUSH, M_RES, M_RET, M_RETI,
	M_RETN, M_RL, M_RLA, M_RLC, M_RLCA, M_RLD, M_RR, M_RRA, M_RRC, M_RRCA,
	M_RRD, M_RST, M_SBC, M_SCF, M_SET, M_SL1, M_SLA, M_SRA, M_SRL, M_SUB,
	M_XOR,
	M_ILLEGAL,

	M_MAXIMUM
};

// Chars for each mnemonic
static const char mnem_1[] = "aaabccccccccddddeeehiiiiiiiiijjlllllnnoooooopprrrrrrrrrrrrrrrssssssssx?";
static const char mnem_2[] = "ddniacppppppaeijixxammmnnnnnnprdddddeorttuuuoueeeelllllrrrrrsbcellrruo ";
static const char mnem_3[] = "cddtlf ddiilac n  xl    cddii   ddiigp ditttpssttt accd accdtcft1aalbr ";
static const char mnem_4[] = "    l   r r    z   t012   r r    r r   rr di h  in   a    a            ";

// Mnemonic for each opcode
static const char mnemonic[256] = {
	M_NOP , M_LD , M_LD , M_INC    , M_INC , M_DEC    , M_LD  , M_RLCA,	// 00
	M_EX  , M_ADD, M_LD , M_DEC    , M_INC , M_DEC    , M_LD  , M_RRCA,
	M_DJNZ, M_LD , M_LD , M_INC    , M_INC , M_DEC    , M_LD  , M_RLA ,	// 10
	M_JR  , M_ADD, M_LD , M_DEC    , M_INC , M_DEC    , M_LD  , M_RRA ,

	M_JR  , M_LD , M_LD , M_INC    , M_INC , M_DEC    , M_LD  , M_DAA ,	// 20
	M_JR  , M_ADD, M_LD , M_DEC    , M_INC , M_DEC    , M_LD  , M_CPL ,

	M_JR  , M_LD , M_LD , M_INC    , M_INC , M_DEC    , M_LD  , M_SCF ,	// 30
	M_JR  , M_ADD, M_LD , M_DEC    , M_INC , M_DEC    , M_LD  , M_CCF ,

	M_LD  , M_LD , M_LD , M_LD     , M_LD  , M_LD     , M_LD  , M_LD  ,	// 40
	M_LD  , M_LD , M_LD , M_LD     , M_LD  , M_LD     , M_LD  , M_LD  ,
	M_LD  , M_LD , M_LD , M_LD     , M_LD  , M_LD     , M_LD  , M_LD  ,	// 50
	M_LD  , M_LD , M_LD , M_LD     , M_LD  , M_LD     , M_LD  , M_LD  ,
	M_LD  , M_LD , M_LD , M_LD     , M_LD  , M_LD     , M_LD  , M_LD  ,	// 60
	M_LD  , M_LD , M_LD , M_LD     , M_LD  , M_LD     , M_LD  , M_LD  ,
	M_LD  , M_LD , M_LD , M_LD     , M_LD  , M_LD     , M_HALT, M_LD  ,	// 70
	M_LD  , M_LD , M_LD , M_LD     , M_LD  , M_LD     , M_LD  , M_LD  ,

	M_ADD , M_ADD, M_ADD, M_ADD    , M_ADD , M_ADD    , M_ADD , M_ADD ,	// 80
	M_ADC , M_ADC, M_ADC, M_ADC    , M_ADC , M_ADC    , M_ADC , M_ADC ,

	M_SUB , M_SUB, M_SUB, M_SUB    , M_SUB , M_SUB    , M_SUB , M_SUB ,	// 90
	M_SBC , M_SBC, M_SBC, M_SBC    , M_SBC , M_SBC    , M_SBC , M_SBC ,

	M_AND , M_AND, M_AND, M_AND    , M_AND , M_AND    , M_AND , M_AND ,	// a0
	M_XOR , M_XOR, M_XOR, M_XOR    , M_XOR , M_XOR    , M_XOR , M_XOR ,

	M_OR  , M_OR , M_OR , M_OR     , M_OR  , M_OR     , M_OR  , M_OR  ,	// b0
	M_CP  , M_CP , M_CP , M_CP     , M_CP  , M_CP     , M_CP  , M_CP  ,

	M_RET , M_POP, M_JP , M_JP     , M_CALL, M_PUSH   , M_ADD , M_RST ,	// c0
	M_RET , M_RET, M_JP , M_ILLEGAL, M_CALL, M_CALL   , M_ADC , M_RST ,

	M_RET , M_POP, M_JP , M_OUT    , M_CALL, M_PUSH   , M_SUB , M_RST ,	// d0
	M_RET , M_EXX, M_JP , M_IN     , M_CALL, M_ILLEGAL, M_SBC , M_RST ,

	M_RET , M_POP, M_JP , M_EX     , M_CALL, M_PUSH   , M_AND , M_RST ,	// e0
	M_RET , M_JP , M_JP , M_EX     , M_CALL, M_ILLEGAL, M_XOR , M_RST ,

	M_RET , M_POP, M_JP , M_DI     , M_CALL, M_PUSH   , M_OR  , M_RST ,	// f0
	M_RET , M_LD , M_JP , M_EI     , M_CALL, M_ILLEGAL, M_CP  , M_RST
};

// Source/destination addressing modes for each opcode
#define A(d,s) (((A_ ## d) << 8) | (A_ ## s))

static const short adr_mode[256] = {
	A(IMPL,IMPL)  , A(REG3,IMM16) , A(BC_IND,A)  , A(REG3,IMPL) , A(REG2,IMPL) , A(REG2,IMPL) , A(REG2,IMM8) , A(IMPL,IMPL) ,	// 00
	A(AF_AF,IMPL) , A(HL,REG3)    , A(A,BC_IND)  , A(REG3,IMPL) , A(REG2,IMPL) , A(REG2,IMPL) , A(REG2,IMM8) , A(IMPL,IMPL) ,
	A(REL,IMPL)   , A(REG3,IMM16) , A(DE_IND,A)  , A(REG3,IMPL) , A(REG2,IMPL) , A(REG2,IMPL) , A(REG2,IMM8) , A(IMPL,IMPL) ,	// 10
	A(REL,IMPL)   , A(HL,REG3)    , A(A,DE_IND)  , A(REG3,IMPL) , A(REG2,IMPL) , A(REG2,IMPL) , A(REG2,IMM8) , A(IMPL,IMPL) ,
	A(COND2,REL)  , A(REG3,IMM16) , A(ABS16,HL)  , A(REG3,IMPL) , A(REG2,IMPL) , A(REG2,IMPL) , A(REG2,IMM8) , A(IMPL,IMPL) ,	// 20
	A(COND2,REL)  , A(HL,REG3)    , A(HL,ABS16)  , A(REG3,IMPL) , A(REG2,IMPL) , A(REG2,IMPL) , A(REG2,IMM8) , A(IMPL,IMPL) ,
	A(COND2,REL)  , A(REG3,IMM16) , A(ABS16,A)   , A(REG3,IMPL) , A(REG2,IMPL) , A(REG2,IMPL) , A(REG2,IMM8) , A(IMPL,IMPL) ,	// 30
	A(COND2,REL)  , A(HL,REG3)    , A(A,ABS16)   , A(REG3,IMPL) , A(REG2,IMPL) , A(REG2,IMPL) , A(REG2,IMM8) , A(IMPL,IMPL) ,
	A(REG2,REG1)  , A(REG2,REG1)  , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2X,REG1), A(REG2,REG1) ,	// 40
	A(REG2,REG1)  , A(REG2,REG1)  , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2X,REG1), A(REG2,REG1) ,
	A(REG2,REG1)  , A(REG2,REG1)  , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2X,REG1), A(REG2,REG1) ,	// 50
	A(REG2,REG1)  , A(REG2,REG1)  , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2X,REG1), A(REG2,REG1) ,
	A(REG2,REG1)  , A(REG2,REG1)  , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2X,REG1), A(REG2,REG1) ,	// 60
	A(REG2,REG1)  , A(REG2,REG1)  , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2X,REG1), A(REG2,REG1) ,
	A(REG2,REG1X) , A(REG2,REG1X) , A(REG2,REG1X), A(REG2,REG1X), A(REG2,REG1X), A(REG2,REG1X), A(IMPL,IMPL) , A(REG2,REG1X),	// 70
	A(REG2,REG1)  , A(REG2,REG1)  , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2,REG1) , A(REG2X,REG1), A(REG2,REG1) ,
	A(A,REG1)     , A(A,REG1)     , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    ,	// 80
	A(A,REG1)     , A(A,REG1)     , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    ,
	A(REG1,IMPL)  , A(REG1,IMPL)  , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) ,	// 90
	A(A,REG1)     , A(A,REG1)     , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    , A(A,REG1)    ,
	A(REG1,IMPL)  , A(REG1,IMPL)  , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) ,	// a0
	A(REG1,IMPL)  , A(REG1,IMPL)  , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) ,
	A(REG1,IMPL)  , A(REG1,IMPL)  , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) ,	// b0
	A(REG1,IMPL)  , A(REG1,IMPL)  , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) , A(REG1,IMPL) ,
	A(COND,IMPL)  , A(REG4,IMPL)  , A(COND,IMM16), A(IMM16,IMPL), A(COND,IMM16), A(REG4,IMPL) , A(A,IMM8)    , A(RST,IMPL)  ,	// c0
	A(COND,IMPL)  , A(IMPL,IMPL)  , A(COND,IMM16), A(IMPL,IMPL) , A(COND,IMM16), A(IMM16,IMPL), A(A,IMM8)    , A(RST,IMPL)  ,
	A(COND,IMPL)  , A(REG4,IMPL)  , A(COND,IMM16), A(ABS8,A)    , A(COND,IMM16), A(REG4,IMPL) , A(IMM8,IMPL) , A(RST,IMPL)  ,	// d0
	A(COND,IMPL)  , A(IMPL,IMPL)  , A(COND,IMM16), A(A,ABS8)    , A(COND,IMM16), A(IMPL,IMPL) , A(A,IMM8)    , A(RST,IMPL)  ,
	A(COND,IMPL)  , A(REG4,IMPL)  , A(COND,IMM16), A(SP_IND,HL) , A(COND,IMM16), A(REG4,IMPL) , A(IMM8,IMPL) , A(RST,IMPL)  ,	// e0
	A(COND,IMPL)  , A(HL_IND,IMPL), A(COND,IMM16), A(DE_HL,IMPL), A(COND,IMM16), A(IMPL,IMPL) , A(IMM8,IMPL) , A(RST,IMPL)  ,
	A(COND,IMPL)  , A(REG4,IMPL)  , A(COND,IMM16), A(IMPL,IMPL) , A(COND,IMM16), A(REG4,IMPL) , A(IMM8,IMPL) , A(RST,IMPL)  ,	// f0
	A(COND,IMPL)  , A(SP,HL)      , A(COND,IMM16), A(IMPL,IMPL) , A(COND,IMM16), A(IMPL,IMPL) , A(IMM8,IMPL) , A(RST,IMPL)
};


/*
 *  sprintf into a "stream"
 */

struct SFILE {
	char *buffer;
	char *current;
};

static int mon_sprintf(SFILE *f, const char *format, ...)
{
	int n;
	va_list args;
	va_start(args, format);
	vsprintf(f->current, format, args);
	f->current += n = strlen(f->current);
	va_end(args);
	return n;
}


/*
 *  Disassemble one instruction, return number of bytes
 */

static const char *reg_name[] = {"b", "c", "d", "e", "h", "l", "*", "a"};
static const char *reg_name_ix[] = {"b", "c", "d", "e", "hx", "lx", "*", "a"};	// undoc
static const char *reg_name_iy[] = {"b", "c", "d", "e", "hy", "ly", "*", "a"};	// undoc
static const char *reg_name_16[] = {"bc", "de", "hl", "sp"};
static const char *reg_name_16_2[] = {"bc", "de", "hl", "af"};
static const char *cond_name[] = {"nz", "z", "nc", "c", "po", "pe", "p", "m"};

static void operand(SFILE *f, char mode, uint32 &adr, uint8 op, bool ix, bool iy)
{
	switch (mode) {
		case A_IMPL:
			break;

		case A_IMM8:
			mon_sprintf(f, "$%02x", mon_read_byte(adr)); adr++;
			break;

		case A_IMM16:
			mon_sprintf(f, "$%04x", (mon_read_byte(adr + 1) << 8) | mon_read_byte(adr)); adr += 2;
			break;

		case A_ABS8:
			mon_sprintf(f, "($%02x)", mon_read_byte(adr)); adr++;
			break;

		case A_ABS16:
			mon_sprintf(f, "($%04x)", (mon_read_byte(adr + 1) << 8) | mon_read_byte(adr)); adr += 2;
			break;

		case A_REL:
			mon_sprintf(f, "$%04x", (adr + 2 + (int8)mon_read_byte(adr)) & 0xffff); adr++;
			break;

		case A_A:
			mon_sprintf(f, "a");
			break;

		case A_HL:
			mon_sprintf(f, ix ? "ix" : (iy ? "iy" : "hl"));
			break;

		case A_SP:
			mon_sprintf(f, "sp");
			break;

		case A_REG1:
		case A_REG1X: {
			int reg = op & 7;
			if (reg == 6) {
				if (ix || iy) {
					mon_sprintf(f, "(%s+$%02x)", ix ? "ix" : "iy", mon_read_byte(adr)); adr++;
				} else
					mon_sprintf(f, "(hl)");
			} else if (mode == A_REG1)
				mon_sprintf(f, "%s", ix ? reg_name_ix[reg] : (iy ? reg_name_iy[reg] : reg_name[reg]));
			else
				mon_sprintf(f, "%s", reg_name[reg]);
			break;
		}

		case A_REG2:
		case A_REG2X: {
			int reg = (op >> 3) & 7;
			if (reg == 6) {
				if (ix || iy) {
					mon_sprintf(f, "(%s+$%02x)", ix ? "ix" : "iy", mon_read_byte(adr)); adr++;
				} else
					mon_sprintf(f, "(hl)");
			} else if (mode == A_REG2)
				mon_sprintf(f, "%s", ix ? reg_name_ix[reg] : (iy ? reg_name_iy[reg] : reg_name[reg]));
			else
				mon_sprintf(f, "%s", reg_name[reg]);
			break;
		}

		case A_REG3:
			mon_sprintf(f, reg_name_16[(op >> 4) & 3]);
			break;

		case A_REG4:
			mon_sprintf(f, ix ? "ix" : (iy ? "iy" : reg_name_16_2[(op >> 4) & 3]));
			//mon_sprintf(f, reg_name_16_2[(op >> 4) & 3]);
			// kpb, not sure, to fix: dd e5 push hl(should be push ix), also
			// dd e1 pop hl(should be pop ix).
			break;

		case A_COND:
			mon_sprintf(f, cond_name[(op >> 3) & 7]);
			break;

		case A_COND2:
			mon_sprintf(f, cond_name[(op >> 3) & 3]);
			break;

		case A_BIT:
			mon_sprintf(f, "%d", (op >> 3) & 7);
			break;

		case A_RST:
			mon_sprintf(f, "$%02x", op & 0x38);
			break;

		case A_BC_IND:
			mon_sprintf(f, "(bc)");
			break;

		case A_DE_IND:
			mon_sprintf(f, "(de)");
			break;

		case A_HL_IND:
			mon_sprintf(f, ix ? "(ix)" : (iy ? "(iy)" : "(hl)"));
			break;

		case A_SP_IND:
			mon_sprintf(f, "(sp)");
			break;

		case A_DE_HL:
			mon_sprintf(f, "de,hl");
			break;

		case A_AF_AF:
			mon_sprintf(f, "af,af'");
			break;
	}
}

static int print_instr(SFILE *f, char mnem, char dst_mode, char src_mode, uint32 adr, uint8 op, bool ix, bool iy)
{
	uint32 orig_adr = adr;

	// Print mnemonic
	mon_sprintf(f, "%c%c%c%c ", mnem_1[mnem], mnem_2[mnem], mnem_3[mnem], mnem_4[mnem]);

	// Print destination operand
	operand(f, dst_mode, adr, op, ix, iy);

	// Print source operand
	if (src_mode != A_IMPL)
		mon_sprintf(f, ",");
	operand(f, src_mode, adr, op, ix, iy);

	return adr - orig_adr;
}


static int disass_cb(SFILE *f, uint32 adr, bool ix, bool iy)
{
	int num;

	// Fetch opcode
	uint8 op;
	if (ix || iy) {
		op = mon_read_byte(adr + 1);
		num = 2;
	} else {
		op = mon_read_byte(adr);
		num = 1;
	}

	// Decode mnemonic and addressing modes
	char mnem, dst_mode = A_IMPL, src_mode = A_IMPL;
	switch (op & 0xc0) {
		case 0x00:
			dst_mode = A_REG1;
			switch ((op >> 3) & 7) {
				case 0: mnem = M_RLC; break;
				case 1: mnem = M_RRC; break;
				case 2: mnem = M_RL; break;
				case 3: mnem = M_RR; break;
				case 4: mnem = M_SLA; break;
				case 5: mnem = M_SRA; break;
				case 6: mnem = M_SL1; break;
				case 7: mnem = M_SRL; break;
			}
			break;
		case 0x40:
			mnem = M_BIT; dst_mode = A_BIT; src_mode = A_REG1;
			break;
		case 0x80:
			mnem = M_RES; dst_mode = A_BIT; src_mode = A_REG1;
			break;
		case 0xc0:
			mnem = M_SET; dst_mode = A_BIT; src_mode = A_REG1;
			break;
	}

	// Print instruction
	print_instr(f, mnem, dst_mode, src_mode, adr, op, ix, iy);
	return num;
}

static int disass_ed(SFILE *f, uint32 adr)
{
	// Fetch opcode
	uint8 op = mon_read_byte(adr);

	// Decode mnemonic and addressing modes
	char mnem, dst_mode = A_IMPL, src_mode = A_IMPL;
	switch (op) {
		case 0x40:
		case 0x48:
		case 0x50:
		case 0x58:
		case 0x60:
		case 0x68:
		case 0x78:
			mon_sprintf(f, "in\t%s,(c)", reg_name[(op >> 3) & 7]);
			return 1;
		case 0x70:
			mon_sprintf(f, "in\t(c)");
			return 1;

		case 0x41:
		case 0x49:
		case 0x51:
		case 0x59:
		case 0x61:
		case 0x69:
		case 0x79:
			mon_sprintf(f, "out\t(c),%s", reg_name[(op >> 3) & 7]);
			return 1;
		case 0x71:	// undoc
			mon_sprintf(f, "out\t(c),0");
			return 1;

		case 0x42:
		case 0x52:
		case 0x62:
		case 0x72:
			mnem = M_SBC; dst_mode = A_HL; src_mode = A_REG3;
			break;

		case 0x43:
		case 0x53:
		case 0x63:
		case 0x73:
			mnem = M_LD; dst_mode = A_ABS16; src_mode = A_REG3;
			break;

		case 0x4a:
		case 0x5a:
		case 0x6a:
		case 0x7a:
			mnem = M_ADC; dst_mode = A_HL; src_mode = A_REG3;
			break;

		case 0x4b:
		case 0x5b:
		case 0x6b:
		case 0x7b:
			mnem = M_LD; dst_mode = A_REG3; src_mode = A_ABS16;
			break;

		case 0x44:
		case 0x4c:	// undoc
		case 0x54:	// undoc
		case 0x5c:	// undoc
		case 0x64:	// undoc
		case 0x6c:	// undoc
		case 0x74:	// undoc
		case 0x7c:	// undoc
			mnem = M_NEG;
			break;

		case 0x45:
		case 0x55:	// undoc
		case 0x5d:	// undoc
		case 0x65:	// undoc
		case 0x6d:	// undoc
		case 0x75:	// undoc
		case 0x7d:	// undoc
			mnem = M_RETN;
			break;
		case 0x4d: mnem = M_RETI; break;

		case 0x46:
		case 0x4e:	// undoc
		case 0x66:	// undoc
		case 0x6e:	// undoc
			mnem = M_IM0;
			break;
		case 0x56:
		case 0x76:	// undoc
			mnem = M_IM1;
			break;
		case 0x5e:
		case 0x7e:	// undoc
			mnem = M_IM2;
			break;

		case 0x47:
			mon_sprintf(f, "ld\ti,a");
			return 1;
		case 0x4f:
			mon_sprintf(f, "ld\tr,a");
			return 1;
		case 0x57:
			mon_sprintf(f, "ld\ta,i");
			return 1;
		case 0x5f:
			mon_sprintf(f, "ld\ta,r");
			return 1;

		case 0x67: mnem = M_RRD; break;
		case 0x6f: mnem = M_RLD; break;

		case 0xa0: mnem = M_LDI; break;
		case 0xa1: mnem = M_CPI; break;
		case 0xa2: mnem = M_INI; break;
		case 0xa3: mnem = M_OUTI; break;
		case 0xa8: mnem = M_LDD; break;
		case 0xa9: mnem = M_CPD; break;
		case 0xaa: mnem = M_IND; break;
		case 0xab: mnem = M_OUTD; break;
		case 0xb0: mnem = M_LDIR; break;
		case 0xb1: mnem = M_CPIR; break;
		case 0xb2: mnem = M_INIR; break;
		case 0xb3: mnem = M_OTIR; break;
		case 0xb8: mnem = M_LDDR; break;
		case 0xb9: mnem = M_CPDR; break;
		case 0xba: mnem = M_INDR; break;
		case 0xbb: mnem = M_OTDR; break;

		default:
			mnem = M_ILLEGAL;
			break;
	}

	// Print instruction
	return print_instr(f, mnem, dst_mode, src_mode, adr + 1, op, false, false) + 1;
}

static int disass(SFILE *f, uint32 adr, bool ix, bool iy)
{
	uint8 op = mon_read_byte(adr);
	if (op == 0xcb)
		return disass_cb(f, adr + 1, ix, iy) + 1;
	else
		return print_instr(f, mnemonic[op], adr_mode[op] >> 8, adr_mode[op] & 0xff, adr + 1, op, ix, iy) + 1;
}

static int our_dis_z80(char *s, uint32 adr)
{
	int num;
	char buf[64];
	SFILE sfile = {buf, buf};

	switch (mon_read_byte(adr)) {
		case 0xdd:	// ix prefix
			num = disass(&sfile, adr + 1, true, false) + 1;
			break;
		case 0xed:
			num = disass_ed(&sfile, adr + 1) + 1;
			break;
		case 0xfd:	// iy prefix
			num = disass(&sfile, adr + 1, false, true) + 1;
			break;
		default:
			num = disass(&sfile, adr, false, false);
			break;
	}

 strcpy(s, buf);
	return num;
}

/*******************************************************************/


/*
 * Base type of Z80 controllers
 */

cl_z80::cl_z80(class cl_sim *asim):
  cl_uc(asim)
{
  type= CPU_Z80;
}

int
cl_z80::init(void)
{
  cl_uc::init(); /* Memories now exist */
  ram= mem(MEM_XRAM);
  rom= mem(MEM_ROM);
  return(0);
}

char *
cl_z80::id_string(void)
{
  return("unspecified Z80");
}


/*
 * Making elements of the controller
 */

t_addr
cl_z80::get_mem_size(enum mem_class type)
{
  switch(type)
    {
    case MEM_ROM: return(0x10000);
    case MEM_XRAM: return(0x10000);
    default: return(0);
    }
 return(cl_uc::get_mem_size(type));
}

void
cl_z80::mk_hw_elements(void)
{
  //class cl_base *o;
  /* t_uc::mk_hw() does nothing */
}


/*
 * Help command interpreter
 */

struct dis_entry *
cl_z80::dis_tbl(void)
{
  return(disass_z80);
}

/*struct name_entry *
cl_z80::sfr_tbl(void)
{
  return(0);
}*/

/*struct name_entry *
cl_z80::bit_tbl(void)
{
  //FIXME
  return(0);
}*/

int
cl_z80::inst_length(t_mem code, t_addr addr)
{
  struct dis_entry *tabl= dis_tbl();
  //t_mem next;
  char tmpstr[60];
  uint32 i;

  our_class = this;

  if ((tabl[code].code == 0xdd) || (tabl[code].code == 0xed) ||
      (tabl[code].code == 0xfd)) {
    i = our_dis_z80(tmpstr, (uint32) addr);
    return i;
  }

  return tabl[code].length;
}

/* can't use the following, need to look besides first byte */
int
cl_z80::inst_length(t_mem code)
{
  struct dis_entry *tabl= dis_tbl();
  printf("*******WOOPS!\n");
  exit(1);
  return tabl[code].length;
}

int
cl_z80::longest_inst(void)
{
  return 4;  /* Wild guess, fix me!  What is it used for? */
}

char *
cl_z80::disass(t_addr addr, char *sep)
{
  char work[256], temp[20];
  char *buf, *p, *b, *t;
  uint code;
  int i;

  our_class = this;

  p= work;
  
  code= get_mem(MEM_ROM, addr);

  i= 0;
  while ((code & dis_tbl()[i].mask) != dis_tbl()[i].code &&
         dis_tbl()[i].mnemonic)
    i++;

  if (dis_tbl()[i].mnemonic == NULL)
    {
      buf= (char*)malloc(30);
      strcpy(buf, "UNKNOWN/INVALID");
      return(buf);
    }
  b= dis_tbl()[i].mnemonic;
  if (*b== '?') {
    switch(dis_tbl()[i].code) {
      case 0xcb:  /* ESC code to lots of op-codes, all 2-byte */
        buf= (char*)malloc(60);
        i = our_dis_z80(buf, (uint32) addr);
        b = buf;
      break;
      case 0xdd: /* ESC codes,about 284, vary lengths, IX centric */
        buf= (char*)malloc(60);
        i = our_dis_z80(buf, (uint32) addr);
        b = buf;
      break;
      case 0xed: /* ESC code to about 80 opcodes of various lengths */
        buf= (char*)malloc(60);
        i = our_dis_z80(buf, (uint32) addr);
        b = buf;
      break;
      case 0xfe: /* ESC codes,about 284, vary lengths, IY centric */
        buf= (char*)malloc(60);
        i = our_dis_z80(buf, (uint32) addr);
        b = buf;
      break;
    }
  }

  while (*b)
    {
      if (*b == '%')
	{
	  b++;
	  switch (*(b++))
	    {
	    case 'd': // d    jump relative target, signed? byte immediate operand
	      sprintf(temp, "#0x%x", (uint)get_mem(MEM_ROM, addr+1));
	      break;
	    case 'w': // w    word immediate operand
	      sprintf(temp, "#0x%x",
	         (uint)((get_mem(MEM_ROM, addr+1)) | (get_mem(MEM_ROM, addr+2)<<8)) );
	      break;
	    case 'b': // b    byte immediate operand
	      sprintf(temp, "#0x%x", (uint)get_mem(MEM_ROM, addr+1));
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
  for (p= work, b= buf; *p != ' '; p++, b++)
    *b= *p;
  p++;
  *b= '\0';
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
cl_z80::print_regs(class cl_console *con)
{
  con->dd_printf("SZ-A--P-C  Flags= 0x%02x %3d %c  ",
		 regs.F, regs.F, isprint(regs.F)?regs.F:'.');
  con->dd_printf("A= 0x%02x %3d %c\n",
		 regs.A, regs.A, isprint(regs.A)?regs.A:'.');
  con->dd_printf("%c%c-%c--%c-%c\n",
		 (regs.F&BIT_S)?'1':'0',
		 (regs.F&BIT_Z)?'1':'0',
		 (regs.F&BIT_A)?'1':'0',
		 (regs.F&BIT_P)?'1':'0',
		 (regs.F&BIT_C)?'1':'0');
  con->dd_printf("BC= 0x%04x [BC]= %02x %3d %c  ",
		 regs.BC, ram->get(regs.BC), ram->get(regs.BC),
		 isprint(ram->get(regs.BC))?ram->get(regs.BC):'.');
  con->dd_printf("DE= 0x%04x [DE]= %02x %3d %c  ",
		 regs.DE, ram->get(regs.DE), ram->get(regs.DE),
		 isprint(ram->get(regs.DE))?ram->get(regs.DE):'.');
  con->dd_printf("HL= 0x%04x [HL]= %02x %3d %c\n",
		 regs.HL, ram->get(regs.HL), ram->get(regs.HL),
		 isprint(ram->get(regs.HL))?ram->get(regs.HL):'.');
  con->dd_printf("IX= 0x%04x [IX]= %02x %3d %c  ",
		 regs.IX, ram->get(regs.IX), ram->get(regs.IX),
		 isprint(ram->get(regs.IX))?ram->get(regs.IX):'.');
  con->dd_printf("IY= 0x%04x [IY]= %02x %3d %c  ",
		 regs.IY, ram->get(regs.IY), ram->get(regs.IY),
		 isprint(ram->get(regs.IY))?ram->get(regs.IY):'.');
  con->dd_printf("SP= 0x%04x [SP]= %02x %3d %c\n",
		 regs.SP, ram->get(regs.SP), ram->get(regs.SP),
		 isprint(ram->get(regs.SP))?ram->get(regs.SP):'.');
  
  print_disass(PC, con);
}

/*
 * Execution
 */

int
cl_z80::exec_inst(void)
{
  t_mem code;

  if (fetch(&code))
    return(resBREAKPOINT);
  tick(1);
  switch (code)
    {
    case 0x00: return(inst_nop(code));
    case 0x01: case 0x02: case 0x06: return(inst_ld(code));
    case 0x03: case 0x04: return(inst_inc(code));
    case 0x05: return(inst_dec(code));
    case 0x07: return(inst_rlca(code));

    case 0x08: return(inst_ex(code));
    case 0x09: return(inst_add(code));
    case 0x0a: case 0x0e: return(inst_ld(code));
    case 0x0b: case 0x0d: return(inst_dec(code));
    case 0x0c: return(inst_inc(code));
    case 0x0f: return(inst_rrca(code));


    case 0x10: return(inst_djnz(code));
    case 0x11: case 0x12: case 0x16: return(inst_ld(code));
    case 0x13: case 0x14: return(inst_inc(code));
    case 0x15: return(inst_dec(code));
    case 0x17: return(inst_rla(code));

    case 0x18: return(inst_jr(code));
    case 0x19: return(inst_add(code));
    case 0x1a: case 0x1e: return(inst_ld(code));
    case 0x1b: case 0x1d: return(inst_dec(code));
    case 0x1c: return(inst_inc(code));
    case 0x1f: return(inst_rra(code));


    case 0x20: return(inst_jr(code));
    case 0x21: case 0x22: case 0x26: return(inst_ld(code));
    case 0x23: case 0x24: return(inst_inc(code));
    case 0x25: return(inst_dec(code));
    case 0x27: return(inst_daa(code));

    case 0x28: return(inst_jr(code));
    case 0x29: return(inst_add(code));
    case 0x2a: case 0x2e: return(inst_ld(code));
    case 0x2b: case 0x2d: return(inst_dec(code));
    case 0x2c: return(inst_inc(code));
    case 0x2f: return(inst_cpl(code));


    case 0x30: return(inst_jr(code));
    case 0x31: case 0x32: case 0x36: return(inst_ld(code));
    case 0x33: case 0x34: return(inst_inc(code));
    case 0x35: return(inst_dec(code));
    case 0x37: return(inst_scf(code));

    case 0x38: return(inst_jr(code));
    case 0x39: return(inst_add(code));
    case 0x3a: case 0x3e: return(inst_ld(code));
    case 0x3b: case 0x3d: return(inst_dec(code));
    case 0x3c: return(inst_inc(code));
    case 0x3f: return(inst_ccf(code));

    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
    case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
      return(inst_ld(code));

    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
    case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
      return(inst_ld(code));

    case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
    case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
      return(inst_ld(code));

    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
    case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
      return(inst_ld(code));
    case 0x76: 
      return(inst_halt(code));

    case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
      return(inst_add(code));
    case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
      return(inst_adc(code));

    case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
      return(inst_sub(code));
    case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
      return(inst_sbc(code));

    case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
      return(inst_and(code));
    case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
      return(inst_xor(code));

    case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
      return(inst_or(code));
    case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
      return(inst_cp(code));

    case 0xc0: return(inst_ret(code));
    case 0xc1: return(inst_pop(code));
    case 0xc2: case 0xc3: return(inst_jp(code));
    case 0xc4: return(inst_call(code));
    case 0xc5: return(inst_push(code));
    case 0xc6: return(inst_add(code));
    case 0xc7: return(inst_rst(code));

    case 0xc8: case 0xc9: return(inst_ret(code));
    case 0xca: return(inst_jp(code));

      /* CB escapes out to 2 byte opcodes(CB include), opcodes
         to do register bit manipulations */
    /* case 0xcb: illegal, let fall thru */
    case 0xcc: case 0xcd: return(inst_call(code));
    case 0xce: return(inst_adc(code));
    case 0xcf: return(inst_rst(code));


    case 0xd0: return(inst_ret(code));
    case 0xd1: return(inst_pop(code));
    case 0xd2: return(inst_jp(code));
    case 0xd3: return(inst_out(code));
    case 0xd4: return(inst_call(code));
    case 0xd5: return(inst_push(code));
    case 0xd6: return(inst_sub(code));
    case 0xd7: return(inst_rst(code));

    case 0xd8: return(inst_ret(code));
    case 0xd9: return(inst_exx(code));
    case 0xda: return(inst_jp(code));
    case 0xdb: return(inst_in(code));
    case 0xdc: return(inst_call(code));
      /* DD escapes out to 2 to 4 byte opcodes(DD included)
        with a variety of uses.  It can precede the CB escape
        sequence to extend CB codes with IX+immed_byte */
    case 0xdd: return(inst_dd());
    case 0xde: return(inst_sbc(code));
    case 0xdf: return(inst_rst(code));


    case 0xe0: return(inst_ret(code));
    case 0xe1: return(inst_pop(code));
    case 0xe2: return(inst_jp(code));
    case 0xe3: return(inst_ex(code));
    case 0xe4: return(inst_call(code));
    case 0xe5: return(inst_push(code));
    case 0xe6: return(inst_and(code));
    case 0xe7: return(inst_rst(code));

    case 0xe8: return(inst_ret(code));
    case 0xe9: return(inst_jp(code));
    case 0xea: return(inst_jp(code));
    case 0xeb: return(inst_ex(code));
    case 0xec: return(inst_call(code));
    /* case 0xed: illegal, fall thru */
    case 0xee: return(inst_xor(code));
    case 0xef: return(inst_rst(code));


    case 0xf0: return(inst_ret(code));
    case 0xf1: return(inst_pop(code));
    case 0xf2: return(inst_jp(code));
    case 0xf3: return(inst_di(code));
    case 0xf4: return(inst_call(code));
    case 0xf5: return(inst_push(code));
    case 0xf6: return(inst_or(code));
    case 0xf7: return(inst_rst(code));

    case 0xf8: return(inst_ret(code));
    case 0xf9: return(inst_ld(code));
    case 0xfa: return(inst_jp(code));
    case 0xfb: return(inst_ei(code));
    case 0xfc: return(inst_call(code));
    /* case 0xfd: illegal, fall thru */
    case 0xfe: return(inst_cp(code));
    case 0xff: return(inst_rst(code));
    }
  if (PC)
    PC--;
  else
    PC= get_mem_size(MEM_ROM)-1;
  //tick(-clock_per_cycle());
  sim->stop(resINV_INST);
  return(resINV_INST);
}


/* End of z80.src/z80.cc */
