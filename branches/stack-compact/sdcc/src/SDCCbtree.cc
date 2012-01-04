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

#include <set>

#include <boost/graph/adjacency_list.hpp>

extern "C"
{
#include "SDCCbtree.h"
#include "common.h"
}

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, std::pair<std::set<symbol *>, int> > btree_t;

static btree_t btree;

void btree_init()
{
  btree.clear();
  boost::add_vertex(btree);
}

void btree_add_child(short parent, short child)
{
  boost::add_edge(parent, child, btree);
}

short btree_lowest_common_ancestor(short a, short b)
{
  if(a == b)
    return(a);
  else if (a > b)
    a = boost::source(*boost::in_edges(a, btree).first, btree);
  else // a < b)
    b = boost::source(*boost::in_edges(b, btree).first, btree);
		
  return(btree_lowest_common_ancestor(a, b));
}

void btree_add_symbol(struct symbol *s)
{
  wassert(s);
  wassert(s->block >= 0);
  wassert(s->block < boost::num_vertices(btree));
  btree[s->block].first.insert(s);
}

static void btree_alloc_subtree(btree_t::vertex_descriptor v, int sPtr, int cssize, int *ssize)
{
  std::set<symbol *>::iterator s, s_end;
  for(s = btree[v].first.begin(), s_end = btree[v].first.end(); s != s_end; ++s)
    {
      struct symbol *const sym = *s;
      const int size = getSize (sym->type);
      
      //std::cout << "Allocating symbol " << sym->name << " (" << v << ") to " << sPtr << "\n";
      
      if(port->stack.direction > 0)
        {
          SPEC_STAK (sym->etype) = sym->stack = (sPtr + 1);
          sPtr += size;
        }
      else
        {
          sPtr -= size;
          SPEC_STAK (sym->etype) = sym->stack = sPtr;
        }
        
      cssize += size;
    }
  btree[v].second = cssize;
  if(cssize > *ssize)
    *ssize = cssize;
    
  boost::graph_traits<btree_t>::out_edge_iterator e, e_end;
  for(boost::tie(e, e_end) = boost::out_edges(v, btree); e != e_end; ++e)
    btree_alloc_subtree(boost::target(*e, btree), sPtr, cssize, ssize);
}

void btree_alloc(void)
{
  int ssize = 0;
  btree_alloc_subtree(0, 0, 0, &ssize);
  
  if(currFunc)
    {
      currFunc->stack += ssize;
      SPEC_STAK (currFunc->etype) += ssize;
    }
}

int btree_get_stack_size(short block)
{
  return(btree[block].second);
}

