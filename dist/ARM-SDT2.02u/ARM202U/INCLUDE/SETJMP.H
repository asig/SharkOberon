#pragma force_top_level
#pragma include_only_once

/* setjmp.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.6 */
/* Copyright (C) Codemist Ltd., 1988                            */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */
/* version 0.04 */

/*
 * setjmp.h declares two functions and one type, for bypassing the normal
 * function call and return discipline (useful for dealing with unusual
 * conditions encountered in a low-level function of a program).
 */

#ifndef __setjmp_h
#define __setjmp_h

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __JMP_BUF_SIZE
typedef int jmp_buf[__JMP_BUF_SIZE];
#else
typedef int jmp_buf[22];    /* size suitable for the ARM */
#endif                      /* an array type suitable for holding the data */
                            /* needed to restore a calling environment.    */

/* setjmp is a macro so that it cannot be used other than directly called. */
/* NB that ANSI declare that anyone who undefined the setjmp macro or uses */
/* (or defines) the name setjmp without including this header will get     */
/* what they deserve.                                                      */

#ifdef __STDC__
/* -pcc mode doesn't allow circular definitions... */
#define setjmp(jmp_buf) (setjmp(jmp_buf))
#endif

extern int setjmp(jmp_buf /*env*/);
   /* Saves its calling environment in its jmp_buf argument, for later use
    * by the longjmp function.
    * Returns: If the return is from a direct invocation, the setjmp function
    *          returns the value zero. If the return from a call to the longjmp
    *          function, the setjmp function returns a non zero value.
    */

extern void longjmp(jmp_buf /*env*/, int /*val*/);
   /* Restores the environment saved by the most recent call to setjmp in the
    * same invocation of the program, with the corresponding jmp_buf argument.
    * If there has been no such call, or if the function containing the call
    * to setjmp has terminated execution (eg. with a return statement) in the
    * interim, the behaviour is undefined.
    * All accessible objects have values as of the time longjmp was called,
    * except that the values of objects of automatic storage duration that do
    * not have volatile type and have been changed between the setjmp and
    * longjmp calls are indeterminate.
    * As it bypasses the usual function call and return mechanism, the longjmp
    * function shall execute correctly in contexts of interrupts, signals and
    * any of their associated functions. However, if the longjmp function is
    * invoked from a nested signal handler (that is, from a function invoked as
    * a result of a signal raised during the handling of another signal), the
    * behaviour is undefined.
    * Returns: After longjmp is completed, program execution continues as if
    *          the corresponding call to setjmp had just returned the value
    *          specified by val. The longjmp function cannot cause setjmp to
    *          return the value 0; if val is 0, setjmp returns the value 1.
    */

#ifdef __cplusplus
}
#endif

#endif

/* end of setjmp.h */
