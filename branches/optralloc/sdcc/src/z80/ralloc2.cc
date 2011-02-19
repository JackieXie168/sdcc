// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2010 - 2011
//
// (c) 2010-2011 Goethe-Universität Frankfurt
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// An optimal, polynomial-time register allocator.

#include "SDCCralloc.hpp"

#define REG_C 0
#define REG_B 1
#define REG_E 2
#define REG_D 3
#define REG_L 4
#define REG_H 5
#define REG_A (NUM_REGS - 1)

template <class G_t, class I_t>
float default_operand_cost(const operand *o, const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  float c = 0.0f;

  std::multimap<int, var_t>::const_iterator oi, oi_end;

  var_t byteregs[4];	// Todo: Change this when sdcc supports variables larger than 4 bytes.
  unsigned short int size;

  if(o && IS_SYMOP(o))
    {
      boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key);
      if(oi != oi_end)
        {
          var_t v = oi->second;

          // In registers.
          if(a.local.find(v) != a.local.end())
            {
              c += 1.0f;
              byteregs[I[v].byte] = a.global[v];
              size = 1;

              while(++oi != oi_end)
                {
                  v = oi->second;
                  c += (a.local.find(v) != a.local.end() ? 1.0f : std::numeric_limits<float>::infinity());
                  byteregs[I[v].byte] = a.global[v];
                  size++;
                }

              // Penalty for not placing 2- and 4-byte variables in register pairs
              // Todo: Extend this once the register allcoator can use registers other than bc, de:
              if((size == 2 || size == 4) && (byteregs[1] != byteregs[0] + 1 || byteregs[0] != REG_C && byteregs[0] != REG_E && byteregs[0] != REG_L))
                c += 2.0f;
              if(size == 4 && (byteregs[3] != byteregs[2] + 1 || byteregs[2] != REG_C && byteregs[2] != REG_E && byteregs[0] != REG_L))
                c += 2.0f;

              // Code generator cannot handle variables only partially in A.
              if(OPTRALLOC_A && size > 1)
                for(unsigned short int i = 0; i < size; i++)
                  if(byteregs[i] == REG_A)
                    c += std::numeric_limits<float>::infinity();

              if(OPTRALLOC_A && byteregs[0] == REG_A)
                c -= 0.4f;
              else if(OPTRALLOC_HL && byteregs[0] == REG_L)
                c -= 0.1f;
            }
          // Spilt.
          else
            {
              c += 4.0f;
              while(++oi != oi_end)
                {
                  v = oi->second;
                  c += (a.local.find(v) == a.local.end() ? 4.0f : std::numeric_limits<float>::infinity());
                }
            }
        }
    }

  return(c);
}

template <class G_t, class I_t> static float
default_instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  float c = 0.0f;

  const iCode *ic = G[i].ic;

  c += default_operand_cost(IC_RESULT(ic), a, i, G, I);
  c += default_operand_cost(IC_LEFT(ic), a, i, G, I);
  c += default_operand_cost(IC_RIGHT(ic), a, i, G, I);

  return(c);
}

// Treat assignment separately to handle coalescing.
template <class G_t, class I_t> static float
assign_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  float c = 0.0f;

  const iCode *ic = G[i].ic;

  const operand *right = IC_RIGHT(ic);
  const operand *result = IC_RESULT(ic);

  if(!right || !IS_SYMOP(right) || !result || !IS_SYMOP(result))
    return(default_instruction_cost(a, i, G, I));

  reg_t byteregs[4] = {-1, -1, -1, -1};	// Todo: Change this when sdcc supports variables larger than 4 bytes.

  std::multimap<int, var_t>::const_iterator oi, oi_end;

  int size1 = 0, size2 = 0;

  boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(right)->key);
  if(oi != oi_end)
    {
      var_t v = oi->second;

      if(a.local.find(v) == a.local.end())
        return(default_instruction_cost(a, i, G, I));

      c += 1.0f;
      byteregs[I[v].byte] = a.global[v];
      size1 = 1;

      while(++oi != oi_end)
        {
          v = oi->second;
          c += (a.local.find(v) != a.local.end() ? 1.0f : std::numeric_limits<float>::infinity());
          byteregs[I[v].byte] = a.global[v];
          size1++;
        }

      // Code generator cannot handle variables only partially in A.
      if(OPTRALLOC_A && size1 > 1)
        for(unsigned short int i = 0; i < size1; i++)
          if(byteregs[i] == REG_A)
            c += std::numeric_limits<float>::infinity();

      if(OPTRALLOC_A && byteregs[0] == REG_A)
        c -= 0.4f;
    }

  if(!size1)
    return(default_instruction_cost(a, i, G, I));

  boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(result)->key);
  if(oi != oi_end)
    {
      var_t v = oi->second;

      if(a.local.find(v) == a.local.end())
        return(default_instruction_cost(a, i, G, I));

      c += 1.0f;
      if(byteregs[I[v].byte] == a.global[v])
        c -= 2.0f;
      size2 = 1;

      while(++oi != oi_end)
        {
          v = oi->second;
          c += (a.local.find(v) != a.local.end() ? 1.0f : std::numeric_limits<float>::infinity());
          if(byteregs[I[v].byte] == a.global[v])
            c -= 2.0f;
          size2++;
        }

      if(OPTRALLOC_A && byteregs[0] == REG_A)
        c -= 0.4f;
    }

  if(!size2)
    return(default_instruction_cost(a, i, G, I));

  return(c);
}

