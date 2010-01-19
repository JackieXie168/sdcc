/* gbmch.c */

/*
 *  Copyright (C) 1989-2009  Alan R. Baldwin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Alan R. Baldwin
 * 721 Berkeley St.
 * Kent, Ohio  44240
 */

/* Gameboy mods by Roger Ivie (ivie at cc dot usu dot edu); see gb.h for more info 
 */

#include "asxxxx.h"
#include "gb.h"

char	*cpu	= "Gameboy";
char	*dsft	= "asm";

char	imtab[3] = { 0x46, 0x56, 0x5E };

/*
 * Opcode Cycle Definitions
 */
#define	OPCY_SDP	((char) (0xFF))
#define	OPCY_ERR	((char) (0xFE))

/*	OPCY_NONE	((char) (0x80))	*/
/*	OPCY_MASK	((char) (0x7F))	*/

#define	UN	((char) (OPCY_NONE | 0x00))
#define	P2	((char) (OPCY_NONE | 0x01))

/*
 * GB Opcode Cycle Pages
 */

static char  gbpg1[256] = {
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   4,12, 8, 8, 4, 4, 8, 4,20, 8, 8, 8, 4, 4, 8, 4,
/*10*/   4,12, 8, 8, 4, 4, 8, 4,12, 8, 8, 8, 4, 4, 8, 4,
/*20*/  12,12, 8, 8, 4, 4, 8, 4,12, 8, 8, 8, 4, 4, 8, 4,
/*30*/  12,12, 8, 8,12,12,12, 4,12, 8, 8, 8, 4, 4, 8, 4,
/*40*/   4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
/*50*/   4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
/*60*/   4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
/*70*/   8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4,
/*80*/   4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
/*90*/   4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
/*A0*/   4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
/*B0*/   4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
/*C0*/  20,12,12,16,24,16, 8,16,20,16,12,P2,24,24, 8,16,
/*D0*/  20,12,12,UN,24,16, 8,16,20,16,12,UN,24,UN, 8,16,
/*E0*/  12,12, 8,UN,UN,16, 8,16,16, 4,16,UN,UN,UN, 8,16,
/*F0*/  12,12, 8, 4,UN,16, 8,16, 8, 8,16, 4,UN,UN, 8,16
};

static char  gbpg2[256] = {  /* P2 == CB */
/*--*--* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*--*--* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/*00*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*10*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*20*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*30*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*40*/   8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
/*50*/   8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
/*60*/   8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
/*70*/   8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
/*80*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*90*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*A0*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*B0*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*C0*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*D0*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*E0*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8,
/*F0*/   8, 8, 8, 8, 8, 8,16, 8, 8, 8, 8, 8, 8, 8,16, 8
};

static char *gbPage[2] = {
    gbpg1, gbpg2
};

/*
 * Process a machine op.
 */
VOID
machine(mp)
struct mne *mp;
{
	register int op, t1, t2;
	struct expr e1, e2;
	int rf, v1, v2;
	int d,c,i,th,tl,oops; /* for dealing with .tile */

