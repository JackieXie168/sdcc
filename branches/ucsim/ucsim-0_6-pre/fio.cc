/*
 * Simulator of microcontrollers (fio.cc)
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

#include <stdio.h>
#include <unistd.h>

#include "fiocl.h"


cl_f::cl_f(char *fn, char *mode):
  cl_base()
{
  file_f= NULL;
  file_id= -1;
  file_name= fn;
  file_mode= mode;
}


int
cl_f::init(void)
{
  if ((file_f= fopen(file_name, file_mode)) != NULL)
    file_id= fileno(file_f);
  return file_id;
}


cl_f::~cl_f(void)
{
  if (file_f)
    fclose(file_f);
  file_f= NULL;
  file_id= -1;
}


/* IO primitives */

int
cl_f::read(char *buf, int max)
{
  return ::read(file_id, buf, max);
}


int
cl_f::write(char *buf, int count)
{
  return ::write(file_id, buf, count);
}


int
cl_f::write_str(char *s)
{
  return fprintf(file_f, "%s", s);
}


/* Device handling */

int
cl_f::raw(void)
{
  return 0;
}


int
cl_f::cooked(void)
{
  return 0;
}


int
cl_f::input_avail(void)
{
  return 0;
}


/* End of fio.cc */
