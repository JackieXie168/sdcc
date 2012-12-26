#include <stdio.h>

#include "fiocl.h"

#include "charscl.h"


int
main(int argc, char *argv[])
{
  class cl_f *f= new cl_f(cchars("ftest.txt"), cchars("w"));
  f->init();

  f->write_str("proba\n");
  delete f;

  return 0;
}
