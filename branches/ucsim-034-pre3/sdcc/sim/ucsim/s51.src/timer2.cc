/*
 * Simulator of microcontrollers (timer2.cc)
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

#include "timer2cl.h"
#include "regs51.h"
#include "types51.h"


cl_timer2::cl_timer2(class cl_uc *auc, int aid, char *aid_string,
		     class cl_it_src *exf2_it):
  cl_timer0(auc, /*2*/aid, /*"timer2"*/aid_string)
{
  exf2it= exf2_it;
  mask_RCLK= bmRCLK;
  mask_TCLK= bmTCLK;
  mask_CP_RL2= bmCP_RL2;
}

int
cl_timer2::init(void)
{
  cl_timer0::init();
  cell_rcap2l= uc->mem(MEM_SFR)->get_cell(RCAP2L);
  cell_rcap2h= uc->mem(MEM_SFR)->get_cell(RCAP2H);
  class cl_hw *s= uc->get_hw(HW_UART, 0);
  if (s)
    hws_to_inform->add(s);
  return(0);
}

void
cl_timer2::mem_cell_changed(class cl_mem *mem, t_addr addr)
{
  class cl_mem *sfr= uc->mem(MEM_SFR);
  class cl_cell *c= 0;

  if (mem && sfr && mem == sfr)
    {
      switch (addr)
	{
	case T2CON:
	  c= cell_tcon= sfr->get_cell(T2CON);
	  break;
	}
      if (c)
	{
	  t_mem d= c->get();
	  write(c, &d);
	}
      if (addr == addr_tl)
	cell_tl= sfr->get_cell(addr_tl);
      if (addr == addr_th)
	cell_th= sfr->get_cell(addr_th);
      cell_rcap2l= sfr->get_cell(RCAP2L);
      cell_rcap2h= sfr->get_cell(RCAP2H);
    }
}

void
cl_timer2::added(class cl_hw *new_hw)
{
  if (new_hw->cathegory == HW_UART)
    hws_to_inform->add(new_hw);
}

void
cl_timer2::write(class cl_cell *cell, t_mem *val)
{
  if (cell == cell_tcon)
    {
      C_T = *val & mask_C_T;
      TR  = *val & mask_TR;
      RCLK= *val & mask_RCLK;
      TCLK= *val & mask_TCLK;
      CP_RL2= *val & mask_CP_RL2;
      EXEN2 = *val & bmEXEN2;
      int oldmode= mode;
      if (!(RCLK || TCLK) &&
	  !CP_RL2)
	mode= T2MODE_RELOAD;
      else if (!(RCLK || TCLK) &&
	       CP_RL2)
	mode= T2MODE_CAPTURE;
      else if (RCLK || TCLK)
	mode= T2MODE_BAUDRATE;
      else
	mode= T2MODE_OFF;
      if (mode != oldmode)
	inform_partners(EV_T2_MODE_CHANGED, val);
      T_edge= t2ex_edge= 0;
    }
}

int
cl_timer2::tick(int cycles)
{
  bool nocount= DD_FALSE;

  exf2it->activate();
  
  if (!TR)
    /* Timer OFF */
    return(resGO);

  if (mode == T2MODE_BAUDRATE)
    return(do_t2_baud(cycles));

  /* Determining nr of input clocks */
  if (C_T)
    {
      // Counter mode, falling edge on P1.0 (T2)
      if (/*(uc51->prev_p1 & bmT2) &&
	    !(uc51->sfr->read(P1) & bmT2)*/
	  T_edge)
	{
	  cycles= 1;
	  T_edge= 0;
	}
      else
	nocount= DD_TRUE;
    }
  /* Counting */
  while (cycles--)
    {
      if (CP_RL2)
	do_t2_capture(&cycles, nocount);
      else
	do_t2_reload(&cycles, nocount);
    }
  
  return(resGO);
}

/*
 * Baud rate generator mode of Timer #2
 */

