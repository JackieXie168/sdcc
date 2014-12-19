#include <stdio.h>
#include <unistd.h>

#include "fiocl.h"
#include "utils.h"
#include "charscl.h"


int
main(int argc, char *argv[])
{
  class cl_f *f;
  int i, j;
  char buf[100];
  chars *fn;

  if (argc > 1)
    fn= new chars(argv[1]);
  else
    fn= new cchars("ftest.txt");
  f= new cl_f(*fn, cchars("w"));
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

  f->open(*fn, cchars("r"));
  j= 0;
  printf("j=%d\n", j);
  
  while (f->input_avail())
    {
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
