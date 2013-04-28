/* 0fail.c
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testFail(void)
{
  ASSERT (0);
}
