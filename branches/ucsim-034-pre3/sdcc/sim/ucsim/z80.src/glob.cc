/*
 * Simulator of microcontrollers (glob.cc)
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

#include <stdio.h>

#include "stypes.h"


/* 
%d - byte jump 
%w - 2-byte jump or imm. value
%b - byte imm. value
  */
/*  uint  code, mask;  char  branch;  uchar length;  char  *mnemonic; */
struct dis_entry disass_z80[]= {
  { 0x0000, 0x00ff, ' ', 1, "nop" },
  { 0x0001, 0x00ff, ' ', 3, "ld bc,%w" },
  { 0x0002, 0x00ff, ' ', 1, "ld (bc),a" },
  { 0x0003, 0x00ff, ' ', 1, "inc bc" },
  { 0x0004, 0x00ff, ' ', 1, "inc b" },
  { 0x0005, 0x00ff, ' ', 1, "dec b" },
  { 0x0006, 0x00ff, ' ', 2, "ld b,%b" },
  { 0x0007, 0x00ff, ' ', 1, "rlca" },

  { 0x0008, 0x00ff, ' ', 1, "ex af,af'" },
  { 0x0009, 0x00ff, ' ', 1, "add hl,bc" },
  { 0x000a, 0x00ff, ' ', 1, "ld a,(bc)" },
  { 0x000b, 0x00ff, ' ', 1, "dec bc" },
  { 0x000c, 0x00ff, ' ', 1, "inc c" },
  { 0x000d, 0x00ff, ' ', 1, "dec c" },
  { 0x000e, 0x00ff, ' ', 2, "ld c,%b" },
  { 0x000f, 0x00ff, ' ', 1, "rrca" },

  { 0x0010, 0x00ff, 'R', 2, "djnz %d" },
  { 0x0011, 0x00ff, ' ', 3, "ld de,%w" },
  { 0x0012, 0x00ff, ' ', 1, "ld de,a" },
  { 0x0013, 0x00ff, ' ', 1, "inc de" },
  { 0x0014, 0x00ff, ' ', 1, "inc d" },
  { 0x0015, 0x00ff, ' ', 1, "dec d" },
  { 0x0016, 0x00ff, ' ', 2, "ld d,%b" },
  { 0x0017, 0x00ff, ' ', 1, "rla" },

  { 0x0018, 0x00ff, 'R', 2, "jr %d" },
  { 0x0019, 0x00ff, ' ', 3, "add hl,de" },
  { 0x001a, 0x00ff, ' ', 1, "ld a,de" },
  { 0x001b, 0x00ff, ' ', 1, "dec de" },
  { 0x001c, 0x00ff, ' ', 1, "inc e" },
  { 0x001d, 0x00ff, ' ', 1, "dec e" },
  { 0x001e, 0x00ff, ' ', 2, "ld e" },
  { 0x001f, 0x00ff, ' ', 1, "rra" },

  { 0x0020, 0x00ff, 'R', 2, "jr nz,%d" },
  { 0x0021, 0x00ff, ' ', 3, "ld hl,%w" },
  { 0x0022, 0x00ff, ' ', 3, "ld (%w),hl" },
  { 0x0023, 0x00ff, ' ', 1, "inc hl" },
  { 0x0024, 0x00ff, ' ', 1, "inc h" },
  { 0x0025, 0x00ff, ' ', 1, "dec h" },
  { 0x0026, 0x00ff, ' ', 2, "ld h,%b" },
  { 0x0027, 0x00ff, ' ', 1, "daa" },

  { 0x0028, 0x00ff, 'R', 2, "jr z,%d" },
  { 0x0029, 0x00ff, ' ', 1, "add hl,hl" },
  { 0x002a, 0x00ff, ' ', 3, "ld hl,(%w)" },
  { 0x002b, 0x00ff, ' ', 1, "dec hl" },
  { 0x002c, 0x00ff, ' ', 1, "inc l" },
  { 0x002d, 0x00ff, ' ', 1, "dec l" },
  { 0x002e, 0x00ff, ' ', 2, "ld l, %b" },
  { 0x002f, 0x00ff, ' ', 1, "cpl" },

