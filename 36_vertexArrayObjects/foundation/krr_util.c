#include "krr_util.h"
#include <stdio.h>
#include <stdlib.h>

#if defined __APPLE__ || defined __linux__
#define HAS_BACKTRACE
#include <execinfo.h>
#endif

#ifdef HAS_BACKTRACE
static void* callstack[32];
#endif

void krr_util_print_callstack()
{
#ifdef HAS_BACKTRACE
  int frames = backtrace(callstack, 32);
  int i;
  char** strs = backtrace_symbols(callstack, frames);
  for (i=0; i<frames; i++)
  {
    printf("%s\n", strs[i]);
  }
  free(strs);
#endif
}
