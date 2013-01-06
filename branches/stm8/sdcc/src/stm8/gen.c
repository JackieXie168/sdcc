/*-------------------------------------------------------------------------
  gen.c - code generator for STM8.

  Copyright (C) 2012, Philipp Klaus Krause pkk@spth.de, philipp@informatik.uni-frankfurt.de)
                2011, Vaclav Peroutka

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
-------------------------------------------------------------------------*/

#include "ralloc.h"
#include "gen.h"

static bool regalloc_dry_run;
static unsigned char regalloc_dry_run_cost_bytes;
static unsigned char regalloc_dry_run_cost_cycles;

static struct
{
  short debugLine;
  struct
    {
      int pushed;
    } stack;
}
_G;

enum asminst
{
  A_ADC,
  A_ADD,
  A_LD,
  A_MOV,
  A_SBC,
  A_SUB
};

static const char *asminstnames[] =
{
  "adc",
  "add",
  "ld",
  "mov",
  "sbc",
  "sub"
};

static struct asmop asmop_a, asmop_x;
static struct asmop *const ASMOP_A = &asmop_a;
static struct asmop *const ASMOP_X = &asmop_x;

void
stm8_init_asmops (void)
{
  asmop_a.type = AOP_REG;
  asmop_a.size = 1;
  asmop_a.aopu.bytes[0].in_reg = TRUE;
  asmop_a.aopu.bytes[0].byteu.reg = stm8_regs + A_IDX;

  asmop_x.type = AOP_REG;
  asmop_x.size = 2;
  asmop_x.aopu.bytes[0].in_reg = TRUE;
  asmop_x.aopu.bytes[0].byteu.reg = stm8_regs + XL_IDX;
  asmop_x.aopu.bytes[1].in_reg = TRUE;
  asmop_x.aopu.bytes[1].byteu.reg = stm8_regs + XH_IDX;
}

/*-----------------------------------------------------------------*/
/* aopRS - asmop in register or on stack                           */
/*-----------------------------------------------------------------*/
static bool
aopRS (const asmop *aop)
{
  return (aop->type == AOP_REG || aop->type == AOP_REGSTK || aop->type == AOP_STK);
}

/*-----------------------------------------------------------------*/
/* aopInREg - asmop from offset in the register                    */
/*-----------------------------------------------------------------*/
static bool
aopInReg (const asmop *aop, int offset, short rIdx)
{
  if (!aopRS (aop))
    return (FALSE);

  if (offset >= aop->size)
    return (FALSE);

  if (rIdx == X_IDX)
    return (aopInReg (aop, offset, XL_IDX) && aopInReg (aop, offset + 1, XH_IDX));

  if (rIdx == Y_IDX)
    return (aopInReg (aop, offset, YL_IDX) && aopInReg (aop, offset + 1, YH_IDX));

  return (aop->aopu.bytes[offset].in_reg && aop->aopu.bytes[offset].byteu.reg->rIdx == rIdx);
}

static void
cost(unsigned int bytes, unsigned int cycles)
{
  regalloc_dry_run_cost_bytes += bytes;
  regalloc_dry_run_cost_cycles += cycles;
}

static const char *
aopGet(const asmop *aop, int offset)
{
  static char buffer[256];

  /* Don't really need the value during dry runs, so save some time. */
  if (regalloc_dry_run)
    return ("");

  if (aop->type == AOP_LIT)
    {
      snprintf (buffer, 256, "#0x%x", byteOfVal (aop->aopu.aop_lit, offset));
      return (buffer);
    }

  if (aopRS (aop) && aop->aopu.bytes[offset].in_reg)
    return (aop->aopu.bytes[offset].byteu.reg->name);

  if (aopRS (aop) && !aop->aopu.bytes[offset].in_reg)
    {
      int soffset = aop->aopu.bytes[offset].byteu.stk + _G.stack.pushed;
      snprintf (buffer, 256, "(0x%x, sp)", soffset); //
      return (buffer);
    }

  if (aop->type == AOP_DIR)
    {
      snprintf (buffer, 256, "%s+%d", aop->aopu.aop_immd, offset);
      return (buffer);
    }

  wassert (0);
  return ("dummy");
}

