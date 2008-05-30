#include <stdio.h>
#include "utils.h"
#include "itsrccl.h"
#include "regs51.h"
#include "types51.h"
#include "memarbcl.h"

cl_memarb::cl_memarb(class cl_uc *auc):
  cl_hw(auc, HW_DUMMY, 20, "memarb")
{
}

int
cl_memarb::init(void)
{
  sfr = uc->address_space("sfr");
  rom = uc->address_space("rom");
  register_cell(sfr, 0xc7, &cell_memctr, wtd_none);
  old_memctr = cell_memctr->read();
  printf("MEMCTR = 0x%x\n", old_memctr);
  return(0);
}

int
cl_memarb::tick(int cycles)
{
  class cl_address_decoder *ad = (class cl_address_decoder *) rom->decoders->at(1);
  t_mem memctr = cell_memctr->read();
  if (memctr != old_memctr) {
    printf("MEMCTR changed -> 0x%x\n", memctr);
    old_memctr = memctr;
    ad->activated = 0;
    if ((memctr & 0x30) == 0x00) {
      printf("Mapping bank 0...\n", memctr);
      ad->chip_begin = 0x00000;
    }
    else if ((memctr & 0x30) == 0x10) {
      printf("Mapping bank 1...\n", memctr);
      ad->chip_begin = 0x08000;
    }
    else if ((memctr & 0x30) == 0x20) {
      printf("Mapping bank 2...\n", memctr);
      ad->chip_begin = 0x10000;
    }
    else if ((memctr & 0x30) == 0x30) {
      printf("Mapping bank 3...\n", memctr);
      ad->chip_begin = 0x18000;
    }
    ad->activate(0);
  }
  return(resGO);
}