int
cl_timer2::do_t2_baud(int cycles)
{
  //t_mem t2con= uc51->sfr->get(T2CON);
  //uint p1= get_mem(MEM_SFR, P1);

  /* Baud Rate Generator */
  if (/*(uc51->prev_p1 & bmT2EX) &&
      !(uc51->sfr->read(P1) & bmT2EX) &&
      (t2con & bmEXEN2)*/
      EXEN2 && t2ex_edge)
    cell_tcon->set_bit1(bmEXF2);
  if (C_T)
    {
      if (/*(uc51->prev_p1 & bmT2) &&
	    !(uc51->sfr->read(P1) & bmT2)*/
	  T_edge)
	cycles= 1;
      else
	cycles= 0;
    }
  else
    cycles*= 6;
  if (TR)
    while (cycles--)
      {
	if (!cell_tl->add(1))
	  if (!cell_th->add(1))
	    {
	      cell_th->set(cell_rcap2h->get());
	      cell_tl->set(cell_rcap2l->get());
	      inform_partners(EV_OVERFLOW, 0);
	    }
      }
  return(resGO);
}


/*
 * Capture function of Timer #2
 */

void
cl_timer2::do_t2_capture(int *cycles, bool nocount)
{
  //uint p1= get_mem(MEM_SFR, P1);
  //t_mem t2con= uc51->sfr->get(T2CON);

  /* Capture mode */
  if (nocount)
    *cycles= 0;
  else
    {
      if (!cell_tl->add(1))
	{
	  if (!cell_th->add(1))
	    cell_tcon->set_bit1(bmTF2);
	}
    }
  // capture
  if (/*(uc51->prev_p1 & bmT2EX) &&
      !(uc51->sfr->read(P1) & bmT2EX) &&
      (t2con & bmEXEN2)*/
      EXEN2 && t2ex_edge)
    {
      cell_rcap2h->set(cell_th->get());
      cell_rcap2l->set(cell_tl->get());
      cell_tcon->set_bit1(bmEXF2);
      //uc51->prev_p1&= ~bmT2EX; // Falling edge has been handled
      t2ex_edge= 0;
    }
}


/*
 * Auto Reload mode of Timer #2, counting UP
 */

void
cl_timer2::do_t2_reload(int *cycles, bool nocount)
{
  int overflow;
  bool ext2= 0;
  
  /* Auto-Relode mode */
  overflow= 0;
  if (nocount)
    *cycles= 0;
  else
    {
      if (!cell_tl->add(1))
	{
	  if (!cell_th->add(1))
	    {
	      cell_tcon->set_bit1(mask_TF);
	      overflow++;
	    }
	}
    }
  // reload
  if (/*(uc51->prev_p1 & bmT2EX) &&
      !(uc51->sfr->read(P1) & bmT2EX) &&
      (uc51->sfr->get(T2CON) & bmEXEN2)*/
      EXEN2 && t2ex_edge)
    {
      ext2= DD_TRUE;
      cell_tcon->set_bit1(bmEXF2);
      //uc51->prev_p1&= ~bmT2EX; // Falling edge has been handled
      t2ex_edge= 0;
    }
  if (overflow ||
      ext2)
    {
      cell_th->set(cell_rcap2h->get());
      cell_tl->set(cell_rcap2l->get());
    }
}

void
cl_timer2::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;

  if (where->cathegory == HW_PORT &&
      he == EV_PORT_CHANGED &&
      ep->id == 1)
    {
      t_mem p1n= ep->new_pins & ep->new_value;
      t_mem p1o= ep->pins & ep->prev_value;
      if ((p1n & mask_T) &&
	  !(p1o & mask_T))
	T_edge++;
      if (!(p1n & bmT2EX) &&
	  (p1o & bmT2EX))
	t2ex_edge++;
    }
}

void
cl_timer2::print_info(class cl_console *con)
{
  int t2con= cell_tcon->get();

  con->dd_printf("%s[%d] 0x%04x", id_string, id,
		 256*cell_th->get()+cell_tl->get());
  if (RCLK || TCLK)
    {
      con->dd_printf(" baud");
      if (RCLK)
	con->dd_printf(" RCLK");
      if (TCLK)
	con->dd_printf(" TCLK");
    }
  else
    con->dd_printf(" %s", (CP_RL2)?"capture":"reload");
  con->dd_printf(" 0x%04x",
		 256*cell_rcap2h->get()+cell_rcap2l->get());
  con->dd_printf(" %s", (C_T)?"counter":"timer");
  con->dd_printf(" %s", (TR)?"ON":"OFF");
  con->dd_printf(" irq=%c", (t2con&bmTF2)?'1':'0');
  con->dd_printf(" %s", (uc->get_mem(MEM_SFR, IE)&bmET2)?"en":"dis");
  con->dd_printf(" prio=%d", uc->it_priority(bmPT2));
  con->dd_printf("\n");
}


/* End of s51.src/timer2.cc */
