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
  unsigned char drySTM8iCode (iCode *ic);
  bool stm8_assignment_optimal;
};

// Code for another ic is generated when generating this one. Mark the other as generated.
static void extra_ic_generated(iCode *ic)
{
}

template <class T_t, class G_t, class I_t>
static bool tree_dec_ralloc(T_t &T, G_t &G, const I_t &I)
{
  bool assignment_optimal;

  assignment_optimal = true;

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