template <class G_t, class I_t> static float
ifx_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;

  return(default_operand_cost(IC_COND(ic), a, i, G, I));
}

template <class G_t, class I_t> static float
jumptab_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;

  return(default_operand_cost(IC_JTCOND(ic), a, i, G, I));
}

// This serves to avoid operations that overwrite their own operands before using them, e.g. x = y | a, with x_0 placed in the same reg as y_1.
// Code generation for such cases is broken (and the fix is ugly) so better avoid them.
template <class G_t, class I_t> static bool
result_overwrites_operand(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;
	
  const operand *result = IC_RESULT(ic);
  const operand *left = IC_LEFT(ic);
  const operand *right = IC_RIGHT(ic);
	
  if(!result || !IS_SYMOP(result))
    return(false);
	
  std::multimap<int, var_t>::const_iterator oir, oir_end, oirs;
	
  boost::tie(oir, oir_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(result)->key);
  if(oir == oir_end)
    return(false);
		
  std::multimap<int, var_t>::const_iterator oio, oio_end;
	
  if(left && IS_SYMOP(left))
    for(boost::tie(oio, oio_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(left)->key); oio != oio_end; ++oio)
      for(oirs = oir; oirs != oir_end; ++oirs)
        {
          var_t rvar = oirs->second;
          var_t ovar = oio->second;
          reg_t rreg = a.global[rvar];
          reg_t oreg = a.global[ovar];
      	  if(rreg >= 0 && oreg >= 0 && rreg == oreg && I[rvar].byte < I[ovar].byte)
      		return(true);
        }

  if(right && IS_SYMOP(right))
    for(boost::tie(oio, oio_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(right)->key); oio != oio_end; ++oio)
      for(oirs = oir; oirs != oir_end; ++oirs)
        {
          var_t rvar = oirs->second;
          var_t ovar = oio->second;
          reg_t rreg = a.global[rvar];
          reg_t oreg = a.global[ovar];
      	  if(rreg >= 0 && oreg >= 0 && rreg == oreg && I[rvar].byte < I[ovar].byte)
      		return(true);
        }
        
  return(false);
}

template <class G_t, class I_t> static float
or_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  if(result_overwrites_operand(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

  return(default_instruction_cost(a, i, G, I));
}

// Return true, iff the operand is placed (partially) in r.
template <class G_t>
bool operand_in_reg(const operand *o, reg_t r, const i_assignment &ia, unsigned short int i, const G_t &G)
{
  if(!o || !IS_SYMOP(o))
    return(false);

  std::multimap<int, var_t>::const_iterator oi, oi_end;
  for(boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key); oi != oi_end; ++oi)
    if(oi->second == ia.registers[r][1] || oi->second == ia.registers[r][0])
      return(true);

  return(false);
}

