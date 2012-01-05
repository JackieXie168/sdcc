// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2010 - 2011
//
// (c) 2011 Goethe-Universität Frankfurt
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

#define TD_SALLOC
#define CH_SALLOC

#include "SDCCralloc.hpp"
#include "SDCCsalloc.hpp"

extern "C"
{
  #include "ralloc.h"
};

template <class I_t> void
hc08_add_operand_conflicts_in_node(const cfg_node &n, I_t &I)
{
  const iCode *ic = n.ic;
  
  const operand *result = IC_RESULT(ic);
  const operand *left = IC_LEFT(ic);
  const operand *right = IC_RIGHT(ic);
	
  if(!result || !IS_SYMOP(result))
    return;
    
  // Todo: Identify operations, code generation can always handle and exclude them (as done for the z80-like ports).
   
  operand_map_t::const_iterator oir, oir_end, oirs; 
  boost::tie(oir, oir_end) = n.operands.equal_range(OP_SYMBOL_CONST(result)->key);
  if(oir == oir_end)
    return;
    
  operand_map_t::const_iterator oio, oio_end;
  
  if(left && IS_SYMOP(left))
    for(boost::tie(oio, oio_end) = n.operands.equal_range(OP_SYMBOL_CONST(left)->key); oio != oio_end; ++oio)
      for(oirs = oir; oirs != oir_end; ++oirs)
        {
          var_t rvar = oirs->second;
          var_t ovar = oio->second;
          if(I[rvar].byte < I[ovar].byte)
            boost::add_edge(rvar, ovar, I);
        }
        
  if(right && IS_SYMOP(right))
    for(boost::tie(oio, oio_end) = n.operands.equal_range(OP_SYMBOL_CONST(right)->key); oio != oio_end; ++oio)
      for(oirs = oir; oirs != oir_end; ++oirs)
        {
          var_t rvar = oirs->second;
          var_t ovar = oio->second;
          if(I[rvar].byte < I[ovar].byte)
            boost::add_edge(rvar, ovar, I);
        }
}


// Does not really do register allocation yet. For now this is just a dummy needed for stack allocation.
template <class T_t, class G_t, class I_t, class SI_t>
int tree_dec_ralloc(T_t &T, G_t &G, const I_t &I, SI_t &SI)
{
  bool assignment_optimal;

  assignment_optimal = true;
    
#if defined(TD_SALLOC) || defined(CH_SALLOC)
  set_spilt(G, I, SI);
#endif

  return(!assignment_optimal);
}

iCode *hc08_ralloc2_cc(ebbIndex *ebbi)
{
  iCode *ic;

  cfg_t control_flow_graph;

  con_t conflict_graph;

#ifdef TD_SALLOC
  scon_t stack_conflict_graph;
  std::list<unsigned int> ordering;
  std::map<unsigned int, std::set<unsigned int> > separators;
#endif

  ic = create_cfg(control_flow_graph, conflict_graph, ebbi);

  if(options.dump_graphs)
    dump_cfg(control_flow_graph);

  if(options.dump_graphs)
    dump_con(conflict_graph);

  tree_dec_t tree_decomposition;

#ifndef TD_SALLOC
  thorup_tree_decomposition(tree_decomposition, control_flow_graph);
#else
  thorup_tree_decomposition(tree_decomposition, control_flow_graph, &ordering, &separators);
#endif


  tree_dec_ralloc(tree_decomposition, control_flow_graph, conflict_graph, stack_conflict_graph);

#if defined(TD_SALLOC) || defined(CH_SALLOC)
  if(options.dump_graphs)
    dump_scon(stack_conflict_graph);

  if (SALLOC_TD)
    tree_dec_salloc(control_flow_graph, stack_conflict_graph, ordering, separators);
  else if(SALLOC_CH)
    chaitin_salloc(stack_conflict_graph);
#endif

  if(!SALLOC_TD && !SALLOC_CH)
    redoStackOffsets ();

  return(ic);
}