static void
op8_cost (const asmop *op2, int offset2)
{
  AOP_TYPE op2type = op2->type;
  int r2Idx = ((aopRS (op2) && op2->aopu.bytes[offset2].in_reg)) ? op2->aopu.bytes[offset2].byteu.reg->rIdx : -1;

  switch (op2type)
    {
    case AOP_LIT:
    case AOP_IMMD:
      cost (2, 1);
      return;
    case AOP_DIR:
      cost (3, 1);
      return;
    case AOP_REG:
    case AOP_REGSTK:
    case AOP_STK:
      if (r2Idx != -1)
        goto error;
      cost (2, 1);
      return;
    default:
      goto error;
    }
error:
  printf("op2 type: %d, offset %d, rIdx %d\n", op2type, offset2, r2Idx);
  wassert (0);
  cost (8, 4 * 8);
}

static void
ld_cost (const asmop *op1, int offset1, const asmop *op2, int offset2)
{
  AOP_TYPE op1type = op1->type;
  AOP_TYPE op2type = op2->type;

  /* Costs are symmetric */
  if (aopRS (op2) || op2type == AOP_DUMMY)
    {
      const asmop *tmp = op1;
      const int tmpo = offset1;
      op1 = op2;
      op2 = tmp;
      offset1 = offset2;
      offset2 = tmpo;
      op1type = op1->type;
      op2type = op2->type;
    }

  int r1Idx = ((aopRS (op1) && op1->aopu.bytes[offset1].in_reg)) ? op1->aopu.bytes[offset1].byteu.reg->rIdx : -1;
  int r2Idx = ((aopRS (op2) && op2->aopu.bytes[offset2].in_reg)) ? op2->aopu.bytes[offset2].byteu.reg->rIdx : -1;

  switch (op1type)
    {
    case AOP_REG:
    case AOP_REGSTK:
    case AOP_STK:
      switch (op2type)
        {
        case AOP_LIT:
          if (r1Idx != A_IDX)
            goto error;
          cost (2, 1);
          return;
        case AOP_REG:
        case AOP_REGSTK:
        case AOP_STK:
          switch (r1Idx)
            {
            case A_IDX:
              switch (r2Idx)
                {
                case XL_IDX:
                case XH_IDX:
                  cost (1, 1);
                  return;
                case YL_IDX:
                case YH_IDX:
                case -1:
                  cost (2, 1);
                  return;
                default:
                  goto error;
                }
            case XL_IDX:
            case XH_IDX:
              if (r2Idx != A_IDX)
                goto error;
              cost (1, 1);
              return;
            case YL_IDX:
            case YH_IDX:
            case -1:
              if (r2Idx != A_IDX)
                goto error;
              cost (2, 1);
              return;
          }
        case AOP_DIR:
          if (r1Idx != A_IDX)
            goto error;
          cost (3, 2);
          return;
        default:
          goto error;
        }
    case AOP_DIR:
      if (r2Idx != A_IDX)
        goto error;
      cost (3, 2);
      return;
    default:
      goto error;
    }
error:
  printf("op1 type: %d, offset %d, rIdx %d\n", op1type, offset1, r1Idx);
  printf("op2 type: %d, offset %d, rIdx %d\n", op2type, offset2, r2Idx);
  wassert (0);
  cost (8, 4 * 8);
}

static void
mov_cost (const asmop *op1, const asmop *op2)
{
  if (op2->type == AOP_LIT || op2->type == AOP_IMMD)
    cost (4, 1);
  else
    cost (5, 1);
}

static void
emit3cost (enum asminst inst, const asmop *op1, int offset1, const asmop *op2, int offset2)
{
  switch (inst)
  {
  case A_ADC:
  case A_ADD:
    op8_cost (op2, offset2);
    break;
  case A_LD:
    ld_cost (op1, offset1, op2, offset2);
    break;
  case A_MOV:
    mov_cost (op1, op2);
    break;
  case A_SBC:
  case A_SUB:
    op8_cost (op2, offset2);
    break;
  default:
    wassertl (0, "Tried to get cost for unknown instruction");
  }
}

