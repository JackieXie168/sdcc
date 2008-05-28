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
  sfr = uc->address_space(MEM_SFR_ID);
  /*
  if (sfr) {
    printf("sfr\n");
    watched_cells = new cl_list(1, 1, "watched_cells");
    watched_cells->add(new cl_watched_cell(sfr, 0xc7, NULL, wtd_none));
  }
  */
  register_cell(sfr, 0xc7, &cell_memctr, wtd_none);
  old_memctr = cell_memctr->read();
  return(0);
}

int
cl_memarb::tick(int cycles)
{
  if (cell_memctr->read() != old_memctr) {
    printf("MEMCTR changed -> %x\n", cell_memctr->read());
    // remap needed part of the 128k chip into ROM address space
    // using a cl_address_decoder (see some examples in cmd.src/cmdmem.cc)
    old_memctr = cell_memctr->read();
  }
  return(resGO);
}