	clrexpr(&e1);
	clrexpr(&e2);
	op = (int) mp->m_valu;
	rf = mp->m_type;
	switch (rf) {

	case S_INH1:
		outab(op);
		break;

	case S_RET:
		if (more()) {
			if ((v1 = admode(CND)) != 0) {
				outab(op | (v1<<3));
			} else {
				qerr();
			}
		} else {
			outab(0xC9);
		}
		break;

	case S_PUSH:
		if (admode(R16X)) {
			outab(op+0x30);
			break;
		} else
		if ((v1 = admode(R16)) != 0 && (v1 &= 0xFF) != SP) {
			outab(op | (v1<<4));
			break;
		}
		aerr();
		break;

	case S_RST:
		v1 = (int) absexpr();
		if (v1 & ~0x38) {
			aerr();
			v1 = 0;
		}
		outab(op|v1);
		break;

	case S_IM:
		expr(&e1, 0);
		abscheck(&e1);
		if (e1.e_addr > 2) {
			aerr();
			e1.e_addr = 0;
		}
		outab(op);
		outab(imtab[(int) e1.e_addr]);
		break;

	case S_BIT:
		expr(&e1, 0);
		t1 = 0;
		v1 = (int) e1.e_addr;
		if (v1 > 7) {
			++t1;
			v1 &= 0x07;
		}
		op |= (v1<<3);
		comma(1);
		addr(&e2);
		abscheck(&e1);
		if (genop(0xCB, op, &e2, 0) || t1)
			aerr();
		break;

	case S_RL:
		t1 = 0;
		t2 = addr(&e2);
		if (more()) {
			if ((t2 != S_R8) || (e2.e_addr != A))
				++t1;
			comma(1);
			t2 = addr(&e2);
		}
		if (genop(0xCB, op, &e2, 0) || t1)
			aerr();
		break;

	case S_SWAP:
		t1 = 0;
		t2 = addr(&e2);
		if( more() ) {
		 	t1++;
		}
		if (genop(0xCB, op, &e2, 0) || t1)
			aerr();
		break;

	case S_AND:
	case S_SUB:
		t1 = 0;
		t2 = addr(&e2);
		if (more()) {
			if ((t2 != S_R8) || (e2.e_addr != A))
				++t1;
			comma(1);
			t2 = addr(&e2);
		}
		if (genop(0, op, &e2, 1) || t1)
			aerr();
		break;

	case S_ADC:
	case S_SBC:
		t1 = addr(&e1);
		t2 = 0;
		if (more()) {
			comma(1);
			t2 = addr(&e2);
		}
		if (t2 == 0) {
			if (genop(0, op, &e1, 1))
				aerr();
			break;
		}
		if ((t1 == S_R8) && (e1.e_addr == A)) {
			if (genop(0, op, &e2, 1))
				aerr();
			break;
		}
		aerr();
		break;

	case S_ADD:
		t1 = addr(&e1);
		t2 = 0;
		if (more()) {
			comma(1);
			t2 = addr(&e2);
		}
		if (t2 == 0) {
			if (genop(0, op, &e1, 1))
				aerr();
			break;
		}
		if ((t1 == S_R8) && (e1.e_addr == A)) {
			if (genop(0, op, &e2, 1))
				aerr();
			break;
		}
		if ((t1 == S_R16) && (t2 == S_R16)) {
		  if( rf == S_ADD ) {
		    op = 0x09;
		    v1 = (int) e1.e_addr;
		    v2 = (int) e2.e_addr;
		    if( ( v1 == HL ) && ( v2 <= SP ) ) {
		      outab( op | ( v2 << 4 ) );
		      break;
		    }
		  }
		}
		if ((t1 == S_R16) && (t2 == S_IMMED)) {
		  if (rf != S_ADD ) {
		    aerr();
		    break;
		  }
		  if( e1.e_addr == SP ) {
		    outab( 0xE8 );
		    outrb(&e2,0);
		    break;
		  }
		}
		aerr();
		break;


        case S_LDX:

		/* These are loads which increment or decrement HL. The
		 * valid instructions are:
		 *
		 *	ldi a,(hl)	; A <- (HL), HL++
		 *	ldi (hl),a	; (HL) <- A, HL++
		 *	ldd a,(hl)	; A <- (HL), HL--
		 *	ldd (hl),a	; (HL) <- A, HL--
		 *
		 * aliases:
		 *	ld  a,(hli)	; A <- (HL), HL++
		 *	ld  (hli),a	; (HL) <- A, HL++
		 *	ld  a,(hld)	; A <- (HL), HL--
		 *	ld  (hld),a	; (HL) <- A, HL--
		 *
		 * op is output unchanged for "ld? (hl),a" and 0x08 is
		 * added for "ld? a,(hl)".
		 */

		t1 = addr(&e1);
		comma(1);
		t2 = addr(&e2);

		if( ( t1 == S_R8 ) && ( t2 == S_IDHL ) )
		{
		  /* It's "ld? r,(rp)". Make certain r is A. */

		  if( e1.e_addr != A )
		  {
		    aerr();
		    break;
		  }

		  outab( op | 0x08 );
		  break;
		}

		if( ( t1 == S_IDHL ) && ( t2 == S_R8 ) )
		{
		  /* It's ld? (rp),r". Make certain r is A. */

		  if( e2.e_addr != A )
		  {
		    aerr();
		    break;
		  }

		  outab( op );
		  break;
		}

		/* It's some bogus addressing mode. */

		aerr();
		break;

	case S_LD:
		t1 = addr(&e1);
		comma(1);
		t2 = addr(&e2);

		if (t1 == S_R8) {
			v1 = (int) (e1.e_addr<<3);
			if (genop(0, op | v1, &e2, 0) == 0)
				break;
			if (t2 == S_IMMED) {
				outab(v1 | 0x06);
				outrb(&e2,0);
				break;
			}
		}

		v1 = (int) e1.e_addr;
		v2 = (int) e2.e_addr;

		if ((t1 == S_R16) && (t2 == S_IMMED)) {
			outab(0x01 | (v1<<4));
			outrw(&e2, 0);
			break;
		}
		if ((t1 == S_INDM) && (t2 == S_R16)) {
			if (v2 == SP) {
			  outab(0x08);
			  outrw(&e1, 0);
			  break;
			} 
		}
		if ((t1 == S_R8) && (v1 == A) && (t2 == S_INDM)) {
			outab(0xFA);
			outrw(&e2, 0);
			break;
		}
		if ((t1 == S_INDM) && (t2 == S_R8) && (v2 == A)) {
			outab(0xEA);
			outrw(&e1, 0);
			break;
		}
		if ((t2 == S_R8) && (t1 == S_IDHL)) {
			outab(0x70|v2);
			break;
		}
		if ((t2 == S_IMMED) && (t1 == S_IDHL)) {
			outab(0x36);
			outrb(&e2, 0);
			break;
		}
		if ((t1 == S_R8) && (v1 == A)) {
			if ((t2 == S_IDBC) || (t2 == S_IDDE)) {
				outab(0x0A | ((t2-S_INDR)<<4));
				break;
			}
		}
		if ((t2 == S_R8) && (v2 == A)) {
			if ((t1 == S_IDBC) || (t1 == S_IDDE)) {
				outab(0x02 | ((t1-S_INDR)<<4));
				break;
			}
		}
		if ( ( t1 == S_R16 ) && ( t2 == S_R16 ) ) {
		  if( ( v1 == SP ) && ( v2 == HL ) ) {
		    outab( 0xf9 );
		    break;
		  }
		  if( ( v1 == HL ) && ( v2 == SP ) ) {
		    outab( 0xf8 );
		    break;
		  }
		}
		if( ( t1 == S_R8 ) && ( v1 == A ) && ( t2 == S_IDC ) ) {
		    outab( 0xF2 );
		    break;
		}
		if( ( t2 == S_R8 ) && ( v2 == A ) && ( t1 == S_IDC ) ) {
		    outab( 0xE2 );
		    break;
		}
		if( ( t1 == S_R8 ) && ( v1 == A ) && ( t2 == S_IDHLD ) ) {
		    outab( 0x3A );
		    break;
		}
		if( ( t2 == S_R8 ) && ( v2 == A ) && ( t1 == S_IDHLD ) ) {
		    outab( 0x32 );
		    break;
		}
		if( ( t1 == S_R8 ) && ( v1 == A ) && ( t2 == S_IDHLI ) ) {
		    outab( 0x2A );
		    break;
		}
		if( ( t2 == S_R8 ) && ( v2 == A ) && ( t1 == S_IDHLI ) ) {
		    outab( 0x22 );
		    break;
		}
		aerr();
		break;


	case S_DEC:
	case S_INC:
		t1 = addr(&e1);
		v1 = (int) e1.e_addr;
		if (t1 == S_R8) {
			outab(op|(v1<<3));
			break;
		}
		if (t1 == S_IDHL) {
			outab(op|0x30);
			break;
		}
		if (t1 == S_R16) {
			if (rf == S_INC) {
				outab(0x03|(v1<<4));
				break;
			}
			if (rf == S_DEC) {
				outab(0x0B|(v1<<4));
				break;
			}
		}
		aerr();
		break;

	case S_JR:
		if ((v1 = admode(CND)) != 0 ) {
			if ((v1 &= 0xFF) <= 0x03) {
				op += (v1+1)<<3;
			} else {
				aerr();
			}
			comma(1);
		}
		expr(&e2, 0);
		outab(op);
		if (mchpcr(&e2)) {
			v2 = (int) (e2.e_addr - dot.s_addr - 1);
			if ((v2 < -128) || (v2 > 127))
				aerr();
			outab(v2);
		} else {
			outrb(&e2, R_PCR);
		}
		if (e2.e_mode != S_USER)
			rerr();
		break;

	case S_CALL:
		if ((v1 = admode(CND)) != 0) {
			op |= (v1&0xFF)<<3;
			comma(1);
		} else {
			op = 0xCD;
		}
		expr(&e1, 0);
		outab(op);
		outrw(&e1, 0);
		break;

	case S_JP:
		if ((v1 = admode(CND)) != 0) {
			op |= (v1&0xFF)<<3;
			comma(1);
			expr(&e1, 0);
			outab(op);
			outrw(&e1, 0);
			break;
		}
		t1 = addr(&e1);
		if (t1 == S_USER) {
			outab(0xC3);
			outrw(&e1, 0);
			break;
		}
		if ((e1.e_addr == 0) && (t1 == S_IDHL)) {
			outab(0xE9);
			break;
		}
		aerr();
		break;

	case S_LDH:
		/* These instructions interact with the "zero page",
		 * which on Gameboy is 0xff00 - 0xffff.
		 *
		 * The valid instructions are:
		 *
		 *	ldh a,(n)	; A <- (0xFF00 + n)
		 * alias:  in a,(n)	; A <- (0xFF00 + n)
		 *
		 *	ldh (n),a	; (0xFF00 + n) <- A
		 * alias:  out (n),a	; (0xFF00 + n) <- A
		 *
		 *	ldh a,(c)	; A <- (0xFF00 + c)
		 * alias:  ld a,(c)	; A <- (0xFF00 + c)
		 * alias:  in a,(c)	; A <- (0xFF00 + c)
		 *
		 *	ldh (c),a	; (0xFF00 + c) <- A
		 * alias:  ld  (c),a	; (0xFF00 + c) <- A
		 * alias:  out (c),a	; (0xFF00 + c) <- A
		 */

		t1 = addr( &e1 );
		comma(1);
		t2 = addr( &e2 );
		v1 = (int) e1.e_addr;
		v2 = (int) e2.e_addr;

		if( ( t1 == S_R8 ) && ( v1 == A ) && ( t2 == S_INDM ) ) {
		    outab( 0xF0 );
		    outab( v2 );
		    break;
		}
		if( ( t2 == S_R8 ) && ( v2 == A ) && ( t1 == S_INDM ) ) {
		    outab( 0xE0 );
		    outab( v1 );
		    break;
		}
		if( ( t1 == S_R8 ) && ( v1 == A ) && ( t2 == S_IDC ) ) {
		    outab( 0xF2 );
		    break;
		}
		if( ( t2 == S_R8 ) && ( v2 == A ) && ( t1 == S_IDC ) ) {
		    outab( 0xE2 );
		    break;
		}
		aerr();
		break;


	case S_IN:
		/* These instructions interact with the "zero page",
		 * which on Gameboy is 0xff00 - 0xffff. Since all the
		 * I/O is in this range, I'm keeping the in and out
		 * mnemonics.
		 *
		 * The valid instructions are:
		 *
		 *	in a,(n)	; A <- (0xFF00 + n)
		 * alias:  ldh a,(n)	; A <- (0xFF00 + n)
		 *
		 *	in a,(c)	; A <- (0xFF00 + c)
		 * alias:  ldh a,(c)	; A <- (0xFF00 + c)
		 * alias:  ld  a,(c)	; A <- (0xFF00 + c)
		 */

		t1 = addr( &e1 );
		comma(1);
		t2 = addr( &e2 );
		v1 = (int) e1.e_addr;
		v2 = (int) e2.e_addr;

		if( ( t1 == S_R8 ) && ( v1 == A ) ) {
		  if( t2 == S_IDC ) {
		    outab( 0xF2 );
		    break;
		  }
		  if( t2 == S_INDM ) {
		    outab( 0xF0 );
		    outab( v2 );
		    break;
		  }
		}
		aerr();
		break;

	case S_OUT:
		/* These instructions interact with the "zero page",
		 * which on Gameboy is 0xff00 - 0xffff. Since all the
		 * I/O is in this range, I'm keeping the in and out
		 * mnemonics.
		 *
		 * The valid instructions are:
		 *
		 *	out (n),a	; (0xFF00 + n) <- A
		 * alias:  ldh (n),a	; (0xFF00 + n) <- A
		 *
		 *	out (c),a	; (0xFF00 + C) <- A
		 * alias:  ldh (c),a)	; (0xFF00 + C) <- A
		 * alias:  ld  (c),a)	; (0xFF00 + C) <- A
		 */

		t1 = addr( &e1 );
		comma(1);
		t2 = addr( &e2 );
		v1 = (int) e1.e_addr;
		v2 = (int) e2.e_addr;

		if( ( t2 == S_R8 ) && ( v2 == A ) ) {
		  if( t1 == S_IDC ) {
		    outab( 0xE2 );
		    break;
		  }
		  if( t1 == S_INDM ) {
		    outab( 0xE0 );
		    outab( v1 );
		    break;
		  }
		}
		aerr();
		break;

	case S_TILE:
		/* The .tile pseudo-op. It generates two bytes from
		 * an 8-character ASCII string to represent a line of
		 * pixels in a Gameboy character.
		 */

		/* Like .ASCII, the first character after .TILE is used
		 * as the string delimiter. Get it.
		 */

		if ((d = getnb()) == '^') {
		  d = get();
		}
		if(d == '\0' ) {
		  qerr();
		}

		/* .tile deals with chunks of 8 characters. We need to
		 * generate an error if we get fewer than 8 characters in
		 * chunk, so we have a modulo-8 counter to keep track of
		 * how many characters we've processed. We also need to
		 * generate an error if we see a character we don't 
		 * recognize; this can be done either with a goto
		 * or an 'oops' flag. Although I normally lean towards
		 * goto implementations, since I didn't design _all_
		 * of this code that would be ugly; so we need to
		 * initialize the oops flag. We also need to initialize
		 * the variables we'll be using to collect the bits.
		 */

		i = 0;
		c = get(); /* Prime the pump */
		th = 0;
		tl = 0;
		oops = 0;

		/* Process characters until we find one we don't
		 * understand, encounter the delimiter, or run into
		 * the end of line.
		 */

		while( ( oops == 0 ) && ( c != d ) && ( c != 0 ) ) {

		  th = th << 1;
		  tl = tl << 1;

		  switch( c ) {
		    case ' ':
		    case '0':
		      break;
		    case '.':
		    case '1':
		      tl++; 
		      break;
		    case '+':
		    case '2':
		      th++; 
		      break;
		    case '*':
		    case '3':
		      th++; 
		      tl++; 
		      break;
		    default:  
		      oops = 1;
		      break;
		  }

		  c = get();
		  i++;

		  /* Spit out the tile data.
		   */

		  if( i == 8 ) {
		    outab( tl );
		    outab( th );
		    i = 0;
		    tl = 0;
		    th = 0;
		  }
		}

		/* Figure out whether we left the while loop early. If so,
		 * complain.
		 */

		if( i != 0 ) {
		  aerr();
		  break;
		}

		/* Make sure we have the delimiter next. This should
		 * already have been fetched by the end of the while().
		 * What this primarily buys us that the check for
		 * the modulo-8 counter does not is detecting a string
		 * which ended with an end-of-line rather than a
		 * delimiter.
		 */

		if( c != d ) {
		  aerr();
		  break;
		}

		break;
		

	default:
		opcycles = OPCY_ERR;
		err('o');
		break;
	}