template <class G_t, class I_t>
bool Ainst_ok(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;

  std::map<int, i_assignment>::const_iterator iai = a.i_assignments.find(i);
  if(iai == a.i_assignments.end())
    {
      std::cerr << "ERROR: Instruction assignment not found.\n";
      return(false);
    }
  const i_assignment &ia = iai->second;

  if(ia.registers[REG_A][1] < 0)
    return(true);	// Register A not in use.

  //if(i == 15) std::cout << "Ainst_ok: A = (" << ia.registers[REG_A][0] << ", " << ia.registers[REG_A][1] << "), inst " << i << ", " << ic->key << "\n";
  if(I[ia.registers[REG_A][1]].byte)
    {
      //if(i == 15) std::cout << "Byte: Dropping at " << i << ", " << ic->key << "(" << int(ic->op) << "\n";
      return(false);
    }

  // Check if the result of this instruction is placed in A.
  bool result_in_A = operand_in_reg(IC_RESULT(ic), REG_A, ia, i, G);

  // Check if an input of this instruction is placed in A.
  bool input_in_A;
  switch(ic->op)
    {
    case IFX:
      input_in_A = operand_in_reg(IC_COND(ic), REG_A, ia, i, G);
      break;
    case JUMPTABLE:
      input_in_A = operand_in_reg(IC_JTCOND(ic), REG_A, ia, i, G);
      break;
    default:
      input_in_A = operand_in_reg(IC_LEFT(ic), REG_A, ia, i, G) || operand_in_reg(IC_RIGHT(ic), REG_A, ia, i, G);
      break;
    }

  if(!result_in_A && !input_in_A)
    {
      // Variable in A is not used by this instruction
      if(ic->op == '+' && IS_ITEMP (IC_LEFT (ic)) && IS_ITEMP (IC_RESULT (ic)) && IS_OP_LITERAL (IC_RIGHT (ic)) &&
          ulFromVal (OP_VALUE (IC_RIGHT (ic))) == 1 &&
          OP_KEY (IC_RESULT (ic)) == OP_KEY (IC_LEFT (ic)))
        return(true);

      if(ic->op == '=' && !POINTER_SET (ic) && isOperandEqual(IC_RESULT(ic), IC_RIGHT(ic)))
        return(true);

      if(ic->op == GOTO || ic->op == LABEL)
        return(true);

      //if(i == 15) std::cout << "Not Used: Dropping at " << i << ", " << ic->key << "(" << int(ic->op) << "\n";
      return(false);
    }

  // Last use of operand in A.
  const std::set<var_t> &dying = G[i].dying;
  if(input_in_A && (result_in_A || dying.find(ia.registers[REG_A][1]) != dying.end() || dying.find(ia.registers[REG_A][0]) != dying.end()))
    {
      if(ic->op != IFX &&
          !((ic->op == RIGHT_OP || ic->op == LEFT_OP) && IS_OP_LITERAL(IC_RIGHT(ic))) &&
          !(ic->op == '=' && !(IY_RESERVED && POINTER_SET(ic))) &&
          !IS_BITWISE_OP (ic) &&
          !((ic->op == '-' || ic->op == '+' || ic->op == EQ_OP) && IS_OP_LITERAL(IC_RIGHT(ic))))
        {
          //if(i == 15) std::cout << "Last use: Dropping at " << i << ", " << ic->key << "(" << int(ic->op) << ")\n";
          return(false);
        }
    }
  // A is used, and has to be preserved for later use.
  else if(input_in_A &&
          ic->op != IFX &&
          ic->op != JUMPTABLE)
    {
      //if(i == 15) std::cout << "Intermediate use: Dropping at " << i << ", " << ic->key << "(" << int(ic->op) << "\n";
      return(false);
    }

  // First use of operand in A.
  if(result_in_A &&
      !POINTER_GET(ic) &&
      ic->op != '+' &&
      ic->op != '-' &&
      !IS_BITWISE_OP(ic) &&
      ic->op != '=' &&
      ic->op != EQ_OP &&
      ic->op != '<' &&
      ic->op != '>' &&
      ic->op != CAST &&
      ic->op != CALL &&
      ic->op != PCALL &&
      ic->op != GETHBIT &&
      !((ic->op == LEFT_OP || ic->op == RIGHT_OP) && IS_OP_LITERAL(IC_RIGHT(ic))))
    {
      //if(i == 15) std::cout << "First use: Dropping at " << i << ", " << ic->key << "(" << int(ic->op) << "\n";
      return(false);
    }

  //if(i == 15) std::cout << "Default OK\n";

  return(true);
}

