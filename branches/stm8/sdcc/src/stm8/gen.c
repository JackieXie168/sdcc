/*-------------------------------------------------------------------------
  gen.c - code generator for STM8.

  Copyright (C) 2012 - 2013, Philipp Klaus Krause pkk@spth.de, philipp@informatik.uni-frankfurt.de)
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

/* Use the D macro for basic (unobtrusive) debugging messages */
#define D(x) do if (options.verboseAsm) { x; } while (0)

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
  A_AND,
  A_CLR,
  A_CP,
  A_LD,
  A_MOV,
  A_OR,
  A_RLC,
  A_RRC,
  A_SBC,
  A_SLL,
  A_SRL,
  A_SUB,
  A_TNZ,
  A_XOR
};

static const char *asminstnames[] =
{
  "adc",
  "add",
  "and",
  "clr",
  "cp",
  "ld",
  "mov",
  "or",
  "rlc",
  "rrc",
  "sbc",
  "sll",
  "srl",
  "sub",
  "tnz",
  "xor"
};

static struct asmop asmop_a, asmop_x, asmop_y, asmop_zero;
static struct asmop *const ASMOP_A = &asmop_a;
static struct asmop *const ASMOP_X = &asmop_x;
static struct asmop *const ASMOP_Y = &asmop_y;
static struct asmop *const ASMOP_ZERO = &asmop_zero;

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

  asmop_y.type = AOP_REG;
  asmop_y.size = 2;
  asmop_y.aopu.bytes[0].in_reg = TRUE;
  asmop_y.aopu.bytes[0].byteu.reg = stm8_regs + YL_IDX;
  asmop_y.aopu.bytes[1].in_reg = TRUE;
  asmop_y.aopu.bytes[1].byteu.reg = stm8_regs + YH_IDX;

  asmop_zero.type = AOP_LIT;
  asmop_zero.size = 1;
  asmop_zero.aopu.aop_lit = constVal ("0");
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

/*-----------------------------------------------------------------*/
/* aopInREg - asmop from offset on stack                           */
/*-----------------------------------------------------------------*/
static bool
aopOnStack (const asmop *aop, int offset, int size)
{
  int i;

  if (!aopRS (aop))
    return (FALSE);

  if (offset + size > aop->size)
    return (FALSE);

  for (i = offset; i < offset + size; i++)
    if (aop->aopu.bytes[i].in_reg)
      return (FALSE);

  return (TRUE);
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
      snprintf (buffer, 256, "(0x%02x, sp)", soffset);
      return (buffer);
    }

  if (aop->type == AOP_IMMD)
    {
      snprintf (buffer, 256, "%s+%d", aop->aopu.aop_immd, offset);
      return (buffer);
    }

  if (aop->type == AOP_DIR)
    {
      snprintf (buffer, 256, "%s+%d", aop->aopu.aop_dir, offset);
      return (buffer);
    }

  wassert (0);
  return ("dummy");
}

/* For operantions that always have the accumulator as left operand. */
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
  fprintf(stderr, "op2 type: %d, offset %d, rIdx %d\n", op2type, offset2, r2Idx);
  wassert (0);
  cost (8, 4 * 8);
}

