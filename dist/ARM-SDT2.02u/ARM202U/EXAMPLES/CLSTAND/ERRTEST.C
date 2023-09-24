/*
 * Standalone Hello World program - tests for presence of the FP
 *         support code and also shows how to interface to the
 *         standalone C kernel's error handler.
 *
 * Copyright (C) 1993 Advanced RISC Machines Limited.
 */

#include "rtstand.h"

extern void __swi(0) put_char(int ch);
extern void __swi(2) put_string(char *string);

/* First let's roll our own, primitive hex number printer.
 * Strictly %#.8X format.
 */
static void puth(unsigned n) {
  int j;
  put_string("0X");
  for (j = 0;  j < 8;  ++j) {
    put_char("0123456789ABCDEF"[n >> 28]);
    n <<= 4;
  }
}

static jmp_buf err_label;

/* This is the function weakly-referenced from the standalone C kernel.
 * If it exists, it will be called when a run-time error occurs.
 * If the error is a 'pseudo-error', raised because the error-handler
 * has been called directly, then the user's register set *r will contain
 * random values for a1-a4 and ip (r[0-3], r[12]) and r[15] will be
 * identical to r[14].
 */
void __err_handler(__rt_error *e, __rt_registers *r) {
  put_string("errhandler called: code = ");
  puth(e->errnum);
  put_string(": ");  put_string(e->errmess);  put_string("\r\n");
  put_string("caller's pc = ");  puth(r->r[15]);
  put_string("\r\nreturning...\r\n");
#ifdef LONGJMP
  longjmp(err_label, e->errnum);
#endif
}

#ifdef DIVIDE_ERROR
#define LIMIT 0
#else
#define LIMIT 1
#endif

int main(int argc, char *argv[]) {
  int rc;

  put_string("(the floating point instruction-set is ");
  if (!__rt_fpavailable()) put_string("not ");
  put_string("available)\r\n");

/* Set up the jmp_buffer, and if returning due to longjmp then
 * goto errlabel
 */
  if ((rc = setjmp(err_label)) != 0) goto errlabel;

  if (__rt_fpavailable()) {
    float a;
    put_string("Using Floating point, but casting to int ...\r\n");
    for (a=(float) 10.0;a>=(float) LIMIT;a-=(float) 1.0) {
      put_string("10000 / "); puth((int) a); put_string(" = ");
      puth((int) (10000.0/a)); put_string("\r\n");
    }
  } else {
    int a;
    put_string("Using integer arithmetic ...\r\n");
    for (a=10;a>=LIMIT;a--) {
      put_string("10000 / "); puth(a); put_string(" = ");
      puth(10000/a); put_string("\r\n");
    }
  }
  return 0;

errlabel:
  put_string("\nReturning from __err_handler() with errnum = ");
  puth(rc);
  put_string("\r\n\n");
  return 0;
}
