#include <stdio.h>
#include "ddconfig.h"
#include "uccc2430f128cl.h"
#include "regs51.h"
#include "memarbcl.h"
// SOC specific hardware elements
// #include "..."

cl_uccc2430f128::cl_uccc2430f128(int Itype, int Itech, class cl_sim *asim):
  cl_51core(Itype, Itech, asim)
{
}

void
cl_uccc2430f128::mk_hw_elements(void)
{
  class cl_hw *h;

  cl_51core::mk_hw_elements();
  hws->add(h = new cl_memarb(this));
  h->init();
}

void
cl_uccc2430f128::make_memories(void)
{
  class cl_address_space *as;

  rom = as = new cl_address_space("rom", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  iram = as = new cl_address_space("iram", 0, 0x100, 8);
  as->init();
  address_spaces->add(as);

  sfr = as = new cl_address_space("sfr", 0x80, 0x80, 8);
  as->init();
  address_spaces->add(as);

  xram = as = new cl_address_space("xram", 0, 0x100000, 8);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip = new cl_memory_chip("rom_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = address_space("rom"), chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip = new cl_memory_chip("iram_chip", 0x100, 8);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = address_space("iram"), chip, 0, 0xff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip = new cl_memory_chip("xram_chip", 0x100000, 8);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = address_space("xram"), chip, 0, 0xfffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip = new cl_memory_chip("sfr_chip", 0x80, 8);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = address_space("sfr"), chip, 0x80, 0xff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  acc = sfr->get_cell(ACC);
  psw = sfr->get_cell(PSW);
}

void
cl_uccc2430f128::clear_sfr(void)
{
  cl_51core::clear_sfr();
}

class cl_memory_cell *
cl_uccc2430f128::get_indirect(uchar addr, int *res)
{
  *res = resGO;
  return (iram->get_cell(addr));
}

