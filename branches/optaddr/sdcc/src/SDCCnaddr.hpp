// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2011
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
//
//
// Optimal placement of bank switching instructions for named address spaces.

#ifndef SDCCNADDR_HH
#define SDCCNADDR_HH 1


#include <map>
#include <vector>
#include <sstream>
#include <fstream>

#include <boost/graph/graphviz.hpp>

#include "SDCCtree_dec.hpp"

extern "C"
{
#include "SDCCsymt.h"
#include "SDCCicode.h"
#include "SDCCBBlock.h"
#include "SDCCopt.h"
}

typedef short int naddrspace_t; // Named address spaces. -1: Undefined, Others: see map.

struct cfg_node
{
  iCode *ic;
  std::set<naddrspace_t> possible_naddrspaces;
};

struct tree_dec_naddr_node
{
  std::set<unsigned int> bag;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, cfg_node, float> cfg_t; // The edge property is the cost of subdividing he edge and inserting a bank switching instruction.
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, tree_dec_naddr_node> tree_dec_naddr_t;

// A quick-and-dirty function to get the CFG from sdcc (a simplified version of the function from SDCCralloc.hpp).
void
create_cfg_naddr(cfg_t &cfg, iCode *start_ic, ebbIndex *ebbi)
{
  iCode *ic;

  std::map<int, unsigned int> key_to_index;
  {
    int i;

    for (ic = start_ic, i = 0; ic; ic = ic->next, i++)
      {
        boost::add_vertex(cfg);
        key_to_index[ic->key] = i;
        cfg[i].ic = ic;
      }
  }

  // Get control flow graph from sdcc.
  for (ic = start_ic; ic; ic = ic->next)
    {
      if (ic->op != GOTO && ic->op != RETURN && ic->op != JUMPTABLE && ic->next)
        boost::add_edge(key_to_index[ic->key], key_to_index[ic->next->key], 3.0f, cfg);

      if (ic->op == GOTO)
        boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, ic->label)->sch->key], 6.0f, cfg);
      else if (ic->op == RETURN)
        boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, returnLabel)->sch->key], 6.0f, cfg);
      else if (ic->op == IFX)
        boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, IC_TRUE(ic) ? IC_TRUE(ic) : IC_FALSE(ic))->sch->key], 6.0f, cfg);
      else if (ic->op == JUMPTABLE)
        for (symbol *lbl = (symbol *)(setFirstItem (IC_JTLABELS (ic))); lbl; lbl = (symbol *)(setNextItem (IC_JTLABELS (ic))))
          boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, lbl)->sch->key], 6.0f, cfg);
    }
}

// Annotate nodes of the control flow graph with the set of possible named address spaces active there.
void annotate_cfg_naddr(cfg_t &cfg)
{
  typedef typename boost::graph_traits<cfg_t>::vertex_descriptor vertex_t;

  std::map<const symbol *, naddrspace_t> sym_to_index;
  naddrspace_t na_max = -1;

  std::vector<bool> predetermined(boost::num_vertices (cfg), false);

  // Initialize the cfg vertices where there is information on the desired named address space.
  for (vertex_t i = 0; i < boost::num_vertices (cfg); i++)
    {
      const iCode *ic = cfg[i].ic;
      const symbol *addrspace;

      // We do not know the current named address space when entering a function or after calling one.
      if (ic->op == CALL || ic->op == PCALL || ic->op == FUNCTION)
        predetermined[i] = true;
      
      // Set the required named address spaces
      if (addrspace = getAddrspaceiCode (ic))
        {
          naddrspace_t na;

          if (sym_to_index.find (addrspace) == sym_to_index.end ())
            sym_to_index[addrspace] = ++na_max;
          na = sym_to_index[addrspace];

          cfg[i].possible_naddrspaces.insert (na);
          predetermined[i] = true;
        }
      else
        cfg[i].possible_naddrspaces.insert(-1);
    }

  // Extend.
  for(bool change = true; change; change = false)
    for (vertex_t i = 0; i < boost::num_vertices (cfg); i++)
    {
      if (predetermined[i])
        continue;

      size_t oldsize = cfg[i].possible_naddrspaces.size();
      {
        typedef typename boost::graph_traits<cfg_t>::out_edge_iterator n_iter_t;
        n_iter_t n, n_end;    
        for (boost::tie(n, n_end) = boost::out_edges(i, cfg);  n != n_end; ++n)
          {
            vertex_t v = boost::target(*n, cfg);
            cfg[i].possible_naddrspaces.insert(cfg[v].possible_naddrspaces.begin(), cfg[v].possible_naddrspaces.end());
          }
      }
      {
        typedef typename boost::graph_traits<cfg_t>::in_edge_iterator n_iter_t;
        n_iter_t n, n_end;    
        for (boost::tie(n, n_end) = boost::in_edges(i, cfg);  n != n_end; ++n)
          {
            vertex_t v = boost::source(*n, cfg);
            cfg[i].possible_naddrspaces.insert(cfg[v].possible_naddrspaces.begin(), cfg[v].possible_naddrspaces.end());
          }
      }

      if (oldsize != cfg[i].possible_naddrspaces.size())
          change = true;
    }
}

template <class T_t, class G_t>
void tree_dec_address_switch_nodes(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
}

template <class T_t, class G_t>
void tree_dec_address_switch(T_t &T, const G_t &G)
{
  tree_dec_address_switch_nodes(T, find_root(T), G);
}

// Dump cfg, with numbered nodes, show posible address spaces at each node.
void dump_cfg_naddr(const cfg_t &cfg)
{
  std::ofstream dump_file((std::string(dstFileName) + ".dumpnaddrcfg" + currFunc->rname + ".dot").c_str());

  std::string *name = new std::string[num_vertices(cfg)];
  for (unsigned int i = 0; i < boost::num_vertices(cfg); i++)
    {
      std::ostringstream os;
      os << i << ", " << cfg[i].ic->key << ": ";
      std::set<naddrspace_t>::const_iterator n;
      for (n = cfg[i].possible_naddrspaces.begin(); n != cfg[i].possible_naddrspaces.end(); ++n)
        os << *n << " ";
      name[i] = os.str();
    }
  boost::write_graphviz(dump_file, cfg, boost::make_label_writer(name));
  delete[] name;
}

#endif

