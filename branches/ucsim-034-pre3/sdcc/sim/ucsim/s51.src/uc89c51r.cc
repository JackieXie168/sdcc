/*
 * Simulator of microcontrollers (uc89c51r.cc)
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

#include <stdio.h>

// local
#include "uc89c51rcl.h"
#include "regs51.h"
#include "pcacl.h"


t_uc89c51r::t_uc89c51r(int Itype, int Itech, class cl_sim *asim):
  t_uc51r(Itype, Itech, asim)
{
  it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF4, 0x0033, false,
				      "PCA module #4"));
  it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF3, 0x0033, false,
				      "PCA module #3"));
  it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF2, 0x0033, false,
				      "PCA module #2"));
  it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF1, 0x0033, false,
				      "PCA module #1"));
  it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF0, 0x0033, false,
				      "PCA module #0"));
  it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCF, 0x0033, false,
				      "PCA counter"));
}


void
t_uc89c51r::mk_hw_elements(void)
{
  class cl_hw *h;

  t_uc51r::mk_hw_elements();
  hws->add(h= new cl_pca(this, 0));
  h->init();
  hws->add(h= new cl_pca(this, 1));
  h->init();
  hws->add(h= new cl_pca(this, 2));
  h->init();
  hws->add(h= new cl_pca(this, 3));
  h->init();
  hws->add(h= new cl_pca(this, 4));
  h->init();
  hws->add(h= new cl_89c51r_dummy_hw(this));
  h->init();
}


void
t_uc89c51r::reset(void)
{
  t_uc51r::reset();
  sfr->set_bit1(CCAPM0, bmECOM);
  sfr->set_bit1(CCAPM1, bmECOM);
  sfr->set_bit1(CCAPM2, bmECOM);
  sfr->set_bit1(CCAPM3, bmECOM);
  sfr->set_bit1(CCAPM4, bmECOM);
  t0_overflows= 0;
  dpl0= dph0= dpl1= dph1= 0;
  sfr->set(IPH, 0);
}

int
t_uc89c51r::it_priority(uchar ie_mask)
{
  uchar l, h;

  l= sfr->get(IP) & ie_mask;
  h= sfr->get(IPH) & ie_mask;
  if (!h && !l)
    return(0);
  if (!h && l)
    return(1);
  if (h && !l)
    return(2);
  if (h && l)
    return(3);
  return(0);
}

void
t_uc89c51r::pre_inst(void)
{
  if (sfr->get(AUXR1) & bmDPS)
    {
      sfr->set(DPL, dpl1);
      sfr->set(DPH, dph1);
    }
  else
    {
      sfr->set(DPL, dpl0);
      sfr->set(DPH, dph0);
    }
}

void
t_uc89c51r::post_inst(void)
{
  if (sfr->get(AUXR1) & bmDPS)
    {
      dpl1= sfr->get(DPL);
      dph1= sfr->get(DPH);
    }
  else
    {
      dpl0= sfr->get(DPL);
      dph0= sfr->get(DPH);
    }
}


/*
 * Simulating timers
 *
 * Calling inherited method to simulate timer #0, #1, #2 and then 
 * simulating Programmable Counter Array
 */

void
t_uc89c51r::do_extra_hw(int cycles)
{
  t_uc51r::do_extra_hw(cycles);
  do_pca(cycles);
}

int
t_uc89c51r::t0_overflow(void)
{
  uchar cmod= sfr->get(CMOD) & (bmCPS0|bmCPS1);

  if (cmod == bmCPS1)
    t0_overflows++;
  return(0);
}


/*
 * Simulating Programmable Counter Array
 */

int
t_uc89c51r::do_pca(int cycles)
{
  int ret= resGO;
  uint ccon= sfr->get(CCON);

  if (!(ccon & bmCR))
    return(resGO);
  if (state == stIDLE &&
      (ccon & bmCIDL))
    return(resGO);

  switch (sfr->get(CMOD) & (bmCPS1|bmCPS0))
    {
    case 0:
      ret= do_pca_counter(cycles);
      break;
    case bmCPS0:
      ret= do_pca_counter(cycles*3);
      break;
    case bmCPS1:
      ret= do_pca_counter(t0_overflows);
      t0_overflows= 0;
      break;
    case (bmCPS0|bmCPS1):
      if ((prev_p1 & bmECI) != 0 &
	  (sfr->get/*FIXME:read?*/(P1) & bmECI) == 0)
	do_pca_counter(1);
      break;
    }
  return(ret);
}