  { 0x0030, 0x00ff, 'R', 2, "jr nc,%d" },
  { 0x0031, 0x00ff, ' ', 3, "ld sp,%w" },
  { 0x0032, 0x00ff, ' ', 3, "ld (%w),a" },
  { 0x0033, 0x00ff, ' ', 1, "inc sp" },
  { 0x0034, 0x00ff, ' ', 1, "inc hl" },
  { 0x0035, 0x00ff, ' ', 1, "dec hl" },
  { 0x0036, 0x00ff, ' ', 2, "ld (hl),nn" },
  { 0x0037, 0x00ff, ' ', 1, "scf" },

  { 0x0038, 0x00ff, 'R', 2, "jr c,%d" },
  { 0x0039, 0x00ff, ' ', 1, "add hl,sp" },
  { 0x003a, 0x00ff, ' ', 3, "ld a,(%w)" },
  { 0x003b, 0x00ff, ' ', 1, "dec sp" },
  { 0x003c, 0x00ff, ' ', 1, "inc a" },
  { 0x003d, 0x00ff, ' ', 1, "dec a" },
  { 0x003e, 0x00ff, ' ', 2, "ld a" },
  { 0x003f, 0x00ff, ' ', 1, "ccf" },

  { 0x0040, 0x00ff, ' ', 1, "ld b,b" },
  { 0x0041, 0x00ff, ' ', 1, "ld b,c" },
  { 0x0042, 0x00ff, ' ', 1, "ld b,d" },
  { 0x0043, 0x00ff, ' ', 1, "ld b,e" },
  { 0x0044, 0x00ff, ' ', 1, "ld b,h" },
  { 0x0045, 0x00ff, ' ', 1, "ld b,l" },
  { 0x0046, 0x00ff, ' ', 1, "ld b,(hl)" },
  { 0x0047, 0x00ff, ' ', 1, "ld b,a" },

  { 0x0048, 0x00ff, ' ', 1, "ld c,b" },
  { 0x0049, 0x00ff, ' ', 1, "ld c,c" },
  { 0x004a, 0x00ff, ' ', 1, "ld c,d" },
  { 0x004b, 0x00ff, ' ', 1, "ld c,e" },
  { 0x004c, 0x00ff, ' ', 1, "ld c,h" },
  { 0x004d, 0x00ff, ' ', 1, "ld c,l" },
  { 0x004e, 0x00ff, ' ', 1, "ld c,(hl)" },
  { 0x004f, 0x00ff, ' ', 1, "ld c,a" },

  { 0x0050, 0x00ff, ' ', 1, "ld d,b" },
  { 0x0051, 0x00ff, ' ', 1, "ld d,c" },
  { 0x0052, 0x00ff, ' ', 1, "ld d,d" },
  { 0x0053, 0x00ff, ' ', 1, "ld d,e" },
  { 0x0054, 0x00ff, ' ', 1, "ld d,h" },
  { 0x0055, 0x00ff, ' ', 1, "ld d,l" },
  { 0x0056, 0x00ff, ' ', 1, "ld d,(hl)" },
  { 0x0057, 0x00ff, ' ', 1, "ld d,a" },

  { 0x0058, 0x00ff, ' ', 1, "ld e,b" },
  { 0x0059, 0x00ff, ' ', 1, "ld e,c" },
  { 0x005a, 0x00ff, ' ', 1, "ld e,d" },
  { 0x005b, 0x00ff, ' ', 1, "ld e,e" },
  { 0x005c, 0x00ff, ' ', 1, "ld e,h" },
  { 0x005d, 0x00ff, ' ', 1, "ld e,l" },
  { 0x005e, 0x00ff, ' ', 1, "ld e,(hl)" },
  { 0x005f, 0x00ff, ' ', 1, "ld e,a" },

  { 0x0060, 0x00ff, ' ', 1, "ld h,b" },
  { 0x0061, 0x00ff, ' ', 1, "ld h,c" },
  { 0x0062, 0x00ff, ' ', 1, "ld h,d" },
  { 0x0063, 0x00ff, ' ', 1, "ld h,e" },
  { 0x0064, 0x00ff, ' ', 1, "ld h,h" },
  { 0x0065, 0x00ff, ' ', 1, "ld h,l" },
  { 0x0066, 0x00ff, ' ', 1, "ld h,(hl)" },
  { 0x0067, 0x00ff, ' ', 1, "ld h,a" },