static void
emit3_o (enum asminst inst, asmop *op1, int offset1, asmop *op2, int offset2)
{
  emit3cost (inst, op1, offset1, op2, offset2);
  if (regalloc_dry_run)
    return;

  char *l = Safe_strdup (aopGet (op1, offset1));
  emitcode (asminstnames[inst], "%s, %s", l, aopGet (op2, offset2));
  Safe_free (l);
}

static void
emit3 (enum asminst inst, asmop *op1, asmop *op2)
{
  emit3_o (inst, op1, 0, op2, 0);
}

static bool
regFree (int idx, const iCode *ic)
{
  if (idx == X_IDX)
    return (regFree (XL_IDX, ic) && regFree (XH_IDX, ic));
  if (idx == Y_IDX)
    return (regFree (YL_IDX, ic) && regFree (YH_IDX, ic));
  return (!bitVectBitValue (ic->rMask, idx));
}

static bool
regDead (int idx, const iCode *ic)
{
  if (idx == X_IDX)
    return (regDead (XL_IDX, ic) && regDead (XH_IDX, ic));
  if (idx == Y_IDX)
    return (regDead (YL_IDX, ic) && regDead (YH_IDX, ic));
  return (!bitVectBitValue (ic->rSurv, idx));
}

/*-----------------------------------------------------------------*/
/* newAsmop - creates a new asmOp                                  */
/*-----------------------------------------------------------------*/
static asmop *
newAsmop (short type)
{
  asmop *aop;

  aop = Safe_calloc (1, sizeof (asmop));
  aop->type = type;

  return aop;
}

/*-----------------------------------------------------------------*/
/* freeAsmop - free up the asmop given to an operand               */
/*----------------------------------------------------------------*/
static void
freeAsmop (operand *op)
{
  asmop *aop;

  wassert (op);

  aop = op->aop;

  if (!aop)
    return;

  Safe_free (aop);

  op->aop = NULL;
  if (IS_SYMOP (op))
    {
      OP_SYMBOL (op)->aop = NULL;
      /* if the symbol has a spill */
      if (SPIL_LOC (op))
        SPIL_LOC (op)->aop = NULL;
    }
}

/*-----------------------------------------------------------------*/
/* aopForSym - for a true symbol                                   */
/*-----------------------------------------------------------------*/
static asmop *
aopForSym (const iCode *ic, symbol *sym)
{
  asmop *aop;

  wassert (ic);
  wassert (sym);
  wassert (sym->etype);

  /* if already has one */
  if (sym->aop)
    {
      return sym->aop;
    }

  if (IS_FUNC (sym->type))
    {
      sym->aop = aop = newAsmop (AOP_IMMD);
      aop->aopu.aop_immd = sym->rname;
      aop->size = 2;
    }
  /* Assign depending on the storage class */
  else if (sym->onStack || sym->iaccess)
    {
      aop = 0;
      wassertl (0, "Unimplemented on stack asmop.");
    }
  else
    {
      aop = newAsmop (AOP_DIR);
      aop->aopu.aop_dir = sym->rname;
      aop->size = getSize (sym->type);
    }

  sym->aop = aop;

  return aop;
}

