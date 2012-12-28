/*
 * Simulator of microcontrollers (fiocl.h)
 *
 * Copyright (C) 1997,12 Drotos Daniel, Talker Bt.
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

#ifndef FIOCL_HEADER
#define FIOCL_HEADER

#include "pobjcl.h"


/* Regular file */

class cl_f: public cl_base
{
 private:
  char *file_name, *file_mode;
  FILE *file_f;
  int file_id;
 public:
  cl_f(char *fn, char *mode);
  virtual ~cl_f(void);
  virtual int init(void);
  virtual int open(void) { return init(); }
  virtual int open(char *fn);
  virtual int open(char *fn, char *mode);
  virtual int close(void);
  
  FILE *f(void) { return file_f; };
  int id(void) { return file_id; };

  virtual int read(char *buf, int max);
  virtual int write(char *buf, int count);
  virtual int write_str(char *s);
  virtual int write_str(const char *s);

  virtual int raw(void);
  virtual int cooked(void);
  virtual int input_avail(void);
};


#endif

/* End of fiocl.h */