  { 0x0068, 0x00ff, ' ', 1, "ld l,b" },
  { 0x0069, 0x00ff, ' ', 1, "ld l,c" },
  { 0x006a, 0x00ff, ' ', 1, "ld l,d" },
  { 0x006b, 0x00ff, ' ', 1, "ld l,e" },
  { 0x006c, 0x00ff, ' ', 1, "ld l,h" },
  { 0x006d, 0x00ff, ' ', 1, "ld l,l" },
  { 0x006e, 0x00ff, ' ', 1, "ld l,(hl)" },
  { 0x006f, 0x00ff, ' ', 1, "ld l,a" },

  { 0x0070, 0x00ff, ' ', 1, "ld (hl),b" },
  { 0x0071, 0x00ff, ' ', 1, "ld (hl),c" },
  { 0x0072, 0x00ff, ' ', 1, "ld (hl),d" },
  { 0x0073, 0x00ff, ' ', 1, "ld (hl),e" },
  { 0x0074, 0x00ff, ' ', 1, "ld (hl),h" },
  { 0x0075, 0x00ff, ' ', 1, "ld (hl),l" },
  { 0x0076, 0x00ff, ' ', 1, "halt" },
  { 0x0077, 0x00ff, ' ', 1, "ld (hl),a" },

  { 0x0078, 0x00ff, ' ', 1, "ld a,b" },
  { 0x0079, 0x00ff, ' ', 1, "ld a,c" },
  { 0x007a, 0x00ff, ' ', 1, "ld a,d" },
  { 0x007b, 0x00ff, ' ', 1, "ld a,e" },
  { 0x007c, 0x00ff, ' ', 1, "ld a,h" },
  { 0x007d, 0x00ff, ' ', 1, "ld a,l" },
  { 0x007e, 0x00ff, ' ', 1, "ld a,(hl)" },
  { 0x007f, 0x00ff, ' ', 1, "ld a,a" },

  { 0x0080, 0x00ff, ' ', 1, "add a,b" },
  { 0x0081, 0x00ff, ' ', 1, "add a,c" },
  { 0x0082, 0x00ff, ' ', 1, "add a,d" },
  { 0x0083, 0x00ff, ' ', 1, "add a,e" },
  { 0x0084, 0x00ff, ' ', 1, "add a,h" },
  { 0x0085, 0x00ff, ' ', 1, "add a,l" },
  { 0x0086, 0x00ff, ' ', 1, "add a,(hl)" },
  { 0x0087, 0x00ff, ' ', 1, "add a,a" },

  { 0x0088, 0x00ff, ' ', 1, "adc a,b" },
  { 0x0089, 0x00ff, ' ', 1, "adc a,c" },
  { 0x008a, 0x00ff, ' ', 1, "adc a,d" },
  { 0x008b, 0x00ff, ' ', 1, "adc a,e" },
  { 0x008c, 0x00ff, ' ', 1, "adc a,h" },
  { 0x008d, 0x00ff, ' ', 1, "adc a,l" },
  { 0x008e, 0x00ff, ' ', 1, "adc a,(hl)" },
  { 0x008f, 0x00ff, ' ', 1, "adc a,a" },

  { 0x0090, 0x00ff, ' ', 1, "sub a,b"},
  { 0x0091, 0x00ff, ' ', 1, "sub a,c"},
  { 0x0092, 0x00ff, ' ', 1, "sub a,d"},
  { 0x0093, 0x00ff, ' ', 1, "sub a,e"},
  { 0x0094, 0x00ff, ' ', 1, "sub a,h"},
  { 0x0095, 0x00ff, ' ', 1, "sub a,l"},
  { 0x0096, 0x00ff, ' ', 1, "sub a,(hl)"},
  { 0x0097, 0x00ff, ' ', 1, "sub a,a"},