template <class G_t, class I_t>
bool HLinst_ok(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;

  // HL always unused on gbz80.
  if(TARGET_IS_GBZ80)
    return(true);

  std::map<int, i_assignment>::const_iterator iai = a.i_assignments.find(i);
  if(iai == a.i_assignments.end())
    {
      std::cerr << "ERROR: Instruction assignment not found.\n";
      return(false);
    }
  const i_assignment &ia = iai->second;

  bool unused_L = (ia.registers[REG_L][1] < 0);
  bool unused_H = (ia.registers[REG_H][1] < 0);

  if(unused_L && unused_H)
    return(true);	// Register HL not in use.

  //if(ic->key == 95) std::cout << "HLinst_ok: L = (" << ia.registers[REG_L][0] << ", " << ia.registers[REG_L][1] << "), H = (" << ia.registers[REG_H][0] << ", " << ia.registers[REG_H][1] << ")inst " << i << ", " << ic->key << "\n";

  bool result_in_L = operand_in_reg(IC_RESULT(ic), REG_L, ia, i, G);
  bool result_in_H = operand_in_reg(IC_RESULT(ic), REG_H, ia, i, G);
  bool result_in_HL = result_in_L || result_in_H;

  bool input_in_L, input_in_H;
  switch(ic->op)
    {
    case IFX:
      input_in_L = operand_in_reg(IC_COND(ic), REG_L, ia, i, G);
      input_in_H = operand_in_reg(IC_COND(ic), REG_L, ia, i, G);
      break;
    case JUMPTABLE:
      input_in_L = operand_in_reg(IC_JTCOND(ic), REG_L, ia, i, G);
      input_in_H = operand_in_reg(IC_JTCOND(ic), REG_L, ia, i, G);
      break;
    default:
      input_in_L = operand_in_reg(IC_LEFT(ic), REG_L, ia, i, G) || operand_in_reg(IC_RIGHT(ic), REG_L, ia, i, G);
      input_in_H = operand_in_reg(IC_LEFT(ic), REG_H, ia, i, G) || operand_in_reg(IC_RIGHT(ic), REG_H, ia, i, G);
      break;
    }
  bool input_in_HL = input_in_L || input_in_H;

  const std::set<var_t> &dying = G[i].dying;
  bool dying_L = result_in_L || dying.find(ia.registers[REG_L][1]) != dying.end() || dying.find(ia.registers[REG_L][0]) != dying.end();
  bool dying_H = result_in_H || dying.find(ia.registers[REG_H][1]) != dying.end() || dying.find(ia.registers[REG_H][0]) != dying.end();

  bool result_only_HL = (result_in_L || unused_L || dying_L) && (result_in_H || unused_H || dying_H);

#if 0
  {
    std::cout << "Result: ";
    std::multimap<int, var_t>::const_iterator oi, oi_end;
    for(boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(IC_RESULT(ic))->key); oi != oi_end; ++oi)
      std::cout << oi->second << " ";
    std::cout << "\n";
    print_assignment(a);
    std::cout << "\n";
  }
#endif
  //if(ic->key == 95) std::cout << "Result in L: " << result_in_L << ", result in H: " << result_in_H << "\n";

  if(ic->op == RETURN)
    return(true);

  if((IS_GB || IY_RESERVED) && (IS_TRUE_SYMOP(IC_LEFT(ic)) || IS_TRUE_SYMOP(IC_RIGHT(ic))))
    return(false);

  // HL overwritten by result.
  if(result_only_HL && !POINTER_SET(ic) &&
      (ic->op == ADDRESS_OF ||
       ic->op == '+' ||
       ic->op == '='))
    return(true);
//std::cout << "DS?\n";
  if(IC_RESULT(ic) && IS_SYMOP(IC_RESULT(ic)) && isOperandInDirSpace(IC_RESULT(ic)))
    return(false);
//std::cout << "DS1\n";
  if((input_in_HL || !result_only_HL) && IC_LEFT(ic) && IS_SYMOP(IC_LEFT(ic)) && isOperandInDirSpace(IC_LEFT(ic)))
    return(false);
//std::cout << "DS2\n";
  if((input_in_HL || !result_only_HL) && IC_RIGHT(ic) && IS_SYMOP(IC_RIGHT(ic)) && isOperandInDirSpace(IC_RIGHT(ic)))
    return(false);
//std::cout << "!DS\n";

