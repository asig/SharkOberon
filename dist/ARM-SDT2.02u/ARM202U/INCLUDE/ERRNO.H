#pragma force_top_level
#pragma include_only_once

/* errno.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.1.3 */
/* Copyright (C) Codemist Ltd., 1988                            */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */
/* version 1 */

#ifndef __errno_h
#define __errno_h

#ifndef errno
#  define errno __errno
extern volatile int errno;
   /*
    * expands to a modifiable lvalue that has type volatile int, the value of
    * which is set to a positive error code by several library functions. It is
    * initialised to zero at program startup, but is never set to zero by any
    * library function. The value of errno may be set to nonzero by a library
    * function call whether or not there is an error, provided the use of errno
    * is not documented in the description of the function in the Standard.
    */
#endif

#define EDOM    1
   /*
    * if a domain error occurs (an input argument is outside the domain over
    * which the mathematical function is defined) the integer expression errno
    * acquires the value of the macro EDOM and HUGE_VAL is returned. EDOM may
    * be used by non-mathematical functions.
    */
#define ERANGE 2
   /*
    * a range error occurs if the result of a function can not be represented
    * as a double value. If the result overflows (the magnitude of the result
    * is so large that it cannot be represented in an object of the specified
    * type), the function returns the value of the macro HUGE_VAL, with the
    * same sign as the correct value of the function; the integer expression
    * errno acquires the value of the macro ERANGE. If the result underflows
    * (the magnitude of the result is so small that it cannot be represented
    * in an object of the specified type), the function returns zero; the
    * integer expression errno acquires the value of the macro ERANGE. ERANGE
    * may be used by non-mathematical functions.
    */
#define ESIGNUM 3

#endif

/* end of errno.h */
