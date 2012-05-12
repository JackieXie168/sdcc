// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2011
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
//
// Lifetime-optimal speculative partial redundancy elimination.

#include <boost/graph/graphviz.hpp>

#include "SDCCtree_dec.hpp"

extern "C"
{
#include "SDCCsymt.h"
#include "SDCCicode.h"
#include "SDCCBBlock.h"
#include "SDCCopt.h"
#include "SDCCy.h"
}

#ifdef HAVE_STX_BTREE_SET_H
#include <stx/btree_set.h>
#endif

#ifdef HAVE_STX_BTREE_SET_H
typedef stx::btree_set<bool> lospreset_t; // Faster than std::set
#else
typedef std::set<bool> lospreset_t;
#endif

struct assignment_lospre
{
  float s;
  lospreset_t local;
  std::vector<bool> global;

  bool operator<(const assignment_lospre& a) const
  {
    lospreset_t::const_iterator i, ai, i_end, ai_end;

    i_end = local.end();
    ai_end = a.local.end();

    for (i = local.begin(), ai = a.local.begin();; ++i, ++ai)
      {
        if (i == i_end)
          return(true);
        if (ai == ai_end)
          return(false);

        if (*i < *ai)
          return(true);
        if (*i > *ai)
          return(false);

        if (global[*i] < a.global[*ai])
          return(true);
        if (global[*i] > a.global[*ai])
          return(false);
      }
  }
};

bool assignments_lospre_locally_same(const assignment_lospre &a1, const assignment_lospre &a2)
{
  if (a1.local != a2.local)
    return(false);

  lospreset_t::const_iterator i, i_end;
  for (i = a1.local.begin(), i_end = a1.local.end(); i != i_end; ++i)
    if (a1.global[*i] != a2.global[*i])
      return(false);

  return(true);
}

struct cfg_lospre_node
{
  iCode *ic;

  bool uses;
  bool invalidates;
};

typedef std::list<assignment_lospre> assignment_list_lospre_t;

struct tree_dec_lospre_node
{
  std::set<unsigned int> bag;
  assignment_list_lospre_t assignments;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, cfg_lospre_node, float> cfg_lospre_t; // The edge property is the cost of subdividing the edge and inserting an instruction (for now we always use 1, optimizing for code size, but relative execution frequency could be used when optimizing for speed or total energy consumption; aggregates thereof can be a good idea as well).
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, tree_dec_lospre_node> tree_dec_lospre_t;

// Handle Leaf nodes in the nice tree decomposition
template <class T_t, class G_t>
void tree_dec_lospre_leaf(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  assignment_lospre a;
  assignment_list_lospre_t &alist = T[t].assignments;

  a.s = 0;
  a.global.resize(boost::num_vertices(G));
  alist.push_back(a);
}

// Handle introduce nodes in the nice tree decomposition
template <class T_t, class G_t>
int tree_dec_lospre_introduce(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end;
  assignment_list_lospre_t::iterator ai;
  boost::tie(c, c_end) = adjacent_vertices(t, T);

  assignment_list_lospre_t &alist2 = T[t].assignments;
  assignment_list_lospre_t &alist = T[*c].assignments;

  std::set<unsigned short> new_inst;
  std::set_difference(T[t].bag.begin(), T[t].bag.end(), T[*c].bag.begin(), T[*c].bag.end(), std::inserter(new_inst, new_inst.end()));
  unsigned short int i = *(new_inst.begin());

  for(ai = alist.begin(); ai != alist.end(); ++ai)
    {
      ai->local.insert(i);

      ai->global[i] = true;
      alist2.push_back(*ai);

      if (!G[i].uses)
        {
          ai->global[i] = false;
          alist2.push_back(*ai);
        }
    }

  alist.clear();

  return((int)alist2.size() <= options.max_allocs_per_node ? 0 : -1);
}

// Handle forget nodes in the nice tree decomposition
template <class T_t, class G_t>
void tree_dec_lospre_forget(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
}

// Handle join nodes in the nice tree decomposition
template <class T_t, class G_t>
void tree_dec_lospre_join(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end, c2, c3;
  boost::tie(c, c_end) = adjacent_vertices(t, T);

  c2 = c;
  ++c;
  c3 = c;

  assignment_list_lospre_t &alist1 = T[t].assignments;
  assignment_list_lospre_t &alist2 = T[*c2].assignments;
  assignment_list_lospre_t &alist3 = T[*c3].assignments;

  alist2.sort();
  alist3.sort();

  assignment_list_lospre_t::iterator ai2, ai3;
  for (ai2 = alist2.begin(), ai3 = alist3.begin(); ai2 != alist2.end() && ai3 != alist3.end();)
    {
      if (assignments_lospre_locally_same(*ai2, *ai3))
        {
          ai2->s += ai3->s;
          for (size_t i = 0; i < ai2->global.size(); i++)
            ai2->global[i] = (ai2->global[i] || ai3->global[i]);
          alist1.push_back(*ai2);
          ++ai2;
          ++ai3;
        }
      else if (*ai2 < *ai3)
        {
          ++ai2;
          continue;
        }
      else if (*ai3 < *ai2)
        {
          ++ai3;
          continue;
        }
    }

  alist2.clear();
  alist3.clear();
}

template <class T_t, class G_t>
int tree_dec_lospre_nodes(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;

  adjacency_iter_t c, c_end;
  typename boost::graph_traits<T_t>::vertex_descriptor c0, c1;

  boost::tie(c, c_end) = adjacent_vertices(t, T);

  switch (out_degree(t, T))
    {
    case 0:
      tree_dec_lospre_leaf(T, t, G);
      break;
    case 1:
      c0 = *c;
      tree_dec_lospre_nodes(T, c0, G);
      if (T[c0].bag.size() < T[t].bag.size())
        {
        if (tree_dec_lospre_introduce(T, t, G))
          return(-1);
        }
      else
        tree_dec_lospre_forget(T, t, G);
      break;
    case 2:
      c0 = *c++;
      c1 = *c;
      tree_dec_lospre_nodes(T, c0, G);
      tree_dec_lospre_nodes(T, c1, G);
      tree_dec_lospre_join(T, t, G);
      break;
    default:
      std::cerr << "Not nice.\n";
      break;
    }
  return(0);
}

template <class T_t, class G_t>
int tree_dec_lospre (T_t &T, const G_t &G)
{
  if(tree_dec_lospre_nodes(T, find_root(T), G))
    return(-1);

  return(0);
}

