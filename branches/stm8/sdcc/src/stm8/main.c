/*-------------------------------------------------------------------------
  main.c - STM8 specific definitions.

  Philipp Klaus Krause <pkk@spth.de> 2012-2013
  Valentin Dudouyt <valentin.dudouyt@gmail.com> 2013

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/

#include "common.h"

#include "ralloc.h"
#include "gen.h"

static char stm8_defaultRules[] = {
#include "peeph.rul"
};


static char *stm8_keywords[] = {
  "at",
  "critical",
  NULL
};

static void
stm8_init (void)
{
  // fprintf(stderr, "stm8_init\n");
  asm_addTree (&asm_asxxxx_mapping);
}


static void
stm8_reset_regparm (void)
{
}

static int
stm8_reg_parm (sym_link * l, bool reentrant)
{
  return FALSE;
}

static bool
stm8_parseOptions (int *pargc, char **argv, int *i)
{
  return FALSE;
}

static void
stm8_finaliseOptions (void)
{
  port->mem.default_local_map = data;
  port->mem.default_globl_map = data;
}

static void
stm8_setDefaultOptions (void)
{
  options.nopeep = 0;
  options.stackAuto = 1;
  options.intlong_rent = 1;
  options.float_rent = 1;
  options.noRegParams = 0;
  options.noOptsdccInAsm = 1;
  options.code_loc = 0x8000;

  options.out_fmt = 'i';        /* Default output format is ihx */
}

static const char *
stm8_getRegName (const struct reg_info *reg)
{
  if (reg)
    return reg->name;
  return "err";
}

void
stm8_genInitStartup(FILE * of)
{
  return;
}

int
stm8_genIVT(struct dbuf_s * oBuf, symbol ** intTable, int intCount)
{
  int i;
  dbuf_tprintf (oBuf, "\tint _main ;int0\n"); // int0 (Reset)
  for(i = 1; i < 32; i++)
  {
    dbuf_tprintf (oBuf, "\tint 0x0000 ;int%d\n", i); // int<n>
  }
  return TRUE;
}

static bool
_hasNativeMulFor (iCode * ic, sym_link * left, sym_link * right)
{
  sym_link *test = NULL;
  int result_size = IS_SYMOP(IC_RESULT(ic)) ? getSize(OP_SYM_TYPE(IC_RESULT(ic))) : 4;

  if (ic->op != '*')
    {
      return FALSE;
    }

  return (result_size == 1 || IS_CHAR (left) && IS_CHAR (right) && result_size == 2);
}

/* Indicate which extended bit operations this port supports */
static bool
hasExtBitOp (int op, int size)
{
  return FALSE; /* None for now - some will be implemented later. */
}

/** $1 is always the basename.
    $2 is always the output file.
    $3 varies
    $l is the list of extra options that should be there somewhere...
    MUST be terminated with a NULL.
*/
static const char *_linkCmd[] =
{
  "sdld", "-nf", "\"$1\"", NULL
};

/* $3 is replaced by assembler.debug_opts resp. port->assembler.plain_opts */
static const char *stm8AsmCmd[] =
{
  "sdasstm8", "$l", "$3", "\"$1.asm\"", NULL
};

static const char *const _crt[] = { "crt0.rel", NULL, };

PORT stm8_port = {
  TARGET_ID_STM8,
  "stm8",
  "STM8",                       /* Target name */
  NULL,                         /* Processor name */
  {
   glue,
   TRUE,                        /* We want stm8_genIVT to be triggered */
   NO_MODEL,
   NO_MODEL,
   NULL,                        /* model == target */
   },
  {                             /* Assembler */
   stm8AsmCmd,
   NULL,
   "-plosgffwy",                /* Options with debug */
   "-plosgffw",                 /* Options without debug */
   0,
   ".asm"},
  {                             /* Linker */
   _linkCmd,
   NULL,                        //LINKCMD,
   NULL,
   ".rel",
   1,
   _crt,                        /* crt */
   NULL,                        /* libs */
   },
  {                             /* Peephole optimizer */
   stm8_defaultRules,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   },
  {
   /* Sizes: char, short, int, long, long long, ptr, fptr, gptr, bit, float, max */
   1, 2, 2, 4, 8, 2, 2, 2, 1, 4, 4},
  /* tags for generic pointers */
  {0x00, 0x40, 0x60, 0x80},     /* far, near, xstack, code */
  {
   "XSEG",
   "STACK",
   "CODE",
   "DATA",
   NULL,                        /* idata */
   NULL,                        /* pdata */
   NULL,                        /* xdata */
   NULL,                        /* bit */
   "RSEG (ABS)",
   "GSINIT",                    /* static initialization */
   NULL,                        /* overlay */
   "GSFINAL",
   "HOME",
   NULL,                        /* xidata */
   NULL,                        /* xinit */
   NULL,                        /* const_name */
   "CABS (ABS)",                /* cabs_name */
   "DABS (ABS)",                /* xabs_name */
   NULL,                        /* iabs_name */
   "INITIALIZED",               /* name of segment for initialized variables */
   "INITIALIZER",               /* name of segment for copies of initialized variables in code space */
   NULL,
   NULL,
   1                            /* CODE  is read-only */
   },
  {NULL, NULL},
  {
   -1, 0, 7, 2, 0, 2},          /* stack information */
  /* Use more fine-grained control for multiplication / division. */
  {
   0, -1},
  {
   stm8_emitDebuggerSymbol},
  {
   0,                           /* maxCount */
   3,                           /* sizeofElement */
   /* The rest of these costs are bogus. They approximate */
   /* the behavior of src/SDCCicode.c 1.207 and earlier.  */
   {4, 4, 4},                   /* sizeofMatchJump[] */
   {0, 0, 0},                   /* sizeofRangeCompare[] */
   0,                           /* sizeofSubtract */
   3,                           /* sizeofDispatch */
   },
  "_",
  stm8_init,
  stm8_parseOptions,
  NULL,
  NULL,
  stm8_finaliseOptions,
  stm8_setDefaultOptions,
  stm8_assignRegisters,
  stm8_getRegName,
  stm8_keywords,
  0,                            /* no assembler preamble */
  NULL,                         /* no genAssemblerEnd */
  stm8_genIVT,
  0,                            /* no genXINIT code */
  stm8_genInitStartup,                         /* genInitStartup */
  stm8_reset_regparm,
  stm8_reg_parm,
  NULL,                         /* process_pragma */
  NULL,                         /* getMangledFunctionName */
  _hasNativeMulFor,             /* hasNativeMulFor */
  hasExtBitOp,                  /* hasExtBitOp */
  NULL,                         /* oclsExpense */
  TRUE,
  TRUE,                         /* little endian */
  0,                            /* leave lt */
  0,                            /* leave gt */
  1,                            /* transform <= to ! > */
  1,                            /* transform >= to ! < */
  1,                            /* transform != to !(a == b) */
  0,                            /* leave == */
  FALSE,                        /* Array initializer support. */
  0,                            /* no CSE cost estimation yet */
  NULL,                         /* builtin functions */
  GPOINTER,                     /* treat unqualified pointers as "generic" pointers */
  1,                            /* reset labelKey to 1 */
  1,                            /* globals & local statics allowed */
  5,                            /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */
  PORT_MAGIC
};

