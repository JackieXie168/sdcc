/*
 * Simulator of microcontrollers (hw.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include "ddconfig.h"

#include <stdlib.h>
#include "i_string.h"

#include "stypes.h"
#include "hwcl.h"


/*
 *____________________________________________________________________________
 */

cl_watched_cell::cl_watched_cell(class cl_mem *amem, t_addr aaddr,
				 class cl_cell **astore,
				 enum what_to_do_on_cell_change awtd)
{
  mem= amem;
  addr= aaddr;
  store= astore;
  wtd= awtd;
  if (mem)
    {
      cell= mem->get_cell(addr);
      if (store)
	*store= cell;
    }
}

void
cl_watched_cell::mem_cell_changed(class cl_mem *amem, t_addr aaddr,
				  class cl_hw *hw)
{
  if (mem &&
      mem == amem &&
      addr == aaddr)
    {
      cell= mem->get_cell(addr);
      if (store &&
	  (wtd & WTD_RESTORE))
	*store= cell;
      if (wtd & WTD_WRITE)
	{
	  t_mem d= cell->get();
	  hw->write(cell, &d);
	}
    }
}

/*void
cl_used_cell::mem_cell_changed(class cl_mem *amem, t_addr aaddr, 
class cl_hw *hw)
{
  if (mem &&
      mem == amem &&
      addr == aaddr)
    {
      cell= mem->get_cell(addr);
      if (store &&
	  (wtd & WTD_RESTORE))
	*store= cell;
      if (wtd & WTD_WRITE)
	{
	  t_mem d= cell->get();
	  hw->write(cell, &d);
	}
    }
}*/


/*
 *____________________________________________________________________________
 */

cl_hw::cl_hw(class cl_uc *auc, enum hw_cath cath, int aid, char *aid_string):
  cl_guiobj()
{
  flags= HWF_INSIDE;
  uc= auc;
  cathegory= cath;
  id= aid;
  if (aid_string &&
      *aid_string)
    id_string= strdup(aid_string);
  else
    id_string= strdup("unknown hw element");
  hws_to_inform= new cl_list(2, 2);
  watched_cells= new cl_list(2, 2);
}

cl_hw::~cl_hw(void)
{
  free(id_string);
  hws_to_inform->disconn_all();
  delete hws_to_inform;
  delete watched_cells;
}


/*
 * Callback functions for changing memory locations
 */

/*t_mem
cl_hw::read(class cl_mem *mem, t_addr addr)
{
  // Simply return the value
  return(mem->get(addr));
}*/

/*void
cl_hw::write(class cl_mem *mem, t_addr addr, t_mem *val)
{
  // Do not change *val by default
}*/


class cl_cell *
cl_hw::register_cell(class cl_mem *mem, t_addr addr, class cl_cell **store,
		     enum what_to_do_on_cell_change awtd)
{
  class cl_watched_cell *wc;
  class cl_cell *cell;

  if (mem)
    mem->register_hw(addr, this, (int*)0, DD_FALSE);
  wc= new cl_watched_cell(mem, addr, &cell, awtd);
  if (store)
    *store= cell;
  watched_cells->add(wc);
  // announce
  uc->sim->mem_cell_changed(mem, addr);
  return(cell);
}

class cl_cell *
cl_hw::use_cell(class cl_mem *mem, t_addr addr, class cl_cell **store,
		enum what_to_do_on_cell_change awtd)
{
  class cl_watched_cell *wc;
  class cl_cell *cell;

  wc= new cl_used_cell(mem, addr, &cell, awtd);
  if (store)
    *store= cell;
  watched_cells->add(wc);
  return(cell);
}

void
cl_hw::mem_cell_changed(class cl_mem *mem, t_addr addr)
{
  int i;

  for (i= 0; i < watched_cells->count; i++)
    {
      class cl_watched_cell *wc=
	(class cl_watched_cell *)(watched_cells->at(i));
      wc->mem_cell_changed(mem, addr, this);
    }
}


/*
 * Simulating `cycles' number of machine cycle
 */

int
cl_hw::tick(int cycles)
{
  return(0);
}

void
cl_hw::inform_partners(enum hw_event he, void *params)
{
  int i;

  for (i= 0; i < hws_to_inform->count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(hws_to_inform->at(i));
      hw->happen(this, he, params);
    }
}


void
cl_hw::print_info(class cl_console *con)
{
  con->dd_printf("%s[%d]\n", id_string, id);
}


t_index
cl_hws::add(void *item)
{
  int i;
  t_index res;

  // pre-add
  for (i= 0; i < count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(at(i));
      hw->adding((class cl_hw *)item);
    }
  // add
  res= cl_list::add(item);
  // post-add
  for (i= 0; i < count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(at(i));
      hw->added((class cl_hw *)item);
    }
  return(res);
}


void
cl_hws::mem_cell_changed(class cl_mem *mem, t_addr addr)
{
  int i;
  
  for (i= 0; i < count; i++)
    {
      class cl_hw *hw= (class cl_hw *)(at(i));
      hw->mem_cell_changed(mem, addr);
    }
}


/* End of hw.cc */