int
t_uc89c51r::do_pca_counter(int cycles)
{
  while (cycles--)
    {
      if (sfr->add(CL, 1) == 0)
	{
	  if (sfr->add(CH, 1) == 0)
	    {
	      /* CH,CL overflow */
	      sfr->set_bit1(CCON, bmCF);
	      do_pca_module(0);
	      do_pca_module(1);
	      do_pca_module(2);
	      do_pca_module(3);
	      do_pca_module(4);
	    }
	}
    }
  return(resGO);
}

int
t_uc89c51r::do_pca_module(int nr)
{
  uchar CCAPM[5]= {0xda, 0xdb, 0xdc, 0xdd, 0xde};
  uchar CCAPL[5]= {0xea, 0xeb, 0xec, 0xed, 0xee};
  uchar CCAPH[5]= {0xfa, 0xfb, 0xfc, 0xfd, 0xfe};
  uchar bmCEX[5]= {bmCEX0, bmCEX1, bmCEX2, bmCEX3, bmCEX4};
  uchar bmCCF[5]= {bmCCF0, bmCCF1, bmCCF2, bmCCF3, bmCCF4};
  uchar ccapm= sfr->get(CCAPM[nr]);
  uint p1= sfr->get(P1);

  if (
      ((ccapm & bmCAPP) &&
       (prev_p1 & bmCEX[nr]) == 0 &&
       (p1 & bmCEX[nr]) != 0)
      ||
      ((ccapm & bmCAPN) &&
       (prev_p1 & bmCEX[nr]) != 0 &&
       (p1 & bmCEX[nr]) == 0)
      )
    {
      /* Capture */
      sfr->set(CCAPL[nr], sfr->get(CL));
      sfr->set(CCAPH[nr], sfr->get(CH));
      sfr->set_bit1(CCON, bmCCF[nr]);
    }

  if (ccapm & bmECOM)
    {
      /* Comparator enabled */
      if (sfr->get(CL) == sfr->get(CCAPL[nr]) &&
	  sfr->get(CH) == sfr->get(CCAPH[nr]))
	{
	  /* Match */
	  if (nr == 4 &&
	      (sfr->get(CMOD) & bmWDTE))
	    {
	      reset();
	    }
	  sfr->set_bit1(CCON, bmCCF[nr]);
	  if (ccapm & bmTOG)
	    {
	      /* Toggle */
	      sfr->set(P1, sfr->get(P1) ^ bmCEX[nr]);
	    }
	}
      if (ccapm & bmPWM)
	{
	  /* PWM */
	  if (sfr->get(CL) == 0)
	    sfr->set(CCAPL[nr], sfr->get(CCAPH[nr]));
	  if (sfr->get(CL) < sfr->get(CCAPL[nr]))
	    sfr->set(P1, sfr->get(P1) & ~(bmCEX[nr]));
	  else
	    sfr->set_bit1(P1, bmCEX[nr]);
	}
    }

  return(resGO);
}


/*
 */

cl_89c51r_dummy_hw::cl_89c51r_dummy_hw(class cl_uc *auc):
  cl_hw(auc, HW_DUMMY, 0, "_89c51r_dummy")
{}

int
cl_89c51r_dummy_hw::init(void)
{
  class cl_mem *sfr= uc->mem(MEM_SFR);
  if (!sfr)
    {
      fprintf(stderr, "No SFR to register %s[%d] into\n", id_string, id);
    }
  //auxr= sfr->register_hw(AUXR, this, 0);
  register_cell(sfr, AUXR, &auxr, wtd_restore);
  return(0);
}

void
cl_89c51r_dummy_hw::write(class cl_cell *cell, t_mem *val)
{
  if (cell == auxr)
    auxr->set_bit0(0x04);
}


/* End of s51.src/uc89c51r.cc */
