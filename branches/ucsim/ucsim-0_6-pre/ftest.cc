#include <stdio.h>
#include <unistd.h>

#include "fiocl.h"
#include "utils.h"
#include "charscl.h"


void
regular_ftest(char *fn)
{
  int i;
  class cl_f *f;

  printf("Testing regular file access: %s\n", fn);
  printf("Write test\n");
  f= new cl_f(fn, cchars("w"));
  f->init();
  f->write_str("proba\n");

  printf("Read test\n");
  f->open(fn, cchars("r"));
  while (f->input_avail())
    {
      char buf[100];
      i= f->read(buf, 99);
      printf("read=%d\n", i);
      if (i)
	buf[i]= 0;
      else
	break;
      printf("data:%s\n", buf);
    }
  f->close();

  delete f;
}

int
main(int argc, char *argv[])
{
  char *fn;

  if (argc > 1)
    fn= argv[1];
  else
    fn= (char*)"ftest.txt";
  regular_ftest(fn);
  
  double last= dnow();
  int cnt= 0;
  while (1)
    {
      if (dnow() - last > 0.5)
	{
	  last= dnow();
	  printf("HUKK\n");
	  if (++cnt > 5)
	    return 0;
	}
    }
  return 0;
}