//std::cout << "HL2\n";

  // Operations that leave HL alone.
  if(ic->op == IFX)
    return(true);
  if(SKIP_IC2(ic))
    return(true);
  if(ic->op == IPUSH && input_in_H)
    return(true);
  if(POINTER_GET(ic) && input_in_L && input_in_H && (getSize(operandType(IC_RESULT(ic))) == 1 || !result_in_HL))
    return(true);
  if(ic->op == LEFT_OP && isOperandLiteral(IC_RIGHT(ic)))
    return(true);

  if((!POINTER_SET(ic) && !POINTER_GET(ic) && (
        (ic->op == '=' ||
         ic->op == CAST ||
         /*ic->op == UNARYMINUS ||*/
         ic->op == RIGHT_OP ||
         /*ic->op == '-' ||*/
         IS_BITWISE_OP(ic) ||
         /*ic->op == '>' ||
         ic->op == '<' ||
         ic->op == EQ_OP ||*/
         (ic->op == '+' && getSize(operandType(IC_RESULT(ic))) == 1) ||
         (ic->op == '+' && getSize(operandType(IC_RESULT(ic))) <= 2 && result_only_HL) ))))	// 16 bit addition might use add hl, rr
    return(true);

  if(IS_VALOP(IC_RIGHT(ic)) && ic->op == EQ_OP)
    return(true);

  //if(ic->op == '=' && POINTER_SET(ic) && getSize(operandType(IC_RIGHT(ic))) > 1)
  //	return(false);

  // HL overwritten by result.
  if(result_only_HL && (ic->op == CALL || ic->op == PCALL))
    return(true);

  if(ic->op == '=' && !POINTER_GET(ic) && !input_in_HL)
    return(true);

  /*if(ic->key >= 94 && ic->key <= 96)
  {
  	std::cout << "HLinst_ok: L = (" << ia.registers[REG_L][0] << ", " << ia.registers[REG_L][1] << "), H = (" << ia.registers[REG_H][0] << ", " << ia.registers[REG_H][1] << ")inst " << i << ", " << ic->key << "\n";
  	std::cout << "Result in L: " << result_in_L << ", result in H: " << result_in_H << "\n";
  	std::cout << "HL default drop at " << ic->key << ", operation: " << ic->op << "\n";
  }*/

  return(false);
}

