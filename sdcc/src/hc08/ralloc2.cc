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

iCode *hc08_ralloc2_cc(ebbIndex *ebbi)
{
  iCode *ic;

  cfg_t control_flow_graph;

  con_t conflict_graph;

  ic = create_cfg(control_flow_graph, conflict_graph, ebbi);

#ifdef TD_SALLOC

#endif

//eBBlock **ebbs = ebbi->bbOrder;
//ic = iCodeLabelOptimize (iCodeFromeBBlock (ebbs, ebbi->count));

  if(!SALLOC_TD && !SALLOC_CH)
    redoStackOffsets ();

  return(ic);
}

