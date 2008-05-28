#ifndef MEMARB_HEADER
#define MEMARB_HEADER

#include "stypes.h"
#include "pobjcl.h"
#include "uccl.h"
#include "newcmdcl.h"

class cl_memarb: public cl_hw
{
  protected:
    class cl_address_space *sfr;
    class cl_memory_cell *cell_memctr;
    t_mem old_memctr;
  public:
    cl_memarb(class cl_uc *auc);
    //virtual ~cl_memarb(void);
    virtual int init(void);
    /*
    virtual void new_hw_added(class cl_hw *new_hw);
    virtual void added_to_uc(void);
    virtual t_mem read(class cl_memory_cell *cell);
    virtual void write(class cl_memory_cell *cell, t_mem *val);
    virtual void mem_cell_changed(class cl_address_space *mem, t_addr addr);
    */
    virtual int tick(int cycles);
    /*
    virtual void reset(void);
    virtual void happen(class cl_hw *where, enum hw_event he, void *params);
    virtual void print_info(class cl_console_base *con);
    */
};

#endif