/*-----------------------------------------------------------------*/
/* aopOp - allocates an asmop for an operand  :                    */
/*-----------------------------------------------------------------*/
static void
aopOp (operand *op, const iCode *ic)
{
  symbol *sym;
  int i;

  wassert (op);

  /* if already has an asmop */
  if (op->aop)
    return;

  /* if this a literal */
  if (IS_OP_LITERAL (op))
    {
      asmop *aop = newAsmop (AOP_LIT);
      aop->aopu.aop_lit = OP_VALUE (op);
      aop->size = getSize (operandType (op));
      op->aop = aop;
      return;
    }

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      op->aop = aopForSym (ic, OP_SYMBOL (op));
      return;
    }

  sym = OP_SYMBOL (op);

  if (sym->remat)
    {
      wassertl (0, "Unimplemented remat asmop.");
    }

  /* None of the above, which only leaves temporaries. */
  { 
    bool completly_in_regs = TRUE;
    bool completly_on_stack = TRUE;
    asmop *aop = newAsmop (AOP_REGSTK);

    for (i = 0; i < getSize (operandType (op)); i++)
      {
        aop->aopu.bytes[i].in_reg = !!sym->regs[i];
        if (sym->regs[i])
          {
            completly_on_stack = FALSE;
            aop->aopu.bytes[i].byteu.reg = sym->regs[i];
          }
        else if (sym->usl.spillLoc || sym->nRegs && regalloc_dry_run)
          {
            completly_in_regs = FALSE;
            aop->aopu.bytes[i].byteu.stk = sym->stack + i;
            wassertl (sym->stack + i < 200, "Unimplemented EXSTK.");
          }
        else
          wassertl (0, "Unimplemented dummy.");
      }

    if (completly_in_regs)
      aop->type = AOP_REG;
    else if (completly_on_stack)
      aop->type = AOP_STK;

    aop->size = getSize (operandType (op));

    op->aop = aop;
    return;
  }
}

static void
aopPush (const asmop *op, int offset, int size)
{
  if (size == 1)
    {
      emitcode ("push", "%s", aopGet (op, offset));
      if (op->type == AOP_LIT)
        cost (2, 1);
      else if (op->type == AOP_IMMD)
        cost (2, 1);
      else if (aopInReg (op, offset, A_IDX))
        cost (1, 1);
      else if (op->type == AOP_DIR)
        cost (3, 1);
      else
        wassertl (0, "Invalid aop type for size 1 for push");
    }
  if (size == 2)
    {
      if (aopInReg (op, offset, X_IDX))
        {
          emitcode ("pushw", "x");
          cost (1, 2);
        }
      else if  (aopInReg (op, offset, Y_IDX))
        {
          emitcode ("pushw", "y");
          cost (2, 2);
        }
      else
        wassertl (0, "Invalid aop type for size 2 for pushw");
    }
  else
    wassertl (0, "Invalid size for push/pushw");

  _G.stack.pushed += size;
}

static void
aopPop (const asmop *op, int offset, int size)
{
  if (size == 1)
    {
      emitcode ("pop", "%s", aopGet (op, offset));
      if (aopInReg (op, offset, A_IDX))
        cost (1, 1);
      else if (op->type == AOP_DIR)
        cost (3, 1);
      else
        wassertl (0, "Invalid aop type for size 1 for pop");
    }
  if (size == 2)
    {
      if (aopInReg (op, offset, X_IDX))
        {
          emitcode ("popw", "x");
          cost (1, 2);
        }
      else if  (aopInReg (op, offset, Y_IDX))
        {
          emitcode ("popw", "y");
          cost (2, 2);
        }
      else
        wassertl (0, "Invalid aop type for size 2 for popw");
    }
  else
    wassertl (0, "Invalid size for pop/popw");

  _G.stack.pushed -= size;
}

/*--------------------------------------------------------------------------*/
/* adjustStack - Adjust the stack pointer by n bytes.                       */
/*--------------------------------------------------------------------------*/
static void
adjustStack (int n)
{
  while (n)
    {
      // TODO: For big n, use addition in X or Y when free. Need to fix calling convention before that though.
      if (n > 255)
        {
          emitcode ("addw","sp, #255");
          cost (2, 1);
          n -= 255;
          _G.stack.pushed -= 127;
        }
      else if (n < -255)
        {
          emitcode ("subw","sp, #255");
          cost (2, 1);
          n += 255;
          _G.stack.pushed += 128;
        }
      else if (n > 0)
        {
          emitcode ("addw", "sp, #%d", n);
          cost (2, 1);
          _G.stack.pushed -= n;
          return;
        }
	  else 
	    {
		  emitcode ("subw", "sp, #%d", -n);
          cost (2, 1);
          _G.stack.pushed += n;
          return;
        }
    }
}