  { 0x0098, 0x00ff, ' ', 1, "sbc a,b" },
  { 0x0099, 0x00ff, ' ', 1, "sbc a,c" },
  { 0x009a, 0x00ff, ' ', 1, "sbc a,d" },
  { 0x009b, 0x00ff, ' ', 1, "sbc a,e" },
  { 0x009c, 0x00ff, ' ', 1, "sbc a,h" },
  { 0x009d, 0x00ff, ' ', 1, "sbc a,l" },
  { 0x009e, 0x00ff, ' ', 1, "sbc a,(hl)" },
  { 0x009f, 0x00ff, ' ', 1, "sbc a,a" },

  { 0x00a0, 0x00ff, ' ', 1, "and b" },
  { 0x00a1, 0x00ff, ' ', 1, "and c" },
  { 0x00a2, 0x00ff, ' ', 1, "and d" },
  { 0x00a3, 0x00ff, ' ', 1, "and e" },
  { 0x00a4, 0x00ff, ' ', 1, "and h" },
  { 0x00a5, 0x00ff, ' ', 1, "and l" },
  { 0x00a6, 0x00ff, ' ', 1, "and (hl)" },
  { 0x00a7, 0x00ff, ' ', 1, "and a" },

  { 0x00a8, 0x00ff, ' ', 1, "xor b" },
  { 0x00a9, 0x00ff, ' ', 1, "xor c" },
  { 0x00aa, 0x00ff, ' ', 1, "xor d" },
  { 0x00ab, 0x00ff, ' ', 1, "xor e" },
  { 0x00ac, 0x00ff, ' ', 1, "xor h" },
  { 0x00ad, 0x00ff, ' ', 1, "xor l" },
  { 0x00ae, 0x00ff, ' ', 1, "xor (hl)" },
  { 0x00af, 0x00ff, ' ', 1, "xor a" },

  { 0x00b0, 0x00ff, ' ', 1, "or b" },
  { 0x00b1, 0x00ff, ' ', 1, "or c" },
  { 0x00b2, 0x00ff, ' ', 1, "or d" },
  { 0x00b3, 0x00ff, ' ', 1, "or e" },
  { 0x00b4, 0x00ff, ' ', 1, "or h" },
  { 0x00b5, 0x00ff, ' ', 1, "or l" },
  { 0x00b6, 0x00ff, ' ', 1, "or (hl)" },
  { 0x00b7, 0x00ff, ' ', 1, "or a" },

  { 0x00b8, 0x00ff, ' ', 1, "cp a" },
  { 0x00b9, 0x00ff, ' ', 1, "cp b" },
  { 0x00ba, 0x00ff, ' ', 1, "cp c" },
  { 0x00bb, 0x00ff, ' ', 1, "cp d" },
  { 0x00bc, 0x00ff, ' ', 1, "cp e" },
  { 0x00bd, 0x00ff, ' ', 1, "cp h" },
  { 0x00be, 0x00ff, ' ', 1, "cp (hl)" },
  { 0x00bf, 0x00ff, ' ', 1, "cp a" },

  { 0x00c0, 0x00ff, ' ', 1, "ret nz" },
  { 0x00c1, 0x00ff, ' ', 1, "pop bc" },
  { 0x00c2, 0x00ff, 'A', 3, "jp nz, %w" },
  { 0x00c3, 0x00ff, 'A', 3, "jp %w" },
  { 0x00c4, 0x00ff, 'A', 3, "call nz,%w" },
  { 0x00c5, 0x00ff, ' ', 1, "push bc" },
  { 0x00c6, 0x00ff, ' ', 2, "add a,%b" },
  { 0x00c7, 0x00ff, ' ', 1, "rst 0" },

  { 0x00c8, 0x00ff, ' ', 1, "ret z" },
  { 0x00c9, 0x00ff, ' ', 1, "ret" },
  { 0x00ca, 0x00ff, 'A', 3, "jp z,%w" },
  { 0x00cb, 0x00ff, ' ', 2, "?cb?" }, /* ESC code to lots of op-codes, all 2-byte */
  { 0x00cc, 0x00ff, 'A', 3, "call z,%w" },
  { 0x00cd, 0x00ff, 'A', 3, "call %w" },
  { 0x00ce, 0x00ff, ' ', 2, "adc A,%b" },
  { 0x00cf, 0x00ff, ' ', 1, "rst 8" },