/* For operations that have only one operand, i.e. tnz */
static void
op_cost (const asmop *op1, int offset1)
{
  AOP_TYPE op1type = op1->type;
  int r1Idx = ((aopRS (op1) && op1->aopu.bytes[offset1].in_reg)) ? op1->aopu.bytes[offset1].byteu.reg->rIdx : -1;

  switch (op1type)
    {
    case AOP_DIR:
      cost (4, 1);
      return;
    case AOP_REG:
      if (r1Idx != A_IDX)
        goto error;
      cost (1, 1);
      return;
    case AOP_REGSTK:
    case AOP_STK:
      if (r1Idx != -1)
        goto error;
      cost (2, 1);
      return;
    default:
      goto error;
    }
error:
  fprintf(stderr, "op1 type: %d, offset %d, rIdx %d\n", op1type, offset1, r1Idx);
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
        case AOP_IMMD:
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
  fprintf(stderr, "op1 type: %d, offset %d, rIdx %d\n", op1type, offset1, r1Idx);
  fprintf(stderr, "op2 type: %d, offset %d, rIdx %d\n", op2type, offset2, r2Idx);
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
  case A_AND:
    op8_cost (op2, offset2);
    break;
  case A_CLR:
    op_cost (op1, offset1);
    break;
  case A_CP:
    op8_cost (op2, offset2);
    break;
  case A_LD:
    ld_cost (op1, offset1, op2, offset2);
    break;
  case A_MOV:
    mov_cost (op1, op2);
    break;
  case A_OR:
    op8_cost (op2, offset2);
    break;
  case A_RLC:
  case A_RRC:
    op_cost (op1, offset1);
    break;
  case A_SBC:
    op8_cost (op2, offset2);
    break;
  case A_SLL:
  case A_SRL:
    op_cost (op1, offset1);
    break;
  case A_SUB:
    op8_cost (op2, offset2);
    break;
  case A_TNZ:
    op_cost (op1, offset1);
    break;
  case A_XOR:
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

  if (op2)
    {
      char *l = Safe_strdup (aopGet (op1, offset1));
      emitcode (asminstnames[inst], "%s, %s", l, aopGet (op2, offset2));
      Safe_free (l);
    }
  else
    emitcode (asminstnames[inst], "%s", aopGet (op1, offset1));
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
      int offset;

      sym->aop = aop = newAsmop (AOP_STK);
      aop->size = getSize (sym->type);
      
      for(offset = 0; offset < aop->size; offset++)
        aop->aopu.bytes[offset].byteu.stk = sym->stack + offset;
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

  sym = OP_SYMBOL (op);

  /* if this is a true symbol */
  if (IS_TRUE_SYMOP (op))
    {
      op->aop = aopForSym (ic, sym);
      return;
    }

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
          wassertl (0, "Unimplemented dummy aop.");
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
push (const asmop *op, int offset, int size)
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
  else if (size == 2)
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
pop (const asmop *op, int offset, int size)
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
  else if (size == 2)
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

void swap_to_a(int idx)
{
  switch (idx)
    {
    case XL_IDX:
      emitcode ("exg", "a, yl");
      cost (1, 1);
      break;
    case XH_IDX:
      emitcode ("rlwa", "x");
      cost (1, 1);
      break;
    case YL_IDX:
      emitcode ("exg", "a, yl");
      cost (1, 1);
      break;
    case YH_IDX:
      emitcode ("rlwa", "y");
      cost (2, 1);
      break;
    default:
      wassert (0);
    }
}

void swap_from_a(int idx)
{
  switch (idx)
    {
    case XL_IDX:
      emitcode ("exg", "a, yl");
      cost (1, 1);
      break;
    case XH_IDX:
      emitcode ("rrwa", "x");
      cost (1, 1);
      break;
    case YL_IDX:
      emitcode ("exg", "a, yl");
      cost (1, 1);
      break;
    case YH_IDX:
      emitcode ("rrwa", "y");
      cost (2, 1);
      break;
    default:
      wassert (0);
    }
}

/*--------------------------------------------------------------------------*/
/* adjustStack - Adjust the stack pointer by n bytes.                       */
/*--------------------------------------------------------------------------*/
static void
adjustStack (int n)
{
  while (n)
    {
      // TODO: For small n, adjust stack by using pop/push a and pushw/popw x where these are dead.
      // TODO: For big n, use addition in X or Y when free. Need to fix calling convention before that though.
      if (n > 255)
        {
          emitcode ("addw","sp, #255");
          cost (2, 1);
          n -= 255;
          _G.stack.pushed += 255;
        }
      else if (n < -255)
        {
          emitcode ("subw","sp, #255");
          cost (2, 1);
          n += 255;
          _G.stack.pushed -= 255;
        }
      else if (n > 0)
        {
          emitcode ("addw", "sp, #%d", n);
          cost (2, 1);
          _G.stack.pushed += n;
          return;
        }
	  else 
	    {
		  emitcode ("subw", "sp, #%d", -n);
          cost (2, 1);
          _G.stack.pushed -= -n;
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
  else if (aopInReg (result, roffset, A_IDX) || aopInReg (source, soffset, A_IDX))
    emit3_o (A_LD, result, roffset, source, soffset);
  else if (result->type == AOP_DIR && (source->type == AOP_DIR || source->type == AOP_LIT))
    emit3_o (A_MOV, result, roffset, source, soffset);
  else
    {
      if (save_a)
        push (ASMOP_A, 0, 1);
      if (!aopInReg (source, soffset, A_IDX))
        emit3_o (A_LD, ASMOP_A, 0, source, soffset);
      if (!aopInReg (result, roffset, A_IDX))
        emit3_o (A_LD, result, roffset, ASMOP_A, 0);
      if (save_a)
        pop (ASMOP_A, 0, 1);
    }
}

/*-----------------------------------------------------------------*/
/* genCopy - Copy the value - stack to stack only                  */
/*-----------------------------------------------------------------*/
static void
genCopyStack (asmop *result, asmop *source, bool *assigned, int *size, bool a_free, bool x_free, bool y_free, bool really_do_it_now)
{
  int i, n = result->size;

  for (i = 0; i < n;)
    {
      // Could transfer two bytes at a time now.
      if (i + 1 < n &&
        !assigned[i] && !assigned[i + 1] &&
        !result->aopu.bytes[i].in_reg && !result->aopu.bytes[i + 1].in_reg &&
        !source->aopu.bytes[i].in_reg && !source->aopu.bytes[i + 1].in_reg)
        {
          wassert(*size >= 2);
          if (y_free)
            {
              emitcode ("ldw", "y, %s", aopGet (source, i));
              emitcode ("ldw", "%s, y", aopGet (result, i));
            }
          else if (x_free)
            {
              emitcode ("ldw", "x, %s", aopGet (source, i));
              emitcode ("ldw", "%s, x", aopGet (result, i));
            }
          cost (4, 4);   
          assigned[i] = TRUE;
          assigned[i + 1] = TRUE;
          (*size) -= 2;
          i += 2;
        }
      else
        i++;
    }

  for (i = 0; i < n;)
    {
      // Just one byte to transfer.
      if (a_free && !assigned[i] &&
        (i + 1 >= n || assigned[i + 1] || really_do_it_now) &&
        !result->aopu.bytes[i].in_reg && !source->aopu.bytes[i].in_reg)
        {
          wassert(*size >= 1);
          cheapMove (result, i, source, i, FALSE);
          assigned[i] = TRUE;
          (*size)--;
          i++;
        }
      else
        i++;
    }
}

/*-----------------------------------------------------------------*/
/* genCopy - Copy the value from one reg/stk asmop to another      */
/*-----------------------------------------------------------------*/
static void
genCopy (asmop *result, asmop *source, bool a_dead, bool x_dead, bool y_dead)
{
  int i, regsize, size, n = result->size;
  bool assigned[8] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
  int ex[4];
  bool a_free, x_free, y_free;

  D (emitcode("; genCopy", ""));

  wassertl (n <= 8, "Invalid size for genCopy().");
  wassertl (aopRS (source), "Invalid source type.");
  wassertl (aopRS (result), "Invalid result type.");

  size = n;
  for (i = 0, regsize = 0; i < n; i++)
    regsize += source->aopu.bytes[i].in_reg;

  // Copy (stack-to-stack) what we can with whatever free regs we have.
  a_free = !a_dead;
  x_free = !x_dead;
  y_free = !y_dead;
  for (i = 0; i < n; i++)
    {
      if (aopInReg (source, i, A_IDX))
        a_free = FALSE;
      else if (aopInReg (source, i, XL_IDX) || aopInReg (source, i, XH_IDX))
        x_free = FALSE;
      else if (aopInReg (source, i, YL_IDX) || aopInReg (source, i, YH_IDX))
        y_free = FALSE;
    }
  genCopyStack (result, source, assigned, &size, a_free, x_free, y_free, FALSE);

  // Move everything that can be easily moved from registers to the stack.
  for (i = 0; i < n;)
    {
      if (aopInReg (source, i, X_IDX) && aopOnStack (result, i, 2))
        {
          emitcode ("ldw", "%s, x", aopGet (result, i));
          cost (2, 2);
          assigned[i] = TRUE;
          assigned[i + 1] = TRUE;
          regsize -= 2;
          size -= 2;
          i += 2;
        }
      else if (aopInReg (source, i, Y_IDX) && aopOnStack (result, i, 2))
        {
          emitcode ("ldw", "%s, y", aopGet (result, i));
          cost (2, 2);
          assigned[i] = TRUE;
          assigned[i + 1] = TRUE;
          regsize -= 2;
          size -= 2;
          i += 2;
        }
      else if (aopInReg (source, i, A_IDX) && aopOnStack (result, i, 1))
        {
          emitcode ("ld", "%s, a", aopGet (result, i));
          cost (2, 1);
          assigned[i] = TRUE;
          regsize--;
          size--;
          i++;
        }
      else
        i++;
    }

  // Now do the register shuffling.

  // TODO: Try to use rlwa and rrwa.

  // Try to use exgw
  ex[0] = -1;
  ex[1] = -1;
  ex[2] = -1;
  ex[3] = -1;
  if (regsize >= 4)
    {
      // Find XL and check that it is exchanged with YL.
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, XL_IDX) && aopInReg (source, i, YL_IDX))
          ex[0] = i;
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, YL_IDX) && aopInReg (source, i, XL_IDX))
          ex[1] = i;
      // Find XH and check that it is exchanged with YH.
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, XH_IDX) && aopInReg (source, i, YH_IDX))
          ex[2] = i;
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, YH_IDX) && aopInReg (source, i, XH_IDX))
          ex[3] = i;
      if (ex[0] >= 0 && ex[1] >= 0 && ex[2] >= 0 && ex[3] >= 0)
        {
          emitcode ("exgw", "x, y");
          cost (1, 1);
          assigned[ex[0]] = TRUE;
          assigned[ex[1]] = TRUE;
          assigned[ex[2]] = TRUE;
          assigned[ex[3]] = TRUE;
          regsize -= 4;
          size -= 4;
        }
    }

  // Try to use exg a, xl
  ex[0] = -1;
  ex[1] = -1;
  if (regsize >= 2)
    {
      // Find XL and check that it is exchanged with XH.
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, A_IDX) && aopInReg (source, i, XL_IDX))
          ex[0] = i;
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, XL_IDX) && aopInReg (source, i, A_IDX))
          ex[1] = i;
      if (ex[0] >= 0 && ex[1] >= 0)
        {
          emitcode ("exg", "a, xl");
          cost (1, 1);
          assigned[ex[0]] = TRUE;
          assigned[ex[1]] = TRUE;
          regsize -= 2;
          size -= 2;
        }
    }

  // Try to use exg a, yl
  ex[0] = -1;
  ex[1] = -1;
  if (regsize >= 2)
    {
      // Find XL and check that it is exchanged with XH.
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, A_IDX) && aopInReg (source, i, YL_IDX))
          ex[0] = i;
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, YL_IDX) && aopInReg (source, i, A_IDX))
          ex[1] = i;
      if (ex[0] >= 0 && ex[1] >= 0)
        {
          emitcode ("exg", "a, yl");
          cost (1, 1);
          assigned[ex[0]] = TRUE;
          assigned[ex[1]] = TRUE;
          regsize -= 2;
          size -= 2;
        }
    }

  // Try to use swapw x
  ex[0] = -1;
  ex[1] = -1;
  if (regsize >= 2)
    {
      // Find XL and check that it is exchanged with XH.
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, XL_IDX) && aopInReg (source, i, XH_IDX))
          ex[0] = i;
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, XH_IDX) && aopInReg (source, i, XL_IDX))
          ex[1] = i;
      if (ex[0] >= 0 && ex[1] >= 0)
        {
          emitcode ("swapw", "x");
          cost (1, 1);
          assigned[ex[0]] = TRUE;
          assigned[ex[1]] = TRUE;
          regsize -= 2;
          size -= 2;
        }
    }

  // Try to use swapw y
  ex[0] = -1;
  ex[1] = -1;
  if (regsize >= 2)
    {
      // Find YL and check that it is exchanged with YH.
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, YL_IDX) && aopInReg (source, i, YH_IDX))
          ex[0] = i;
      for (i = 0; i < n; i++)
        if (!assigned[i] && aopInReg (result, i, YH_IDX) && aopInReg (source, i, YL_IDX))
          ex[1] = i;
      if (ex[0] >= 0 && ex[1] >= 0)
        {
          emitcode ("swapw", "y");
          cost (2, 1);
          assigned[ex[0]] = TRUE;
          assigned[ex[1]] = TRUE;
          regsize -= 2;
          size -= 2;
        }
    }

  while (regsize)
    {
      // Find lowest byte that can be assigned and needs to be assigned.
      for (i = 0; i < n; i++)
        {
          size_t j;

          if (assigned[i] || !source->aopu.bytes[i].in_reg)
            continue;

          for (j = 0; j < n; j++)
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

      if (i < n)
        {
          cheapMove (result, i, source, i, FALSE);       // We can safely assign a byte. TODO: Take care of A!
          regsize--;
          size--;
          assigned[i] = TRUE;
          continue;
        }

      // No byte can be assigned safely (i.e. the assignment is a permutation). Cache one in the accumulator.
      if (!regalloc_dry_run)
        wassertl (0, "Unimplemented.");
      cost (80, 80);
      return;
    }

  // Last, move everything that can be easily moved from stack to registers.
  for (i = 0; i < n;)
    {
      if (aopInReg (result, i, X_IDX) && aopOnStack (source, i, 2))
        {
          emitcode ("ldw", "x, %s", aopGet (source, i));
          cost (2, 2);
          assigned[i] = TRUE;
          assigned[i + 1] = TRUE;
          regsize -= 2;
          size -= 2;
          i += 2;
        }
      else if (aopInReg (result, i, Y_IDX) && aopOnStack (source, i, 2))
        {
          emitcode ("ldw", "y, %s", aopGet (source, i));
          cost (2, 2);
          assigned[i] = TRUE;
          assigned[i + 1] = TRUE;
          regsize -= 2;
          size -= 2;
          i += 2;
        }
      else if (aopInReg (result, i, A_IDX) && aopOnStack (source, i, 1))
        {
          emitcode ("ld", "a, %s", aopGet (source, i));
          cost (2, 1);
          assigned[i] = TRUE;
          regsize--;
          size--;
          i++;
        }
      // TODO: Use ldw to load xl, xh, yl, yh when the other half is not in use.
      else if ((aopInReg (result, i, XL_IDX) || aopInReg (result, i, YL_IDX)) && !source->aopu.bytes[i].in_reg)
        {
          emitcode ("exg", "a, %s", aopGet (result, i));
          emitcode ("ld", "a, %s", aopGet (source, i));
          emitcode ("exg", "a, %s", aopGet (result, i));
          cost (4, 3);
          assigned[i] = TRUE;
          regsize--;
          size--;
          i++;
        }
      else if ((aopInReg (result, i, XH_IDX) || aopInReg (result, i, YH_IDX)) && !source->aopu.bytes[i].in_reg)
        {
          bool y = aopInReg (result, i, YH_IDX);
          emitcode ("rlwa", y ? "y, a" : "x, a");
          emitcode ("ld", "a, %s", aopGet (source, i));
          emitcode ("rrwa", y ? "y, a" : "x, a");
          cost (4, 3 + y * 2);
          assigned[i] = TRUE;
          regsize--;
          size--;
          i++;
        }
      else
        i++;
    }

  wassertl (size >= 0, "genCopy() copied more than there is to be copied.");

  // Copy (stack-to-stack) what we can with whatever free regs we have now.
  a_free = !a_dead;
  x_free = !x_dead;
  y_free = !y_dead;
  for (i = 0; i < n; i++)
    {
      if (aopInReg (result, i, A_IDX))
        a_free = FALSE;
      else if (aopInReg (result, i, XL_IDX) || aopInReg (result, i, XH_IDX))
        x_free = FALSE;
      else if (aopInReg (result, i, YL_IDX) || aopInReg (result, i, YH_IDX))
        y_free = FALSE;
    }
  genCopyStack (result, source, assigned, &size, a_free, x_free, y_free, FALSE);

  // Free a reg to copy (stack-to-stack) whatever is left.
  if (size)
    {
      push (ASMOP_A, 0, 1);
      genCopyStack (result, source, assigned, &size, TRUE, x_free, y_free, TRUE);
      pop (ASMOP_A, 0, 1);
    }

  if (size)
    {
      if (!regalloc_dry_run)
        {
          wassertl (0, "genCopy failed to completly copy operands.");
          printf ("%d bytes left.\n", size);
          for (i = 0; i < n ; i++)
            printf ("Byte %d, result in reg %d, source in reg %d. %s assigned.\n", i, result->aopu.bytes[i].in_reg ? result->aopu.bytes[i].byteu.reg->rIdx : -1, source->aopu.bytes[i].in_reg ? source->aopu.bytes[i].byteu.reg->rIdx : -1, assigned[i] ? "" : "not");
        }
      cost (80, 80);
    }
}

