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

  xram = as = new cl_address_space("xram", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  sfr = as = new cl_address_space("sfr", 0x80, 0x80, 8);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip = new cl_memory_chip("sram_chip", 0x2000, 8);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = iram, chip, 0, 0xff, 0x1f00);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
  ad = new cl_address_decoder(as = xram, chip, 0xe000, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip = new cl_memory_chip("sfr_chip", 0x80, 8);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = sfr, chip, 0x80, 0xff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
  ad = new cl_address_decoder(as = xram, chip, 0xdf80, 0xdfff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip = new cl_memory_chip("radio_chip", 0x80, 8);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = xram, chip, 0xdf00, 0xdf7f, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip = new cl_memory_chip("flash_chip", 0x20000, 8);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = xram, chip, 0x0000, 0xdeff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
  ad = new cl_address_decoder(as = rom, chip, 0x0000, 0x7fff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
  ad = new cl_address_decoder(as = rom, chip, 0x8000, 0xffff, 0x8000);
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

