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
  ad = (class cl_address_decoder *) rom->decoders->at(1);
  register_cell(sfr, 0xc7, &cell_memctr, wtd_none);
  old_memctr = cell_memctr->read();
  old_acc = sfr->read(0xe0);
  printf("MEMCTR = 0x%x\n", old_memctr);
  return(0);
}

int
cl_memarb::tick(int cycles)
{
  t_mem memctr = cell_memctr->read();
  t_mem acc = sfr->read(0xe0);
  if (memctr != old_memctr) {
    //printf("MEMCTR changed -> 0x%x\n", memctr);
    old_memctr = memctr;
    ad->activated = 0;
    if ((memctr & 0x30) == 0x00) {
      //printf("Mapping bank 0...\n");
      ad->chip_begin = 0x00000;
    }
    else if ((memctr & 0x30) == 0x10) {
      //printf("Mapping bank 1...\n");
      ad->chip_begin = 0x08000;
    }
    else if ((memctr & 0x30) == 0x20) {
      //printf("Mapping bank 2...\n");
      ad->chip_begin = 0x10000;
    }
    else if ((memctr & 0x30) == 0x30) {
      //printf("Mapping bank 3...\n");
      ad->chip_begin = 0x18000;
    }
    ad->activate(0);
  }
  /*
  if (acc != old_acc) {
    printf("ACC changed -> 0x%x\n", acc);
    old_acc = acc;
  }
  */
  return(resGO);
}