/*
 * Simulator of microcontrollers (pca.cc)
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

#include <ctype.h>

#include "pcacl.h"
#include "regs51.h"


cl_pca::cl_pca(class cl_uc *auc, int aid):
  cl_hw(auc, HW_PCA, aid, "pca")
{
}

int
cl_pca::init(void)
{
  switch (id)
    {
    case 0: addr_ccapXl= CCAP0L; addr_ccapXh= CCAP0H;addr_ccapmX= CCAPM0;break;
    case 1: addr_ccapXl= CCAP1L; addr_ccapXh= CCAP1H;addr_ccapmX= CCAPM1;break;
    case 2: addr_ccapXl= CCAP2L; addr_ccapXh= CCAP2H;addr_ccapmX= CCAPM2;break;
    case 3: addr_ccapXl= CCAP3L; addr_ccapXh= CCAP3H;addr_ccapmX= CCAPM3;break;
    case 4: addr_ccapXl= CCAP4L; addr_ccapXh= CCAP4H;addr_ccapmX= CCAPM4;break;
    default: return(1);
    }
  class cl_mem *sfr= uc->mem(MEM_SFR);
  if (!sfr)
    {
      fprintf(stderr, "No SFR to register PCA[%d] into\n", id);
    }
  ccapXl= sfr->register_hw(addr_ccapXl, this, 0);
  ccapXh= sfr->register_hw(addr_ccapXh, this, 0);
  ccapmX= sfr->get_cell(addr_ccapmX);
  return(0);
}

void
cl_pca::write(class cl_cell *cell, t_mem *val)
{
  if (cell == ccapXl)
    {
      ccapmX->set_bit0(bmECOM);
    }
  else if (cell == ccapXh)
    {
      ccapmX->set_bit1(bmECOM);
    }
}

void
cl_pca::mem_cell_changed(class cl_mem *mem, t_addr addr)
{
  class cl_mem *sfr= uc->mem(MEM_SFR);

  if (mem && sfr && mem == sfr)
    {
      if (addr == addr_ccapXl)
	ccapXl= sfr->get_cell(addr_ccapXl);
      else if (addr == addr_ccapXh)
	ccapXh= sfr->get_cell(addr_ccapXh);
      else if (addr == addr_ccapmX)
	ccapmX= sfr->get_cell(addr_ccapmX);
    }
}

void
cl_pca::print_info(class cl_console *con)
{
  con->dd_printf("%s[%d] FIXME\n", id_string, id);
}


/* End of s51.src/pca.cc */