	if (opcycles == OPCY_NONE) {
		opcycles = gbpg1[cb[0] & 0xFF];
		if ((opcycles & OPCY_NONE) && (opcycles & OPCY_MASK)) {
			opcycles = gbPage[opcycles & OPCY_MASK][cb[1] & 0xFF];
		}
	}
}

/*
 * general addressing evaluation
 * return(0) if general addressing mode output, else
 * return(esp->e_mode)
 */
int
genop(pop, op, esp, f)
int pop, op;
struct expr *esp;
int f;
{
	int t1;
	if ((t1 = esp->e_mode) == S_R8) {
		if (pop)
			outab(pop);
		outab(op|esp->e_addr);
		return(0);
	}
	if (t1 == S_IDHL) {
		if (pop)
			outab(pop);
		outab(op|0x06);
		return(0);
	}
	if ((t1 == S_IMMED) && (f)) {
		if (pop)
			outab(pop);
		outab(op|0x46);
		outrb(esp,0);
		return(0);
	}
	return(t1);
}

/*
 * Branch/Jump PCR Mode Check
 */
int
mchpcr(esp)
struct expr *esp;
{
	if (esp->e_base.e_ap == dot.s_area) {
		return(1);
	}
	if (esp->e_flag==0 && esp->e_base.e_ap==NULL) {
		/*
		 * Absolute Destination
		 *
		 * Use the global symbol '.__.ABS.'
		 * of value zero and force the assembler
		 * to use this absolute constant as the
		 * base value for the relocation.
		 */
		esp->e_flag = 1;
		esp->e_base.e_sp = &sym[1];
	}
	return(0);
}

/*
 * Machine dependent initialization
 */
VOID
minit()
{
	/*
	 * Byte Order
	 */
	hilo = 0;
}

