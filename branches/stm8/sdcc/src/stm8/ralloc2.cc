// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2010 - 2011
//
// (c) 2012 Goethe-Universität Frankfurt
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

//#define DEBUG_RALLOC_DEC // Uncomment to get debug messages while doing register allocation on the tree decomposition.
//#define DEBUG_RALLOC_DEC_ASS // Uncomment to get debug messages about assignments while doing register allocation on the tree decomposition (much more verbose than the one above).

#include "SDCCralloc.hpp"

extern "C"
{
  #include "ralloc.h"
  #include "gen.h"
  unsigned int drySTM8iCode (iCode *ic);
  bool stm8_assignment_optimal;
};

template <class G_t, class I_t>
static void set_surviving_regs(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  iCode *ic = G[i].ic;
  
  ic->rSurv = newBitVect(port->num_regs);
  
  std::set<var_t>::const_iterator v, v_end;
  for (v = G[i].alive.begin(), v_end = G[i].alive.end(); v != v_end; ++v)
    if(a.global[*v] >= 0 && G[i].dying.find(*v) == G[i].dying.end())
      if(!((IC_RESULT(ic) && !POINTER_SET(ic)) && IS_SYMOP(IC_RESULT(ic)) && OP_SYMBOL_CONST(IC_RESULT(ic))->key == I[*v].v))
        ic->rSurv = bitVectSetBit(ic->rSurv, a.global[*v]);
}

template<class G_t>
static void unset_surviving_regs(unsigned short int i, const G_t &G)
{
  iCode *ic = G[i].ic;
  
  freeBitVect(ic->rSurv);
}

template <class G_t, class I_t>
static void assign_operand_for_cost(operand *o, const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  if(!o || !IS_SYMOP(o))
    return;
  symbol *sym = OP_SYMBOL(o);
  operand_map_t::const_iterator oi, oi_end;
  for(boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key); oi != oi_end; ++oi)
    {
      var_t v = oi->second;
      if(a.global[v] >= 0)
        { 
          sym->regs[I[v].byte] = stm8_regs + a.global[v];   
          sym->nRegs = I[v].size;
        }
      else
        {
          sym->regs[I[v].byte] = 0;
          sym->nRegs = I[v].size;
        }
    }
}

template <class G_t, class I_t>
static void assign_operands_for_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;
  
  if(ic->op == IFX)
    assign_operand_for_cost(IC_COND(ic), a, i, G, I);
  else if(ic->op == JUMPTABLE)
    assign_operand_for_cost(IC_JTCOND(ic), a, i, G, I);
  else
    {
      assign_operand_for_cost(IC_LEFT(ic), a, i, G, I);
      assign_operand_for_cost(IC_RIGHT(ic), a, i, G, I);
      assign_operand_for_cost(IC_RESULT(ic), a, i, G, I);
    }
    
  if(ic->op == SEND && ic->builtinSEND)
    assign_operands_for_cost(a, *(adjacent_vertices(i, G).first), G, I);
}

// Cost function.
template <class G_t, class I_t>
static float instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  iCode *ic = G[i].ic;
  float c;

  wassert (TARGET_IS_STM8);

  /*if(!inst_sane(a, i, G, I))
    return(std::numeric_limits<float>::infinity());*/

#if 0
  std::cout << "Calculating at cost at ic " << ic->key << " for: ";
  for(unsigned int i = 0; i < boost::num_vertices(I); i++)
  {
  	std::cout << "(" << i << ", " << int(a.global[i]) << ") ";
  }
  std::cout << "\n";
  std::cout.flush();
#endif

  if(ic->generated)
    return(0.0f);

  switch(ic->op)
    {
    // Register assignment doesn't matter for these:
    case FUNCTION:
    case ENDFUNCTION:
    case LABEL:
    case GOTO:
    case INLINEASM:
      return(0.0f);
    /*case '!':
    case '~':
    case UNARYMINUS:
    case '+':
    case '-':
    case '^':
    case '|':
    case BITWISEAND:
    case IPUSH:
    //case IPOP:
    case CALL:
    case PCALL:
    case RETURN:
    case '*':
    case '/':
    case '%':
    case '>':
    case '<':
    case LE_OP:
    case GE_OP:
    case EQ_OP:
    case NE_OP:
    case AND_OP:
    case OR_OP:
    case GETHBIT:
    case GETABIT:
    case GETBYTE:
    case GETWORD:
    case LEFT_OP:
    case RIGHT_OP:
    case GET_VALUE_AT_ADDRESS:
    case '=':
    case IFX:
    case ADDRESS_OF:
    case JUMPTABLE:
    case CAST:
    case RECEIVE:
    case SEND:
    case DUMMY_READ_VOLATILE:
    case CRITICAL:
    case ENDCRITICAL:
    case SWAP:*/
      assign_operands_for_cost(a, i, G, I);
      set_surviving_regs(a, i, G, I);
      c = drySTM8iCode(ic);
      unset_surviving_regs(i, G);
      return(c);
    default:
      return(0.0f);
    }
}

