#ifndef UCCC2430CL_HEADER
#define UCCC2430CL_HEADER

#include "ddconfig.h"

#include "uc51cl.h"
#include "itsrccl.h"

class cl_uccc2430: public cl_51core
{
  public:
    cl_uccc2430(int Itype, int Itech, class cl_sim *asim);
    virtual void mk_hw_elements(void);
    virtual void make_memories(void);
};

#endif
