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