/*-----------------------------------------------------------------*/
/* cheapMove - Copy a byte from one asmop to another               */
/*-----------------------------------------------------------------*/
static void
cheapMove (asmop *result, int roffset, asmop *source, int soffset, bool save_a)
{
  if (aopRS (result) && aopRS (source) &&
    result->aopu.bytes[roffset].in_reg && source->aopu.bytes[soffset].in_reg &&
    result->aopu.bytes[roffset].byteu.reg == source->aopu.bytes[soffset].byteu.reg)
    return;

  if (aopRS (result) && result->aopu.bytes[roffset].in_reg && result->aopu.bytes[roffset].byteu.reg->rIdx == A_IDX ||
    aopRS (source) && source->aopu.bytes[soffset].in_reg && source->aopu.bytes[soffset].byteu.reg->rIdx == A_IDX)
    emit3_o (A_LD, result, roffset, source, soffset);
  else if (result->type == AOP_DIR && (source->type == AOP_DIR || source->type == AOP_LIT))
    emit3_o (A_MOV, result, roffset, source, soffset);
  else
    {
      if (save_a)
        /*push ()*/;
      emit3_o (A_LD, ASMOP_A, 0, source, soffset);
      emit3_o (A_LD, result, roffset, ASMOP_A, 0);
      if (save_a)
        /*pop ()*/;
    }
}

/*-----------------------------------------------------------------*/
/* genCopy - Copy the value from one reg/stk asmop to another      */
/*-----------------------------------------------------------------*/
static void
genCopy (asmop *result, asmop *source, bool a_dead, bool x_dead, bool y_dead)
{
  int i, regsize, size = result->size;
  bool assigned[8] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
  int ex[4] = {-1, -1, -1, -1};

  emitcode("; genCopy", "");

  wassertl (aopRS (source), "Invalid source type.");
  wassertl (aopRS (result), "Invalid result type.");

  // TODO: Use exg/exgw for optimization.

  // Now do the register shuffling.
  for (i = 0, regsize = 0; i < size; i++)
    regsize += source->aopu.bytes[i].in_reg;
  while (regsize)
    {
      // Find lowest byte that can be assigned and needs to be assigned.
      for (i = 0; i < size; i++)
        {
          size_t j;

          if (assigned[i] || !source->aopu.bytes[i].in_reg)
            continue;

          for (j = 0; j < size; j++)
            {
              if (!source->aopu.bytes[j].in_reg || !result->aopu.bytes[i].in_reg)
                continue;
              if (!assigned[j] && i != j && result->aopu.bytes[i].byteu.reg == source->aopu.bytes[j].byteu.reg)
                goto skip_byte; // We can't write this one without overwriting the source.
            }

          break;                // Found byte that can be written safely.

skip_byte:
          ;
        }

      if (i < size)
        {
          cheapMove (result, i, source, i, FALSE);       // We can safely assign a byte. TODO: Take care of A!
          regsize--;
          assigned[i] = TRUE;
          continue;
        }

      // No byte can be assigned safely (i.e. the assignment is a permutation). Cache one in the accumulator.
      wassertl (0, "Unimplemented.");
      return;
    }

  // In the end, move from the stack to destination.
  wassertl (regsize == size, "Unimplemented.");
}

/*---------------------------------------------------------------------*/
/* stm8_emitDebuggerSymbol - associate the current code location       */
/*   with a debugger symbol                                            */
/*---------------------------------------------------------------------*/
void
stm8_emitDebuggerSymbol (const char *debugSym)
{
  _G.debugLine = 1;
  emitcode ("", "%s ==.", debugSym);
  _G.debugLine = 0;
}

