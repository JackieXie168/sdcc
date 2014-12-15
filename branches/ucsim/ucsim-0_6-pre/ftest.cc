#include <stdio.h>
#include <unistd.h>

#include "fiocl.h"
#include "utils.h"
#include "charscl.h"


int
main(int argc, char *argv[])
{
  class cl_f *f= new cl_f(cchars("ftest.txt"), cchars("w"));
  int i, j;
  char buf[100];

  f->init();

  f->write_str("proba\n");

  /*
    f->open(cchars("ftest.txt"), cchars("r"));
  i= read(f->id(), buf, 99);
  if (i)
    {
      buf[i]= 0;
      printf("%s", buf);
    }
  */

  f->open(cchars("ftest.txt"), cchars("r"));
  j= 0;
  printf("j=%d\n", j);
  
  while (f->input_avail())
    {
      i= f->read(buf, 99);
      printf("i=%d\n", i);
      if (i)
	buf[i]= 0;
      else
	break;
      printf("%s", buf);
    }
  f->close();

  delete f;

  double last= dnow();
  while (1)
    {
      if (dnow() - last > 0.5)
	{
	  last= dnow();
	  printf("HUKK\n");
	}
    }
  return 0;
}
