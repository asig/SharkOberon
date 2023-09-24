/*
 * Standalone run-time system test.
 *
 * This program checks that stack overflow checking works and that it
 * interacts properly with heap extension.
 *
 * Copyright (C) 1991 Advanced RISC Machines Limited.
 */

#include "rtstand.h"

extern void __swi(0) put_char(int ch);
extern void __swi(2) put_string(char *string);

/* First, we make a function to claim a large(-ish) stack frame.
 * Some obfuscation in case the compiler optimises us away...
 */
static int stack(int n, int v) {
  /* claim n KB of stack */
  int x[256],i;

  if (n > 1) v = stack(n-1, v);
  for (i = 0;  i < 256;  ++i) x[i] = v + i;
  return x[0] + x[255];
}

/* Now we roll our own decimal output function - strictly %d format...
 */
static void puti(int n) {
  if (n < 0) {
    put_char('-');
    n = -n;
  }
  if (n > 9) {
    int n1 = n / 10;
    n = n % 10;
    puti(n1);
  }
  put_char(n + '0');
}
 
/* ...and a hex outputter... strictly %#.8X format.
 */
static void puth(unsigned n) {
  int j;
  put_string("0X");
  for (j = 0;  j < 8;  ++j) {
     put_char("0123456789ABCDEF"[n >> 28]);
     n <<= 4;
  }
}

/* Finally, we sit in a loop extending the heap and claiming ever bigger
 * stack franes until something gives. Probably, the heap will give first,
 * as currently tuned, and the program will announce "memory exhausted".
 * If you tune it differently, it can be made to fail will a stack overflow
 * run-time error. Try compiling this -DSTACK_OVERFLOW to provoke it.
 */
int main(int argc, char *argv[]) {
  unsigned size, ask, got, total;
  void *base;

  put_string("kernel memory management test\r\n");

  size = 4;  /* KB */
  ask = 0;
  for (total = 0;;) {
    put_string("force stack to "); puti(size);  put_string("KB\r\n");
    stack(size, 0);
    put_string("request "); puti(ask);  put_string(" words of heap - ");
    got = __rt_alloc(ask, &base);
    total += got;
    put_string("allocate "); puti(got);
    put_string(" words at "); puth((unsigned)base); put_string("\r\n");
    if (got < ask) break;
    ask += got / 2;
#ifdef STACK_OVERFLOW
    size *= 2;
#else
    size += 4;
#endif
  }

  put_string("memory exhausted, ");
  puti(total); put_string(" words of heap, ");
  puti(size);  put_string("KB of stack\r\n"); 

  return 0;
}
