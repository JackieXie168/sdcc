#ifndef UCCC2430F128CL_HEADER
#define UCCC2430F128CL_HEADER

#include "ddconfig.h"

#include "uc51cl.h"
#include "itsrccl.h"

class cl_uccc2430f128: public cl_51core
{
  public:
    cl_uccc2430f128(int Itype, int Itech, class cl_sim *asim);
    virtual void mk_hw_elements(void);
    virtual void make_memories(void);
    virtual void clear_sfr(void);
    virtual class cl_memory_cell *get_indirect(uchar addr, int *res);
};

#endif