/*-----------------------------------------------------------------*/
/* genMove_o - Copy part of one asmop to another                   */
/*-----------------------------------------------------------------*/
static void
genMove_o (asmop *result, int roffset, asmop *source, int soffset, int size, bool a_dead, bool x_dead, bool y_dead)
{
  int i;

  if (aopRS (result) && aopRS (source))
    {
      genCopy (result, source, a_dead, x_dead, y_dead);
      return;
    }

  // TODO: Efficient handling of more special cases.
  for (i = 0; i < size;)
    {
      if (aopInReg (result, roffset + i, X_IDX) && (source->type == AOP_DIR || source->type == AOP_IMMD || source->type == AOP_LIT))
        {
          emitcode ("ldw", "x, %s", aopGet (source, soffset + i));
          cost (3, 2);
          i += 2;
        }
      else if (aopInReg (result, roffset + i, Y_IDX) && (source->type == AOP_DIR || source->type == AOP_IMMD || source->type == AOP_LIT))
        {
          emitcode ("ldw", "y, %s", aopGet (source, soffset + i));
          cost (4, 2);
          i += 2;
        }
      else if (result->type == AOP_DIR && aopInReg (source, soffset + i, X_IDX))
        {
          emitcode ("ldw", "%s, x", aopGet (result, roffset + i));
          cost (3, 2);
          i += 2;
        }
      else if (result->type == AOP_DIR && aopInReg (source, soffset + i, Y_IDX))
        {
          emitcode ("ldw", "%s, y", aopGet (result, roffset + i));
          cost (4, 2);
          i += 2;
        }
      else
        {
          cheapMove (result, roffset + i, source, soffset + i, TRUE); // TODO: Relax requirement on saving a.
          i++;
        }
    }
}

