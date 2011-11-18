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

#include "SDCCtree_dec.hpp"

extern "C"
{
#include "SDCCsymt.h"
#include "SDCCicode.h"
#include "SDCCBBlock.h"
#include "SDCCopt.h"
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

#endif

