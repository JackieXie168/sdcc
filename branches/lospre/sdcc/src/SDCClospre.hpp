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

struct cfg_lospre_node
{
  iCode *ic;
};

struct tree_dec_lospre_node
{
  std::set<unsigned int> bag;
};