/*-----------------------------------------------------------------*/
/* genMove - Copy the value from one asmop to another              */
/*-----------------------------------------------------------------*/
static void
genMove (asmop *result, asmop *source, bool a_dead, bool x_dead, bool y_dead)
{
  genMove_o (result, 0, source, 0, result->size, a_dead, x_dead, y_dead);
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
/* genIpush - genrate code for pushing this gets a little complex  */
/*-----------------------------------------------------------------*/
static void
genIpush (const iCode * ic)
{
  int size, offset = 0;

  D (emitcode ("; genIPush", ""));

  /* if this is not a parm push : ie. it is spill push
     and spill push is always done on the local stack */
  if (!ic->parmPush)
    {
      wassertl (0, "Encountered an unsupported spill push.");
      return;
    }

#if 0
  if (_G.saves.saved == FALSE && !regalloc_dry_run /* Cost is counted at CALL or PCALL instead */ )
    {
      /* Caller saves, and this is the first iPush. */
      /* Scan ahead until we find the function that we are pushing parameters to.
         Count the number of addSets on the way to figure out what registers
         are used in the send set.
       */
      int nAddSets = 0;
      iCode *walk = ic->next;

      while (walk)
        {
          if (walk->op == SEND)
            {
              nAddSets++;
            }
          else if (walk->op == CALL || walk->op == PCALL)
            {
              /* Found it. */
              break;
            }
          else
            {
              /* Keep looking. */
            }
          walk = walk->next;
        }
      _saveRegsForCall (walk, nAddSets, FALSE);
    }
#endif

  /* then do the push */
  aopOp (IC_LEFT (ic), ic);

  for (size = IC_LEFT (ic)->aop->size, offset = 0; size;)
    {
      // TODO: For AOP_IMMD, if X is free, when optimizing for code size, ldw x, m  pushw x is better than push m push m+1.

      if (aopInReg (IC_LEFT (ic)->aop, offset, X_IDX) || aopInReg (IC_LEFT (ic)->aop, offset, Y_IDX))
        {
          push (IC_LEFT (ic)->aop, offset, 2);
          offset += 2;
          size -= 2;
        }
      else if (IC_LEFT (ic)->aop->type == AOP_LIT || aopInReg (IC_LEFT (ic)->aop, offset, A_IDX) || IC_LEFT (ic)->aop->type == AOP_DIR || IC_LEFT (ic)->aop->type == AOP_IMMD)
        {
          push (IC_LEFT (ic)->aop, offset, 1);
          offset++;
          size--;
        }
      else
        {
          cheapMove (ASMOP_A, 0, IC_LEFT (ic)->aop, offset, FALSE);
          push (ASMOP_A, 0, 1);
          offset++;
          size--;
        }
    }

  freeAsmop (IC_LEFT (ic));
}

static void
emitCall (const iCode *ic, bool ispcall)
{
  bool SomethingReturned, bigreturn;
  sym_link *dtype = operandType (IC_LEFT (ic));
  sym_link *etype = getSpec (dtype);
  sym_link *ftype = IS_FUNCPTR (dtype) ? dtype->next : dtype;

  // TODO: Parameters.

  /* Return value of big type or returning struct or union. */
  bigreturn = (getSize (ftype->next) > 2);
  if (bigreturn)
    {
      wassertl (0, "Unimplemented return value size.");
    }

  if (ispcall)
    {
      operand *left = IC_LEFT (ic);

      aopOp (left, ic);

      if (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD)
        {
          emitcode ("call", "%s", aopGet (left->aop, 0));
          cost (3, 4);
        }
      else
        {
          wassertl (0, "Unimplemented call through pointer.");
        }
      freeAsmop (left);
    }
  else
    {
      if (IS_LITERAL (etype))
        {
          emitcode ("call", "0x%04X", ulFromVal (OP_VALUE (IC_LEFT (ic))));
          cost (3, 4);
        }
      else
        {
          bool jump = (!ic->parmBytes && IFFUNC_ISNORETURN (OP_SYMBOL (IC_LEFT (ic))->type));
          emitcode (jump ? "jp" : "call", "%s",
            (OP_SYMBOL (IC_LEFT (ic))->rname[0] ? OP_SYMBOL (IC_LEFT (ic))->rname : OP_SYMBOL (IC_LEFT (ic))->name));
          cost (3, jump ? 1 : 4);
        }
    }

  SomethingReturned = (IS_ITEMP (IC_RESULT (ic)) &&
                       (OP_SYMBOL (IC_RESULT (ic))->nRegs || OP_SYMBOL (IC_RESULT (ic))->spildir))
                       || IS_TRUE_SYMOP (IC_RESULT (ic));

  if (ic->parmBytes || bigreturn)
    adjustStack (-ic->parmBytes - bigreturn * 2);

  /* if we need assign a result value */
  if (SomethingReturned && !bigreturn)
    {
      aopOp (IC_RESULT (ic), ic);

      // TODO: Implement this!

      freeAsmop (IC_RESULT (ic));
    }
}

/*-----------------------------------------------------------------*/
/* genCall - generates a call statement                            */
/*-----------------------------------------------------------------*/
static void
genCall (const iCode *ic)
{
  D (emitcode ("; genCall", ""));

  emitCall (ic, FALSE);
}

/*-----------------------------------------------------------------*/
/* genPCall - generates a call by pointer statement                */
/*-----------------------------------------------------------------*/
static void
genPCall (const iCode *ic)
{
  D (emitcode ("; genPcall", ""));

  emitCall (ic, TRUE);
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
genEndFunction (iCode *ic)
{
  symbol *sym = OP_SYMBOL (IC_LEFT (ic));

  wassert (!regalloc_dry_run);

  /* adjust the stack for the function */
  if (sym->stack)
    adjustStack (sym->stack);

  wassert (!_G.stack.pushed);

  if (IFFUNC_ISNAKED(sym->type))
  {
      D (emitcode (";", "naked function: no epilogue."));
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
/* genReturn - generate code for return statement                  */
/*-----------------------------------------------------------------*/
static void
genReturn (const iCode *ic)
{
  operand *left = IC_LEFT (ic);
  int size;

  D (emitcode ("; genReturn", ""));

  /* if we have no return value then
     just generate the "ret" */
  if (!IC_LEFT (ic))
    goto jumpret;

  /* we have something to return then
     move the return value into place */
  aopOp (left, ic);
  size = left->aop->size;

  switch (size)
    {
    case 1:
      cheapMove (ASMOP_A, 0, left->aop, 0, FALSE);
      break;
    case 2:
      genMove (ASMOP_X, left->aop, TRUE, TRUE, TRUE);
      break;
    default:
      wassertl (0, "Return not implemented for return value of this size.");
    }

  freeAsmop (left);

jumpret:
  /* generate a jump to the return label
     if the next is not the return statement */
  if (!(ic->next && ic->next->op == LABEL && IC_LABEL (ic->next) == returnLabel))
    {
      if (!regalloc_dry_run)
        emitcode ("jp", "%05d$", labelKey2num (returnLabel->key));
      cost (3, 1);
    }
}

/*-----------------------------------------------------------------*/
/* genLabel - generates a label                                    */
/*-----------------------------------------------------------------*/
static void
genLabel (const iCode *ic)
{
  D (emitcode ("; genLabel", ""));

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
  D (emitcode ("; genGoto", ""));

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

  D (emitcode ("; genPlus", ""));

  aopOp (IC_LEFT (ic), ic);
  aopOp (IC_RIGHT (ic), ic);
  aopOp (IC_RESULT (ic), ic);

  /* Swap if left is literal or right is in A. */
  if (left->aop->type == AOP_LIT || left->aop->type == AOP_IMMD || right->aop->size == 1 && aopInReg (right->aop, 1, A_IDX))
    {
      operand *t = right;
      right = left;
      left = t;
    }

  
  size = result->aop->size;
  for(i = 0, started = FALSE; i < size;)
    {
      if (!started &&
        (aopInReg (result->aop, i, X_IDX) || aopInReg (result->aop, i, Y_IDX)) &&
        (right->aop->type == AOP_LIT || right->aop->type == AOP_IMMD || aopOnStack (right->aop, i, 2)))
        {
          bool x = aopInReg (result->aop, i, X_IDX);
          genMove (x ? ASMOP_X : ASMOP_Y, left->aop, FALSE, x, !x); // TODO: Allow use of a sometimes.
          if (right->aop->type != AOP_LIT || byteOfVal (right->aop->aopu.aop_lit, i) || byteOfVal (right->aop->aopu.aop_lit, i + 1))
            {
              emitcode ("addw", x ? "x, %s" : "y, %s", aopGet (right->aop, i));
              cost ((x || aopOnStack (right->aop, 0, 2)) ? 3 : 4, 2);
              started = TRUE;
            }
          i += 2;
        }
      else if (!started && i == size - 2 && // We can use inc only for the only non-zero word, since it neither takes into account an existing carry nor does it update the carry.
        (aopInReg (result->aop, i, X_IDX) || aopInReg (result->aop, i, Y_IDX)) &&
        right->aop->type == AOP_LIT && byteOfVal (right->aop->aopu.aop_lit, i) == 1 && byteOfVal (right->aop->aopu.aop_lit, i + 1) == 0)
        {
          bool x = aopInReg (result->aop, i, X_IDX);
          emitcode ("incw", x ? "x" : "y");
          cost (x ? 1 : 2, 1);
          started = TRUE;
          i += 2;
        }
      else if (right->aop->type == AOP_REG || right->aop->type == AOP_REGSTK && !aopOnStack (right->aop, i, 1))
        {
          if (!regalloc_dry_run)
            wassertl (0, "Unimplemented addition operand.");
          cost (80, 80);
          i++;
        }
      else // TODO: Take care of A. TODO: Handling of right operands that can't be directly added to a.
        {
          cheapMove (ASMOP_A, 0, left->aop, i, FALSE);
          if (!started && right->aop->type == AOP_LIT && !byteOfVal (right->aop->aopu.aop_lit, i))
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

  D (emitcode ("; genMinus", ""));

  aopOp (IC_LEFT (ic), ic);
  aopOp (IC_RIGHT (ic), ic);
  aopOp (IC_RESULT (ic), ic);
  
  size = result->aop->size;
  for(i = 0, started = FALSE; i < size;)
    {
      if (0) // TODO: Use subw where it provides an advantage.
        ;
      else if (right->aop->type == AOP_REG || right->aop->type == AOP_REGSTK && !aopOnStack (right->aop, i, 1))
        {
          if (!regalloc_dry_run)
            wassertl (0, "Unimplemented subtraction operand.");
          cost (80, 80);
          i++;
        }
      else // TODO: Take care of A. TODO: Handling of right operands that can't be directly subtracted from a.
        {
          cheapMove (ASMOP_A, 0, left->aop, i, FALSE);
          if (!started && right->aop->type == AOP_LIT && !byteOfVal (right->aop->aopu.aop_lit, i))
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
/* exchangedCmp : returns the opcode need if the two operands are  */
/*                exchanged in a comparison                        */
/*-----------------------------------------------------------------*/
static int
exchangedCmp (int opcode)
{
  switch (opcode)
    {
    case '<':
      return '>';
    case '>':
      return '<';
    case LE_OP:
      return GE_OP;
    case GE_OP:
      return LE_OP;
    case NE_OP:
      return NE_OP;
    case EQ_OP:
      return EQ_OP;
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "opcode not a comparison");
    }
  return EQ_OP;                 /* shouldn't happen, but need to return something */
}

/*------------------------------------------------------------------*/
/* branchInstCmp : returns the conditional branch instruction that  */
/*                 will branch if the comparison is true            */
/*------------------------------------------------------------------*/
static char *
branchInstCmp (int opcode, int sign)
{
  switch (opcode)
    {
    case '<':
      if (sign)
        return "jrslt";
      else
        return "jrc";
    case '>':
      if (sign)
        return "jrsgt";
      else
        return "jrugt";
    case LE_OP:
      if (sign)
        return "jrsle";
      else
        return "jrule";
    case GE_OP:
      if (sign)
        return "jrsge";
      else
        return "jrnc";
    case NE_OP:
      return "jrne";
    case EQ_OP:
      return "jreq";
    default:
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "opcode not a comparison");
    }
  return "brn";
}

/*------------------------------------------------------------------*/
/* genCmp :- greater or less than (and maybe with equal) comparison */
/*------------------------------------------------------------------*/
static void
genCmp (iCode *ic)
{
  operand *result = IC_RESULT (ic);
  operand *left = IC_LEFT (ic);
  operand *right = IC_RIGHT (ic);
  sym_link *letype, *retype;
  int sign, opcode;
  int size, i;

  D (emitcode ("; genCmp", ""));

  opcode = ic->op;
  sign = 0;
  if (IS_SPEC (operandType (left)) && IS_SPEC (operandType (right)))
    {
      letype = getSpec (operandType (left));
      retype = getSpec (operandType (right));
      sign = !(SPEC_USIGN (letype) | SPEC_USIGN (retype));
    }

  aopOp (IC_LEFT (ic), ic);
  aopOp (IC_RIGHT (ic), ic);
  aopOp (IC_RESULT (ic), ic);

  /* Prefer literal operand on right */
  if (left->aop->type == AOP_LIT ||
    right->aop->type != AOP_LIT && left->aop->type == AOP_DIR ||
    (aopInReg (right->aop, 0, A_IDX) || aopInReg (right->aop, 0, X_IDX) || aopInReg (right->aop, 0, Y_IDX)) && left->aop->type == AOP_STK)
    {
      operand *temp = left;
      left = right;
      right = temp;
      opcode = exchangedCmp (opcode);
    }

  size = max (left->aop->size, right->aop->size);

  if (size == 1 &&
    right->aop->type == AOP_LIT || right->aop->type == AOP_DIR || right->aop->type == AOP_STK &&
    aopInReg (left->aop, 0, A_IDX))
    emit3 (A_CP, ASMOP_A, right->aop);
  else if (size == 2 &&
    right->aop->type == AOP_LIT || right->aop->type == AOP_DIR || right->aop->type == AOP_STK &&
    (aopInReg (left->aop, 0, X_IDX) || aopInReg (left->aop, 0, Y_IDX)))
    {
      if (aopInReg (left->aop, 0, Y_IDX) && right->aop->type == AOP_STK)
        {
          emitcode ("exgw", "x, y");
          emitcode ("cpw", "x, %s", aopGet (right->aop, 0));
          emitcode ("exgw", "x, y");
          cost (4, 4);
        }
      else
        {
          emitcode ("cpw", aopInReg (left->aop, 0, X_IDX) ? "x, %s" : "x, %s", aopGet (right->aop, 0));
          cost (3 + aopInReg (left->aop, 0, Y_IDX), 2);
        }
    }
  else
    {
      if (!regDead (A_IDX, ic))
        push (ASMOP_A, 0, 1);

      for (i = 0; i < size; i++)
        {
          cheapMove (ASMOP_A, 0, left->aop, i, FALSE);
          emit3_o (i ? A_SBC : A_SUB, ASMOP_A, 0, right->aop, i);
        }

      if (!regDead (A_IDX, ic))
        pop (ASMOP_A, 0, 1);
    }

  {
    symbol *tlbl1 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
    symbol *tlbl2 = (regalloc_dry_run ? 0 : newiTempLabel (NULL));

    emitcode (branchInstCmp (opcode, sign), "%05d$", labelKey2num (tlbl1));

    emitcode ("jp", "%05d$", labelKey2num (tlbl2));
    cost (3, 1);
    if (!regalloc_dry_run)
      emitLabel (tlbl1);
    if (!regalloc_dry_run)
      emitLabel (tlbl2);
  }

  freeAsmop (right);
  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genXor- code for xor                                            */
/*-----------------------------------------------------------------*/
static void
genXor (const iCode *ic)
{
  operand *left, *right, *result;
  int size, i;

  D (emitcode ("; genXor", ""));

  aopOp ((left = IC_LEFT (ic)), ic);
  aopOp ((right = IC_RIGHT (ic)), ic);
  aopOp ((result = IC_RESULT (ic)), ic);

  size = getSize (operandType (result));

  /* if left is a literal & right is not then exchange them */
  if (left->aop->type == AOP_LIT && right->aop->type != AOP_LIT || size == 1 && aopInReg (right->aop, 0, A_IDX))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  // TODO: Take care of operands partially in A. TODO: Handling of right operands that can't be directly xored with a. TODO: Use bit complement instructions where it is faster.
  if (!regDead (A_IDX, ic))
    push (ASMOP_A, 0, 1);
  for (i = 0; i < size; i++)
    {
      cheapMove (ASMOP_A, 0, left->aop, i, FALSE);
      emit3_o (A_XOR, ASMOP_A, 0, right->aop, i);
      cheapMove (result->aop, i, ASMOP_A, 0, FALSE);
    }
  if (!regDead (A_IDX, ic))
    pop (ASMOP_A, 0, 1);

  freeAsmop (left);
  freeAsmop (right);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genOr - code for or                                             */
/*-----------------------------------------------------------------*/
static void
genOr (const iCode *ic)
{
  operand *left, *right, *result;
  int size, i;

  D (emitcode ("; genOr", ""));

  aopOp ((left = IC_LEFT (ic)), ic);
  aopOp ((right = IC_RIGHT (ic)), ic);
  aopOp ((result = IC_RESULT (ic)), ic);

  size = getSize (operandType (result));

  /* if left is a literal & right is not then exchange them */
  if (left->aop->type == AOP_LIT && right->aop->type != AOP_LIT || size == 1 && aopInReg (right->aop, 0, A_IDX))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  // TODO: Take care of oparands partially in A. TODO: Handling of right operands that can't be directly ored with a. TODO: Use bit set instructions where it is faster.
  if (!regDead (A_IDX, ic))
    push (ASMOP_A, 0, 1);
  for (i = 0; i < size; i++)
    {
      cheapMove (ASMOP_A, 0, left->aop, i, FALSE);
      emit3_o (A_OR, ASMOP_A, 0, right->aop, i);
      cheapMove (result->aop, i, ASMOP_A, 0, FALSE);
    }
  if (!regDead (A_IDX, ic))
    pop (ASMOP_A, 0, 1);

  freeAsmop (left);
  freeAsmop (right);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genAnd - code for and                                           */
/*-----------------------------------------------------------------*/
static void
genAnd (const iCode *ic)
{
  operand *left, *right, *result;
  int size, i;

  D (emitcode ("; genAnd", ""));

  aopOp ((left = IC_LEFT (ic)), ic);
  aopOp ((right = IC_RIGHT (ic)), ic);
  aopOp ((result = IC_RESULT (ic)), ic);

  size = getSize (operandType (result));

  /* if left is a literal & right is not then exchange them */
  if (left->aop->type == AOP_LIT && right->aop->type != AOP_LIT || size == 1 && aopInReg (right->aop, 0, A_IDX))
    {
      operand *tmp = right;
      right = left;
      left = tmp;
    }

  // TODO: Take care of operands partially in A. TODO: Handling of right operands that can't be directly anded with a. TODO: Use bit reset instructions where it is faster.
  if (!regDead (A_IDX, ic))
    push (ASMOP_A, 0, 1);
  for (i = 0; i < size; i++)
    {
      cheapMove (ASMOP_A, 0, left->aop, i, FALSE);
      emit3_o (A_AND, ASMOP_A, 0, right->aop, i);
      cheapMove (result->aop, i, ASMOP_A, 0, FALSE);
    }
  if (!regDead (A_IDX, ic))
    pop (ASMOP_A, 0, 1);

  freeAsmop (left);
  freeAsmop (right);
  freeAsmop (result);
}

/*------------------------------------------------------------------*/
/* genLeftShiftLiteral - left shifting by known count for size <= 2 */
/*------------------------------------------------------------------*/
static void
genLeftShiftLiteral (operand *left, operand *right, operand *result, const iCode *ic)
{
  int shCount = (int) ulFromVal (right->aop->aopu.aop_lit);
  int size;

  D (emitcode ("; genLeftShiftLiteral", ""));

  size = getSize (operandType (result));

  /* I suppose that the left size >= result size */
  wassert (getSize (operandType (left)) >= size);

  if (shCount >= (size * 8))
    {
      aopOp (left, ic);
      aopOp (result, ic);

      while (size--)
        emit3_o (A_CLR, result->aop, size, 0, 0);
    }
  else
    {
      int i;

      wassertl (size <= 2, "Shifting of longs should be handled by generic function.");

      aopOp (left, ic);
      aopOp (result, ic);

      genMove (result->aop, left->aop, regDead (A_IDX, ic), regDead (X_IDX, ic), regDead (Y_IDX, ic));

      while (shCount--)
        for(i = 0; i < size;)
          {
            if (aopInReg (result->aop, i, X_IDX))
              {
                emitcode ("sllw", "x");
                cost (1, 2);
                i += 2;
              }
            else if (aopInReg (result->aop, i, Y_IDX))
              {
                emitcode ("sllw", "y");
                cost (2, 2);
                i += 2;
              }
            else
              {
                int swapidx = -1;
                if (aopRS (result->aop) && !aopInReg (result->aop, i, A_IDX) && result->aop->aopu.bytes[i].in_reg)
                  swapidx = result->aop->aopu.bytes[i].byteu.reg->rIdx;

                if (swapidx == -1)
                  emit3_o (i ? A_RLC : A_SLL, result->aop, i, 0, 0);
                else
                  {
                    if (!regalloc_dry_run)
                      wassertl (0, "Unimplemented left shift.");
                    cost (80, 80);
                  }

                i++;
              }
          }
    }

  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genLeftShift - generates code for left shifting                 */
/*-----------------------------------------------------------------*/
static void
genLeftShift (const iCode *ic)
{
  operand *left, *right, *result;

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (right, ic);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (right->aop->type == AOP_LIT && getSize (operandType (result)) <= 2)
    {
      genLeftShiftLiteral (left, right, result, ic);
      freeAsmop (right);
      return;
    }

  wassertl (0, "Unimplemented left shift by non-literal.");

  freeAsmop (right);
}

/*------------------------------------------------------------------*/
/* genRightShiftLiteral - right shifting by known count for size <= 2*/
/*------------------------------------------------------------------*/
static void
genRightShiftLiteral (operand *left, operand *right, operand *result, const iCode *ic)
{
  int shCount = (int) ulFromVal (right->aop->aopu.aop_lit);
  int size;

  D (emitcode ("; genRightShiftLiteral", ""));

  size = getSize (operandType (result));

  wassertl (SPEC_USIGN (getSpec (operandType (left))), "Unimplemented signed right shift.");

  /* I suppose that the left size >= result size */
  wassert (getSize (operandType (left)) >= size);

  if (shCount >= (size * 8))
    {
      aopOp (left, ic);
      aopOp (result, ic);

      while (size--)
        emit3_o (A_CLR, result->aop, size, 0, 0);
    }
  else
    {
      int i;

      wassertl (size <= 2, "Shifting of longs should be handled by generic function.");

      aopOp (left, ic);
      aopOp (result, ic);

      genMove (result->aop, left->aop, regDead (A_IDX, ic), regDead (X_IDX, ic), regDead (Y_IDX, ic));

      while (shCount--)
        for(i = size - 1; i >= 0;)
          {
            if (i > 0 && i == size - 1 && aopInReg (result->aop, i - 1, X_IDX))
              {
                emitcode ("srlw", "x");
                cost (1, 2);
                i -= 2;
              }
            else if (i > 0 && i == size - 1 && aopInReg (result->aop, i - 1, Y_IDX))
              {
                emitcode ("srlw", "y");
                cost (2, 2);
                i -= 2;
              }
            else
              {
                int swapidx = -1;
                if (aopRS (result->aop) && !aopInReg (result->aop, i, A_IDX) && result->aop->aopu.bytes[i].in_reg)
                  swapidx = result->aop->aopu.bytes[i].byteu.reg->rIdx;

                if (swapidx == -1)
                  emit3_o ((i != size - 1) ? A_RRC : A_SRL, result->aop, i, 0, 0);
                else
                  {
                    if (!regalloc_dry_run)
                      wassertl (0, "Unimplemented left shift.");
                    cost (80, 80);
                  }

                i--;
              }
          }
    }

  freeAsmop (left);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genRightShift - generates code for right shifting               */
/*-----------------------------------------------------------------*/
static void
genRightShift (const iCode *ic)
{
  operand *left, *right, *result;

  right = IC_RIGHT (ic);
  left = IC_LEFT (ic);
  result = IC_RESULT (ic);

  aopOp (right, ic);

  /* if the shift count is known then do it
     as efficiently as possible */
  if (right->aop->type == AOP_LIT && getSize (operandType (result)) <= 2)
    {
      genRightShiftLiteral (left, right, result, ic);
      freeAsmop (right);
      return;
    }

  wassertl (0, "Unimplemented right shift by non-literal.");

  freeAsmop (right);
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

  D (emitcode ("; genPointerGet", ""));

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

  D (emitcode ("; genAssign", ""));

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);

  aopOp (right, ic);
  aopOp (result, ic);

  genMove(result->aop, right->aop, regDead (A_IDX, ic), regDead (X_IDX, ic), regDead (Y_IDX, ic));

  freeAsmop (right);
  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genIfx - generate code for Ifx statement                        */
/*-----------------------------------------------------------------*/
static void
genIfx (const iCode *ic)
{
  // TODO: This function currently reports code size costs only, other costs will depend on profiler information.

  operand *const cond = IC_COND (ic);
  symbol *const tlbl = (regalloc_dry_run ? 0 : newiTempLabel (NULL));
  aopOp (cond, ic);

  D (emitcode ("; genIfx", ""));

  if (IS_BOOL (operandType (cond)) && cond->aop->type == AOP_DIR)
    {
      if (tlbl)
        emitcode (IC_FALSE (ic) ? "btjt" : "btjf", "%s, #0, !tlabel", aopGet (cond->aop, 0), labelKey2num (tlbl));
      cost (5, 0);
    }
  else if (cond->aop->size == 1 && (aopRS (cond->aop) || cond->aop->type == AOP_DIR))
    {
      // Need to swap when operand is in part of x or y.
      int swapidx = -1;
      if (aopRS (cond->aop) && !aopInReg (cond->aop, 0, A_IDX) && cond->aop->aopu.bytes[0].in_reg)
          swapidx = cond->aop->aopu.bytes[0].byteu.reg->rIdx;

      if (swapidx != -1)
        swap_to_a (swapidx);

      emit3 (A_TNZ, swapidx == -1 ? cond->aop : ASMOP_A, 0);

      if (swapidx != -1)
        swap_from_a (swapidx);

      if (tlbl)
        emitcode (IC_FALSE (ic) ? "jrne" : "jreq", "!tlabel", labelKey2num (tlbl));
      cost (2, 0);
    }
  else if (aopInReg (cond->aop, 0, C_IDX))
    {
      wassertl (IS_BOOL (operandType (cond)), "Variable of type other than _Bool in carry bit.");
      if (tlbl)
        emitcode (IC_FALSE (ic) ? "jrc" : "jrnc", "!tlabel", labelKey2num (tlbl));
      cost (2, 0);
    }
  else if (cond->aop->size == 2 && (aopInReg (cond->aop, 0, X_IDX) || aopInReg (cond->aop, 0, Y_IDX)))
    {
      emitcode ("tnzw", aopInReg (cond->aop, 0, X_IDX) ? "x" : "y");
      cost (aopInReg (cond->aop, 0, X_IDX) ? 1 : 2, 1);
      if (tlbl)
        emitcode (IC_FALSE (ic) ? "jrne" : "jreq", "!tlabel", labelKey2num (tlbl));
      cost (2, 0);
    }
  else
    {
      int i;

      if (!regDead (A_IDX, ic))
        push (A_IDX, 0, 1);

      cheapMove (ASMOP_A, 0, cond->aop, 0, FALSE);
      for (i = 1; i < cond->aop->size; i++)
        emit3_o (A_OR, ASMOP_A, 0, cond->aop, i);

      if (!regDead (A_IDX, ic))
        pop (A_IDX, 0, 1);

      if (tlbl)
        emitcode (IC_FALSE (ic) ? "jrne" : "jreq", "!tlabel", labelKey2num (tlbl));
      cost (2, 0);
    }

  if (tlbl)
    {
      emitcode ("jp", "!tlabel", labelKey2num ((IC_TRUE (ic) ? IC_TRUE (ic) : IC_FALSE (ic))->key));
      cost (3, 0);
      emitLabel (tlbl);
    }

  freeAsmop (cond);
}

/*-----------------------------------------------------------------*/
/* genAddrOf - generates code for address of                       */
/*-----------------------------------------------------------------*/
static void
genAddrOf (const iCode *ic)
{
  symbol *sym;
  operand *result, *left;

  D (emitcode ("; genAddrOf", ""));

  result = IC_RESULT (ic);
  left = IC_LEFT (ic);

  wassert (result);
  wassert (left);
  wassert (IS_TRUE_SYMOP (left));
  sym = OP_SYMBOL (left);
  wassert (sym);

  aopOp (result, ic);

  // TODO: When optimizing for size, putting on-stack address into y when y is free is cheaper calculating in x, then using exgw.
  if (aopInReg (result->aop, 0, Y_IDX) || regDead (Y_IDX, ic) && !regDead (X_IDX, ic))
    {
      if (!sym->onStack)
        {
          wassert (sym->name);
          emitcode ("ldw", "y, #%s", sym->name);
          cost (4, 2);
        }
      else
        {
          emitcode ("ldw", "y, sp");
          emitcode ("addw", "y, #%d", sym->stack + _G.stack.pushed);
          cost (6, 3);
        }
      genMove (result->aop, ASMOP_Y, regDead (A_IDX, ic), FALSE, regDead (X_IDX, ic));
    }
  else // TODO: Handle case of both X and Y alive; TODO: Use mov when destination is a global variable.
    {
      if (!sym->onStack)
        {
          wassert (sym->name);
          emitcode ("ldw", "x, #%s", sym->name);
          cost (3, 2);
        }
      else
        {
          emitcode ("ldw", "x, sp");
          emitcode ("addw", "x, #%d", sym->stack + _G.stack.pushed);
          cost (4, 3);
        }
      genMove (result->aop, ASMOP_X, regDead (A_IDX, ic), TRUE, regDead (Y_IDX, ic));
    }

  freeAsmop (result);
}

/*-----------------------------------------------------------------*/
/* genCast - gen code for casting                                  */
/*-----------------------------------------------------------------*/
static void
genCast (const iCode *ic)
{
  operand *result, *right;
  int size, offset;
  sym_link *rtype;

  D (emitcode ("; genCast", ""));

  result = IC_RESULT (ic);
  right = IC_RIGHT (ic);
  rtype = operandType (right);

  aopOp (right, ic);
  aopOp (result, ic);

  if (result->aop->size <= right->aop->size && (!IS_BOOL (operandType (result)) || IS_BOOL (operandType (right))))
    {
      freeAsmop (right);
      freeAsmop (result);
      genAssign (ic);
      return;
    }
  else if (!IS_BOOL (operandType (result)))
    {
      size = right->aop->size;
      offset = 0;
      while (size--)
        {
          cheapMove (result->aop, offset, right->aop, offset, TRUE); // TODO: Relax restriction on A.
          offset++;
        }

      /* now depending on the sign of the destination */
      size = result->aop->size - right->aop->size;

      /* Unsigned or not an integral type - right fill with zeros */
      if (!IS_SPEC (rtype) || SPEC_USIGN (rtype))
        {
          while (size--)
            cheapMove (result->aop, offset++, ASMOP_ZERO, 0, TRUE); // TODO: Relax restriction on A.
        }
      else
        wassertl (0, "Unimplemented big signed cast.");
    }
  else
    wassertl (0, "Unimplemented cast to _Bool.");

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
      wassertl (0, "Unimplemented iCode");
      break;

    case IPUSH:
      genIpush (ic);
      break;

    case IPOP:
      wassertl (0, "Unimplemented iCode");
      break;

    case CALL:
      genCall (ic);
      break;

    case PCALL:
      genPCall (ic);
      break;

    case FUNCTION:
      genFunction (ic);
      break;

    case ENDFUNCTION:
      genEndFunction (ic);
      break;

    case RETURN:
      genReturn (ic);
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
      wassertl (0, "Unimplemented iCode");
      break;

    case '>':
    case '<':
      genCmp(ic);
      break;

    case LE_OP:
    case GE_OP:
    case NE_OP:

      /* note these two are xlated by algebraic equivalence
         during parsing SDCC.y */
      werror (E_INTERNAL_ERROR, __FILE__, __LINE__, "got '>=' or '<=' shouldn't have come here");
      break;

    case EQ_OP:
      wassertl (0, "Unimplemented iCode");
      break;

    case AND_OP:
    case OR_OP:
      wassertl (0, "Unimplemented iCode");
      break;

    case '^':
      genXor (ic);
      break;

    case '|':
      genOr (ic);
      break;

    case BITWISEAND:
      genAnd (ic);
      break;

    case INLINEASM:
      wassertl (0, "Unimplemented iCode");
      break;

    case RRC:
    case RLC:
      wassertl (0, "Unimplemented iCode");
      break;

    case GETHBIT:
    case GETABIT:
      wassertl (0, "Unimplemented iCode");
      break;

    case LEFT_OP:
      genLeftShift (ic);
      break;

    case RIGHT_OP:
      genRightShift (ic);
      break;

    case GET_VALUE_AT_ADDRESS:
      genPointerGet (ic);
      break;

    case '=':
      genAssign (ic);
      break;

    case IFX:
      genIfx (ic);
      break;

    case ADDRESS_OF:
      genAddrOf (ic);
      break;

    case JUMPTABLE:
      wassertl (0, "Unimplemented iCode");
      break;

    case CAST:
      genCast (ic);
      break;

    case RECEIVE:
    case SEND:
      wassertl (0, "Unimplemented iCode");
      break;

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

