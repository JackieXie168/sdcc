/*
 * Simulator of microcontrollers (port.cc)
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

#include "portcl.h"
#include "regs51.h"
#include "types51.h"


cl_port::cl_port(class cl_uc *auc, int aid):
  cl_hw(auc, HW_PORT, aid, "port")
{
  port_pins= 0xff;
}

int
cl_port::init(void)
{
  switch (id)
    {
    case 0: sfr_addr= P0; break;
    case 1:
      {
	sfr_addr= P1;
	class cl_hw *hw;
	if ((hw= uc->get_hw(HW_TIMER, 2, 0)))
	  hws_to_inform->add(hw);
	break;
      }
    case 2: sfr_addr= P2; break;
    case 3:
      {
	sfr_addr= P3;
	class cl_hw *hw;
	if ((hw= uc->get_hw(HW_TIMER, 0, 0)))
	  hws_to_inform->add(hw);
	if ((hw= uc->get_hw(HW_TIMER, 1, 0)))
	  hws_to_inform->add(hw);
	if ((hw= uc->get_hw(HW_DUMMY, 0, 0)))
	  hws_to_inform->add(hw);
	break;
      }
    default: sfr_addr= P0; return(1);
    }
  class cl_mem *m= uc->mem(MEM_SFR);
  if (!m)
    {
      fprintf(stderr, "No SFR to register port into\n");
    }
  sfr= m->register_hw(sfr_addr, this, (int*)0);
  prev= sfr->get();
  return(0);
}

t_mem
cl_port::read(class cl_cell *cell)
{
  //printf("port[%d] read\n",id);
  return(cell->get() & port_pins);
}

void
cl_port::write(class cl_cell *cell, t_mem *val)
{
  struct ev_port_changed ep;

  (*val)&= 0xff; // 8 bit port
  ep.id= id;
  ep.addr= sfr_addr;
  ep.prev_value= sfr->get();
  ep.new_value= *val;
  ep.pins= ep.new_pins= port_pins;
  if (ep.prev_value != ep.new_value)
    inform_partners(EV_PORT_CHANGED, &ep);
  prev= sfr->get();
  //printf("port[%d] write 0x%x\n",id,val);
}

t_mem
cl_port::set_cmd(t_mem value)
{
  struct ev_port_changed ep;

  ep.id= id;
  ep.addr= sfr_addr;
  ep.pins= port_pins;
  port_pins= value;
  ep.prev_value= sfr->get();
  ep.new_value= sfr->get();
  ep.new_pins= port_pins;
  if (ep.pins != ep.new_pins)
    inform_partners(EV_PORT_CHANGED, &ep);
  return(value);
}

void
cl_port::mem_cell_changed(class cl_mem *mem, t_addr addr)
{
  class cl_mem *s= uc->mem(MEM_SFR);
  
  if (mem && s && mem == s)
    {
      if (addr == sfr_addr)
	{
	  sfr= s->get_cell(sfr_addr);
	  t_mem d= sfr->get();
	  write(sfr, &d);
	}
    }  
}

void
cl_port::print_info(class cl_console *con)
{
  uchar data;

  con->dd_printf("%s[%d]\n", id_string, id);
  data= sfr->get();//uc->get_mem(MEM_SFR, sfr);
  con->dd_printf("P%d    ", id);
  con->print_bin(data, 8);
  con->dd_printf(" 0x%02x %3d %c (Value in SFR register)\n",
		 data, data, isprint(data)?data:'.');

  data= /*uc->*/port_pins/*[id]*/;
  con->dd_printf("Pin%d  ", id);
  con->print_bin(data, 8);
  con->dd_printf(" 0x%02x %3d %c (Output of outside circuits)\n",
		 data, data, isprint(data)?data:'.');

  //data= /*uc->*/port_pins/*[id]*/ & sfr->get();//uc->get_mem(MEM_SFR, sfr);
  data= sfr->read();
  con->dd_printf("Port%d ", id);
  con->print_bin(data, 8);
  con->dd_printf(" 0x%02x %3d %c (Value on the port pins)\n",
		 data, data, isprint(data)?data:'.');
}


/* End of s51.src/port.cc */