// For early removal of assignments that cannot be extended to valid assignments. This is just a dummy for now.
template <class G_t, class I_t>
static bool assignment_hopeless(const assignment &a, unsigned short int i, const G_t &G, const I_t &I, const var_t lastvar)
{
  return(false);
}

// Increase chance of finding good compatible assignments at join nodes. This is just a dummy for now.
template <class T_t>
static void get_best_local_assignment_biased(assignment &a, typename boost::graph_traits<T_t>::vertex_descriptor t, const T_t &T)
{
  a = *T[t].assignments.begin();

  std::set<var_t>::const_iterator vi, vi_end;
  for(vi = T[t].alive.begin(), vi_end = T[t].alive.end(); vi != vi_end; ++vi)
    a.local.insert(*vi);
}

// This is just a dummy for now.
template <class G_t, class I_t>
static float rough_cost_estimate(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  return(0.0f);
}

// Code for another ic is generated when generating this one. Mark the other as generated.
static void extra_ic_generated(iCode *ic)
{
}

template <class T_t, class G_t, class I_t>
static bool tree_dec_ralloc(T_t &T, G_t &G, const I_t &I)
{
  bool assignment_optimal;

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

  assignment ac;
  assignment_optimal = true;
  tree_dec_ralloc_nodes(T, find_root(T), G, I2, ac, &assignment_optimal);

  const assignment &winner = *(T[find_root(T)].assignments.begin());

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Winner: ";
  for(unsigned int i = 0; i < boost::num_vertices(I); i++)
  {
  	std::cout << "(" << i << ", " << int(winner.global[i]) << ") ";
  }
  std::cout << "\n";
  std::cout << "Cost: " << winner.s << "\n";
  std::cout.flush();
#endif

  // Todo: Make this an assertion
  if(winner.global.size() != boost::num_vertices(I))
    {
      std::cerr << "ERROR: No Assignments at root\n";
      exit(-1);
    }

  for(unsigned int v = 0; v < boost::num_vertices(I); v++)
    {
      symbol *sym = (symbol *)(hTabItemWithKey(liveRanges, I[v].v));
      if(winner.global[v] >= 0)
        { 
          sym->regs[I[v].byte] = stm8_regs + winner.global[v];   
          sym->nRegs = I[v].size;
        }
      else
        {
          sym->regs[I[v].byte] = 0;
          sym->nRegs = I[v].size;
        }
    }

  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    set_surviving_regs(winner, i, G, I);	// Never freed. Memory leak?

  return(!assignment_optimal);
}

iCode *stm8_ralloc2_cc(ebbIndex *ebbi)
{
  iCode *ic;

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Processing " << currFunc->name << " from " << dstFileName << "\n"; std::cout.flush();
#endif

  cfg_t control_flow_graph;

  con_t conflict_graph;

  ic = create_cfg(control_flow_graph, conflict_graph, ebbi);

  if(options.dump_graphs)
    dump_cfg(control_flow_graph);

  if(options.dump_graphs)
    dump_con(conflict_graph);

  tree_dec_t tree_decomposition;

  thorup_tree_decomposition(tree_decomposition, control_flow_graph);

  nicify(tree_decomposition);

  alive_tree_dec(tree_decomposition, control_flow_graph);

  good_re_root(tree_decomposition);
  nicify(tree_decomposition);
  alive_tree_dec(tree_decomposition, control_flow_graph);

  if(options.dump_graphs)
    dump_tree_decomposition(tree_decomposition);

  stm8_assignment_optimal = !tree_dec_ralloc(tree_decomposition, control_flow_graph, conflict_graph);

  return(ic);
}

