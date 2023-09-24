#pragma force_top_level
#pragma include_only_once

/* limits.h: ANSI 'C' (X3J11 Oct 88) library header, section 2.2.4.2 */
/* Copyright (C) Codemist Ltd., 1988                            */
/* Copyright (C) Advanced Risc Machines Ltd., 1991              */
/* version 0.01 */

#ifndef __limits_h
#define __limits_h

#define CHAR_BIT 8
    /* max number of bits for smallest object that is not a bit-field (byte) */
#define SCHAR_MIN (-128)
    /* mimimum value for an object of type signed char */
#define SCHAR_MAX 127
    /* maximum value for an object of type signed char */
#define UCHAR_MAX 255
    /* maximum value for an object of type unsigned char */
#define CHAR_MIN 0
    /* minimum value for an object of type char */
#define CHAR_MAX 255
    /* maximum value for an object of type char */
#define MB_LEN_MAX 1
    /* maximum number of bytes in a multibyte character, */
    /* for any supported locale */

#define SHRT_MIN  (-0x8000)
    /* minimum value for an object of type short int */
#define SHRT_MAX  0x7fff
    /* maximum value for an object of type short int */
#define USHRT_MAX 65535U
    /* maximum value for an object of type unsigned short int */
#define INT_MIN   (~0x7fffffff)  /* -2147483648 and 0x80000000 are unsigned */
    /* minimum value for an object of type int */
#define INT_MAX   0x7fffffff
    /* maximum value for an object of type int */
#define UINT_MAX  0xffffffff
    /* maximum value for an object of type unsigned int */
#define LONG_MIN  (~0x7fffffff)
    /* minimum value for an object of type long int */
#define LONG_MAX  0x7fffffff
    /* maximum value for an object of type long int */
#define ULONG_MAX 0xffffffffU
    /* maximum value for an object of type unsigned long int */

#endif

/* end of limits.h */