/*-----------------------------------------------------------------*/
/* genFunction - generated code for function entry                 */
/*-----------------------------------------------------------------*/
static void
genFunction (iCode *ic)
{
  const symbol *sym = OP_SYMBOL_CONST (IC_LEFT (ic));
  sym_link *ftype = operandType (IC_LEFT (ic));

  /* create the function header */
  emitcode (";", "-----------------------------------------");
  emitcode (";", " function %s", sym->name);
  emitcode (";", "-----------------------------------------");

  emitcode ("", "%s:", sym->rname);
  genLine.lineCurr->isLabel = 1;

  if (IFFUNC_ISNAKED(ftype))
  {
      emitcode(";", "naked function: no prologue.");
      return;
  }

  /* adjust the stack for the function */
  if (sym->stack)
    adjustStack (-sym->stack);
}

/*-----------------------------------------------------------------*/
/* genEndFunction - generates epilogue for functions               */
/*-----------------------------------------------------------------*/
static void
genEndFunction ( iCode *ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));

  if (IFFUNC_ISNAKED(sym->type))
  {
      emitcode(";", "naked function: no epilogue.");
      if (options.debug && currFunc)
        debugFile->writeEndFunction (currFunc, ic, 0);
      return;
  }

  if (sym->stack)
    adjustStack (sym->stack);

  if (IFFUNC_ISISR (sym->type))
    {
      /* if debug then send end of function */
      if (options.debug && currFunc)
        {
          debugFile->writeEndFunction (currFunc, ic, 1);
        }

      emitcode ("iret", "");
      cost (1, 11);
    }
  else
    {
      /* if debug then send end of function */
      if (options.debug && currFunc)
        debugFile->writeEndFunction (currFunc, ic, 1);

      emitcode ("ret", "");
      cost (1, 11);
    }
}

/*-----------------------------------------------------------------*/
/* genLabel - generates a label                                    */
/*-----------------------------------------------------------------*/
static void
genLabel (const iCode *ic)
{
  /* special case never generate */
  if (IC_LABEL (ic) == entryLabel)
    return;

  emitLabel (IC_LABEL (ic));
}

/*-----------------------------------------------------------------*/
/* genGoto - generates a jump                                      */
/*-----------------------------------------------------------------*/
static void
genGoto (const iCode *ic)
{
  emitcode ("jp", "%05d$", labelKey2num (IC_LABEL (ic)->key));
  cost (3, 1);
}