  { 0x00d0, 0x00ff, ' ', 1, "ret nc" },
  { 0x00d1, 0x00ff, ' ', 1, "pop de" },
  { 0x00d2, 0x00ff, 'A', 3, "jp nc,%w" },
  { 0x00d3, 0x00ff, ' ', 2, "out (%b),a" },
  { 0x00d4, 0x00ff, 'A', 3, "call nc,%w" },
  { 0x00d5, 0x00ff, ' ', 1, "push de" },
  { 0x00d6, 0x00ff, ' ', 2, "sub %b" },
  { 0x00d7, 0x00ff, ' ', 1, "rst 10H" },

  { 0x00d8, 0x00ff, ' ', 1, "ret c" },
  { 0x00d9, 0x00ff, ' ', 1, "exx" },
  { 0x00da, 0x00ff, 'A', 3, "jp c,%w" },
  { 0x00db, 0x00ff, ' ', 2, "in a,(%b)" },
  { 0x00dc, 0x00ff, 'A', 3, "call c,%w" },
  { 0x00dd, 0x00ff, ' ', 2, "?dd?" },  /* ESC codes,about 284, vary lengths, IX centric */
  { 0x00de, 0x00ff, ' ', 2, "sbc a,%b" },
  { 0x00df, 0x00ff, ' ', 1, "rst 18H" },

  { 0x00e0, 0x00ff, ' ', 1, "ret po" },
  { 0x00e1, 0x00ff, ' ', 1, "pop hl" },
  { 0x00e2, 0x00ff, 'A', 3, "jp po,%w" },
  { 0x00e3, 0x00ff, ' ', 1, "ex (sp),hl" },
  { 0x00e4, 0x00ff, 'A', 3, "call po,%w" },
  { 0x00e5, 0x00ff, ' ', 1, "push hl" },
  { 0x00e6, 0x00ff, ' ', 2, "and %b" },
  { 0x00e7, 0x00ff, ' ', 1, "rst 20H" },

  { 0x00e8, 0x00ff, ' ', 1, "ret pe" },
  { 0x00e9, 0x00ff, 'A', 1, "jp (hl)" },
  { 0x00ea, 0x00ff, 'A', 3, "jp pe,%w" },
  { 0x00eb, 0x00ff, ' ', 1, "ex de,hl" },
  { 0x00ec, 0x00ff, ' ', 3, "call pe, %w" },
  { 0x00ed, 0x00ff, ' ', 2, "?ed?" },  /* ESC code to about 80 opcodes of various lengths */
  { 0x00ee, 0x00ff, ' ', 2, "xor %b" },
  { 0x00ef, 0x00ff, ' ', 1, "rst 28H" },

  { 0x00f0, 0x00ff, ' ', 1, "ret p" },
  { 0x00f1, 0x00ff, ' ', 1, "pop af" },
  { 0x00f2, 0x00ff, 'A', 3, "jp p,%w" },
  { 0x00f3, 0x00ff, ' ', 1, "di" },
  { 0x00f4, 0x00ff, 'A', 3, "call p,%w" },
  { 0x00f5, 0x00ff, ' ', 1, "push af" },
  { 0x00f6, 0x00ff, ' ', 2, "or %b" },
  { 0x00f7, 0x00ff, ' ', 1, "rst 30H" },

  { 0x00f8, 0x00ff, ' ', 1, "ret m" },
  { 0x00f9, 0x00ff, ' ', 1, "ld sp,hl" },
  { 0x00fa, 0x00ff, ' ', 3, "jp m,%w" },
  { 0x00fb, 0x00ff, ' ', 1, "ei" },
  { 0x00fc, 0x00ff, 'A', 3, "call m,%w" },
  { 0x00fd, 0x00ff, ' ', 1, "?fd?" }, /* ESC codes,about 284, vary lengths, IY centric */
  { 0x00fe, 0x00ff, ' ', 2, "cp %b" },
  { 0x00ff, 0x00ff, ' ', 1, "rst 38H" },

  { 0, 0, 0, 0, NULL }
};


/* End of z80.src/glob.cc */
