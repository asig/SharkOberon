#include "memmove.h"

void *__rt_memmove(void *a, const void *b, size_t n)
/* copy memory taking care of overlap */
/* Relies on sizeof(int)=sizeof(void *) and byte addressing.
   Also that memory does not wrap round for direction test. */
{
    /* do it fast if word aligned ... */
    if ((((int)a | (int)b | (int)n) & 3) == 0)
    { int *wa,*wb;
      n >>= 2;
      if (a < (void *)b)
         for (wa = (int *)a, wb = (int *)b; n-- > 0;) *wa++ = *wb++;
      else for (wa = n+(int *)a, wb = n+(int *)b; n-- > 0;) *--wa = *--wb;
    }
    else
    { char *ca,*cb;
      if (a < (void *)b)
         for (ca = (char *)a, cb = (char *)b; n-- > 0;) *ca++ = *cb++;
      else for (ca = n+(char *)a, cb = n+(char *)b; n-- > 0;) *--ca = *--cb;
    }
    return a;
}