// Cost function.
template <class G_t, class I_t>
float instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;

  if(OPTRALLOC_A && !Ainst_ok(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

  if(OPTRALLOC_HL && !HLinst_ok(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

  switch(ic->op)
    {
    case '=':
      return(assign_cost(a, i, G, I));
    case IFX:
      return(ifx_cost(a, i, G, I));
    case JUMPTABLE:
      return(jumptab_cost(a, i, G, I));
    case '|':
      return(or_cost(a, i, G, I));
    default:
      return(default_instruction_cost(a, i, G, I));
    }
}

template <class I_t>
float weird_byte_order(const assignment &a, const I_t &I)
{
  float c = 0.0f;
  
  std::set<var_t>::const_iterator vi, vi_end;
  for(vi = a.local.begin(), vi_end = a.local.end(); vi != vi_end; ++vi)
    if(a.global[*vi] % 2 != I[*vi].byte % 2)
      c += 8.0f;

  return(c);
}

// Check for gaps, i.e. higher bytes of a variable being assigned to regs, while lower byte are not.
template <class I_t>
bool local_assignment_insane(const assignment &a, const I_t &I, var_t lastvar)
{
  std::set<var_t>::const_iterator v, v_end, v_old;
  
  for(v = a.local.begin(), v_end = a.local.end(); v != v_end;)
    {
      v_old = v;
      ++v;
      if(v == v_end)
        {
          if(*v_old != lastvar && I[*v_old].byte != I[*v_old].size - 1)
            return(true);
          break;
        }
      if(I[*v_old].v == I[*v].v)
        {
          if(I[*v_old].byte != I[*v].byte - 1)
            return(true);
        }
      else
        {
          if(*v_old != lastvar && I[*v_old].byte != I[*v_old].size - 1 || I[*v].byte)
            return(true);
        }
    }
	
  return(false);
}

template <class G_t, class I_t>
float rough_cost_estimate(const assignment &a, unsigned short int i, const G_t &G, const I_t &I, var_t lastvar)
{
  // Can check for Ainst_ok() since A only contains 1-byte variables.
  if(OPTRALLOC_A && !Ainst_ok(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

  std::map<int, i_assignment>::const_iterator iai = a.i_assignments.find(i);
  if(iai == a.i_assignments.end())
    {
      std::cerr << "ERROR: Instruction assignment not found.\n";
      return(250.0f);
    }
  const i_assignment &ia = iai->second;

  if(local_assignment_insane(a, I, lastvar))
    return(std::numeric_limits<float>::infinity());

  // Can only check for HLinst_ok() in some cases.
  if(OPTRALLOC_HL &&
      (ia.registers[REG_L][1] >= 0 && ia.registers[REG_H][1] >= 0) &&
      !((ia.registers[REG_L][0] >= 0) ^ (ia.registers[REG_H][0] >= 0)) &&
      !HLinst_ok(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

  // The code generator could deal with this for '+' in some cases though.
  const iCode *ic = G[i].ic;
  if((ic->op == '|' || ic->op == '&' || ic->op == '^' || ic->op == '+' || ic->op == '-') && result_overwrites_operand(a, i, G, I))
    return(std::numeric_limits<float>::infinity());
    
  float c = 0.0f;

  c += weird_byte_order(a, I);

  if(OPTRALLOC_A && ia.registers[REG_A][1] < 0)
    c += 0.03f;

  if(OPTRALLOC_HL && ia.registers[REG_L][1] < 0)
    c += 0.02f;

  // An artifical ordering of assignments.
  if(ia.registers[REG_E][1] < 0)
    c += 0.0001;
  if(ia.registers[REG_D][1] < 0)
    c += 0.00001;

  if(a.marked)
    c -= 0.5f;

  std::set<var_t>::const_iterator v, v_end;
  for(v = a.local.begin(), v_end = a.local.end(); v != v_end; ++v)
    c -= *v * 0.01f;

  return(c - a.local.size() * 0.2f);
}

template <class T_t, class G_t, class I_t>
void tree_dec_ralloc(T_t &T, const G_t &G, const I_t &I)
{
  con2_t I2(boost::num_vertices(I));
  for(unsigned int i = 0; i < boost::num_vertices(I); i++)
    {
      I2[i].v = I[i].v;
      I2[i].byte = I[i].byte;
      I2[i].size = I[i].size;
      I2[i].name = I[i].name;
    }
  typename boost::graph_traits<I_t>::edge_iterator e, e_end;
  for(boost::tie(e, e_end) = boost::edges(I); e != e_end; ++e)
    add_edge(boost::source(*e, I), boost::target(*e, I), I2);

  const assignment ac;
  tree_dec_ralloc_nodes(T, find_root(T), G, I2, ac);

  const assignment &winner = *(T[find_root(T)].assignments.begin());

  /*std::cout << "Winner: ";
  for(unsigned int i = 0; i < boost::num_vertices(I); i++)
  {
  	std::cout << "(" << i << ", " << int(winner.global[i]) << ") ";
  }
  std::cout << "\n";*/

  // Todo: Make this an assertion
  if(winner.global.size() != boost::num_vertices(I))
    std::cerr << "ERROR: No Assignments at root\n";

  for(unsigned int v = 0; v < boost::num_vertices(I); v++)
    {
      symbol *sym = (symbol *)(hTabItemWithKey(liveRanges, I[v].v));
      if(winner.global[v] >= 0)
        {
          if(winner.global[v] != REG_A || !OPTRALLOC_A)
            sym->regs[I[v].byte] = regsZ80 + winner.global[v];
          else
            {
              sym->accuse = ACCUSE_A;
              sym->nRegs = 0;
            }
        }
      else
        spillThis(sym);
    }
}

void z80_ralloc2_cc(ebbIndex *ebbi)
{
  //std::cout << "Processing " << currFunc->name << " from " << dstFileName << "\n"; std::cout.flush();
  //std::cout << "OPTRALLOC_ALL: " << OPTRALLOC_ALL << " OPTRALLOC_A: " << OPTRALLOC_A << "\n";

  cfg_t control_flow_graph;

  con_t conflict_graph;

  create_cfg(control_flow_graph, conflict_graph, ebbi);

  if(z80_opts.dump_graphs)
    dump_cfg(control_flow_graph);

  if(z80_opts.dump_graphs)
    dump_con(conflict_graph);

  tree_dec_t tree_decomposition;

  thorup_tree_decomposition(tree_decomposition, control_flow_graph);

  nicify(tree_decomposition);

  alive_tree_dec(tree_decomposition, control_flow_graph);

  good_re_root(tree_decomposition);
  nicify(tree_decomposition);
  alive_tree_dec(tree_decomposition, control_flow_graph);

  if(z80_opts.dump_graphs)
    dump_tree_decomposition(tree_decomposition);

  tree_dec_ralloc(tree_decomposition, control_flow_graph, conflict_graph);
}