/*-----------------------------------------------------------------*/
/* genPlus - generates code for addition                           */
/*-----------------------------------------------------------------*/
static void
genPlus (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  int size, i;
  bool started;

  emitcode("; genPlus", "");

  aopOp (IC_LEFT (ic), ic);
  aopOp (IC_RIGHT (ic), ic);
  aopOp (IC_RESULT (ic), ic);

  /* Swap if left is literal or right is in A. */
  if (left->aop->type == AOP_LIT || right->aop->size == 1 && aopInReg (right->aop, 1, A_IDX))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  
  size = result->aop->size;
  for(i = 0, started = FALSE; i < size;)
    {
      if (0) // TODO: Use addw where it provides an advantage.
        ;
      else // TODO: Take care of A. TODO: Handling of right operands that can't be directly added to a.
        {
          cheapMove (ASMOP_A, 0, left->aop, i, FALSE);
          if (right->aop->type == AOP_LIT && !byteOfVal (right->aop->aopu.aop_lit, i))
            {
              // Skip over this byte.
            }
          // We can use inc only for the only non-zero byte, since it neither takes into account an existing carry nor does it update the carry.
          else if (!started && i == size - 1 && right->aop->type == AOP_LIT && byteOfVal (right->aop->aopu.aop_lit, i) == 1)
            {
              emitcode ("inc", "a");
              cost (1, 1);
              started = TRUE;
            }
          else
            {
              emit3_o (started ? A_ADC : A_ADD, ASMOP_A, 0, right->aop, i);
              started = TRUE;
            }
          cheapMove (result->aop, i, ASMOP_A, 0, FALSE);
          i++;
        }
    }

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genMinus - generates code for subtraction                       */
/*-----------------------------------------------------------------*/
static void
genMinus (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  int size, i;
  bool started;

  emitcode("; genMinus", "");

  aopOp (IC_LEFT (ic), ic);
  aopOp (IC_RIGHT (ic), ic);
  aopOp (IC_RESULT (ic), ic);
  
  size = result->aop->size;
  for(i = 0, started = FALSE; i < size;)
    {
      if (0) // TODO: Use subw where it provides an advantage.
        ;
      else // TODO: Take care of A. TODO: Handling of right operands that can't be directly subtracted from a.
        {
          cheapMove (ASMOP_A, 0, left->aop, i, FALSE);
          if (right->aop->type == AOP_LIT && !byteOfVal (right->aop->aopu.aop_lit, i))
            {
              // Skip over this byte.
            }
          else
            {
              emit3_o (started ? A_SBC : A_SUB, ASMOP_A, 0, right->aop, i);
              started = TRUE;
            }
          cheapMove (result->aop, i, ASMOP_A, 0, FALSE);
          i++;
        }
    }

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genPointerGet - generate code for pointer get                   */
/*-----------------------------------------------------------------*/
static void
genPointerGet (const iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  int size, i;
  unsigned offset;

  emitcode("; genPointerGet", "");

  aopOp (IC_LEFT (ic), ic);
  aopOp (IC_RIGHT (ic), ic);
  aopOp (IC_RESULT (ic), ic);

  wassertl (right, "GET_VALUE_AT_ADDRESS without right operand");
  wassertl (IS_OP_LITERAL (IC_RIGHT (ic)), "GET_VALUE_AT_ADDRESS with non-literal right operand");

  // TODO: Use Y when X is not available (or save X on stack), use ldw, etc.

  //genCopy (ASMOP_X, left, regDead (A_IDX, ic), regDead (X_IDX, ic), regDead (Y_IDX, ic));

  // TODO: What if right operand is negative?
  offset = byteOfVal (right->aop->aopu.aop_lit, 0) * 256 + byteOfVal (right->aop->aopu.aop_lit, 0);

  size = result->aop->size;
  for (i = 0; i < size; i++)
    {
      if (offset == 0)
        {
          emitcode ("ld", "a, (x)");
          cost (1, 1);
        }
      else
        {
          emitcode ("ld", "a, (0x%x, x)", offset);
          cost (offset < 256 ? 2 : 3, 1);
        }
      cheapMove (result->aop, i, ASMOP_A, 0, FALSE);
      offset++;
    }

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genAssign - generate code for assignment                        */
/*-----------------------------------------------------------------*/
static void
genAssign (const iCode *ic)
{
  operand *result, *right;
  int i;

  emitcode("; genAssign", "");

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);

  aopOp (right, ic);
  aopOp (result, ic);

  if (aopRS (result->aop) && aopRS (right->aop))
    genCopy (result->aop, right->aop, regDead (A_IDX, ic), regDead (X_IDX, ic), regDead (Y_IDX, ic));

  // TODO: Efficient handling of more special cases.
  for (i = 0; i < result->aop->size;)
    {
      if (aopInReg (result->aop, i, X_IDX) && right->aop->type == AOP_DIR)
        {
          emitcode ("ldw", "x, %s", aopGet (right->aop, i));
          cost (3, 2);
          i += 2;
        }
      else if (aopInReg (result->aop, i, Y_IDX) && right->aop->type == AOP_DIR)
        {
          emitcode ("ldw", "y, %s", aopGet (right->aop, i));
          cost (4, 2);
          i += 2;
        }
      else if (result->aop->type == AOP_DIR && aopInReg (right->aop, i, X_IDX))
        {
          emitcode ("ldw", "%s, x", aopGet (result->aop, i));
          cost (3, 2);
          i += 2;
        }
      else if (result->aop->type == AOP_DIR && aopInReg (right->aop, i, Y_IDX))
        {
          emitcode ("ldw", "%s, y", aopGet (result->aop, i));
          cost (4, 2);
          i += 2;
        }
      else
        {
          cheapMove (result->aop, i, right->aop, i, FALSE); // TODO: Take care of A.
          i++;
        }
    }

  freeAsmop (right);
  freeAsmop (result);
}

/*---------------------------------------------------------------------*/
/* genSTM8Code - generate code for STM8 for a single iCode instruction */
/*---------------------------------------------------------------------*/
static void
genSTM8iCode (iCode *ic)
{
  switch (ic->op)
    {
    case '!':
    case '~':
    case UNARYMINUS:
    case IPUSH:
    case IPOP:
    case CALL:
    case PCALL:
      wassertl (0, "Unimplemented iCode");
      break;

    case FUNCTION:
      genFunction (ic);
      break;

    case ENDFUNCTION:
      genEndFunction (ic);
      break;

    case RETURN:
      wassertl (0, "Unimplemented iCode");
      break;

    case LABEL:
      genLabel (ic);
      break;

    case GOTO:
      genGoto (ic);
      break;

    case '+':
      genPlus (ic);
      break;

    case '-':
      genMinus (ic);
      break;

    case '*':
    case '/':
    case '%':
    case '>':
    case '<':
      wassertl (0, "Unimplemented iCode");
      break;

    case LE_OP:
    case GE_OP:
    case NE_OP:

      /* note these two are xlated by algebraic equivalence
         during parsing SDCC.y */
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "got '>=' or '<=' shouldn't have come here");
      break;

    case EQ_OP:
    case AND_OP:
    case OR_OP:
    case '^':
    case '|':
    case BITWISEAND:
    case INLINEASM:
    case RRC:
    case RLC:
    case GETHBIT:
    case GETABIT:
    case LEFT_OP:
    case RIGHT_OP:
      wassertl (0, "Unimplemented iCode");
      break;

    case GET_VALUE_AT_ADDRESS:
      genPointerGet (ic);
      break;

    case '=':
      genAssign (ic);
      break;

    case IFX:
    case ADDRESS_OF:
    case JUMPTABLE:
    case CAST:
    case RECEIVE:
    case SEND:
    case DUMMY_READ_VOLATILE:
    case CRITICAL:
    case ENDCRITICAL:
      wassertl (0, "Unimplemented iCode");
      break;

    default:
      wassertl (0, "Unknown iCode");
    }
}

unsigned int
drySTM8iCode (iCode *ic)
{
  regalloc_dry_run = TRUE;
  regalloc_dry_run_cost_bytes = 0;
  regalloc_dry_run_cost_cycles = 0;

  initGenLineElement ();

  genSTM8iCode (ic);

  destroy_line_list ();

  wassert (regalloc_dry_run);

  return (regalloc_dry_run_cost_bytes);
}

#ifdef DEBUG_DRY_COST
static void
drySTM8Code (iCode * lic)
{
  iCode *ic;

  for (ic = lic; ic; ic = ic->next)
    if (ic->op != FUNCTION && ic->op != ENDFUNCTION && ic->op != LABEL && ic->op != GOTO && ic->op != INLINEASM)
      printf ("; iCode %d total cost: %d\n", ic->key, (int) (drySTM8iCode (ic)));
}
#endif

/*---------------------------------------------------------------------*/
/* genSTM8Code - generate code for STM8 for a block of intructions     */
/*---------------------------------------------------------------------*/
void
genSTM8Code (iCode *lic)
{
#ifdef DEBUG_DRY_COST
  drySTM8Code (lic);
#endif

  iCode *ic;
  regalloc_dry_run = FALSE;

  for (ic = lic; ic; ic = ic->next)
    {
      initGenLineElement ();

      regalloc_dry_run_cost_bytes = 0;
      regalloc_dry_run_cost_cycles = 0;

      if (options.iCodeInAsm)
        {
          const char *iLine = printILine (ic);
          emitcode ("; ic:", "%d: %s", ic->key, iLine);
          dbuf_free (iLine);
        }

      genSTM8iCode(ic);
    }

  /* now we are ready to call the
     peephole optimizer */
  if (!options.nopeep)
    peepHole (&genLine.lineHead);

  /* now do the actual printing */
  printLine (genLine.lineHead, codeOutBuf);

  /* destroy the line list */
  destroy_line_list ();
}

