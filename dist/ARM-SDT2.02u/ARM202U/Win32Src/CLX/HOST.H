/*
 * C compiler file mip/host.h
 * Copyright (C) Codemist Ltd., 1988
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.15.2.7 $  Codemist 11
 * Checkin $Date: 1995/06/29 10:47:31 $
 * Revising $Author: kwelton $
 */

/* AM memo, July 1990: in principle there should be no tests of         */
/* COMPILING_ON_<machine>, but only COMPILING_ON_<operating system> or  */
/* COMPILING_ON_<manufacturer> (for special features).                  */
/* Accordingly COMPILING_ON_<machine> is deprecated.                    */

/*
 * This file deals with peculiarities of the host system under which the
 * compiler is compiled AND peculiarities of the host system under which
 * the compiler will run (hence it might need further explication in the
 * unlikely event that we wish to cross-compile the compiler (repeatedly
 * as opposed to once-off bootstrap)).   It is now loaded first and can
 * therefore not depend on TARGET_xxx parameterisations, these are now
 * done in target.h or, if systematic, in mip/defaults.h.
 * The correct mechanism for host->target dependencies (e.g. if compiling
 * on unix then make a unix compiler, else a homebrew object file version)
 * is via the options.h file, along the lines of:
 *   #ifdef COMPILING_ON_UNIX
 *   #  define TARGET_IS_UNIX
 *   #endif
 * The intent is that most of the pecularities should be linked to
 * COMPILING_ON_machine and/or COMPILING_ON_system.  Further NO OTHER FILE
 * should refer to magic names like 'unix', '__arm' etc., but go via
 * the COMPILING_xxx flags defined here in terms of these.
 * The aim is that this file should suffice for all host dependencies
 * and thus all COMPILING_ON_xxx tests outwith are suspect.  However,
 * the #include file munger clearly needs to so depend.
 */

#ifndef _host_LOADED
#define _host_LOADED 1

#include <stdio.h>
#include <stddef.h>     /* for xsyn.c (and more?) and offsetof test below */
#include <stdlib.h>     /* for EXIT_FAILURE test below */

#ifndef SEEK_SET
#  define SEEK_SET  0
#endif
#ifndef SEEK_CUR
#  define SEEK_CUR  1
#endif
#ifndef SEEK_END
#  define SEEK_END  2
#endif

#ifdef __STDC__

#define BELL      '\a'
typedef void                  /* newline to fool the topcc tool */
             *VoidStar;
typedef const void
             *ConstVoidStar;
#define safe_tolower(ch)      tolower(ch)  /* see comment below */
#define safe_toupper(ch)      toupper(ch)  /* see comment below */

#else  /* ! __STDC__ */

#define BELL      '\007'
typedef char *VoidStar;
#define  ConstVoidStar VoidStar
/*
 * not all C libraries define tolower() and toupper() over all character
 * values. BSD Unix, for example, defines tolower() only over UC chars.
 */
#define safe_tolower(ch)      (isupper(ch) ? tolower(ch) : ch)
#define safe_toupper(ch)      (islower(ch) ? toupper(ch) : ch)

#endif /* ! __STDC__ */

/* The following for the benefit of compiling on SunOS */
#ifndef offsetof
#  define offsetof(T, member)  ((char *)&(((T *)0)->member) - (char *)0)
#endif

#ifdef unix                   /* A temporary sop to older compilers */
#  ifndef __unix              /* (good for long-term portability?)  */
#    define __unix    1
#  endif
#endif

#ifdef __unix
/* Generic unix -- hopefully a split into other variants will not be    */
/* needed.  However, beware the 'bsd' test above and safe_toupper etc.  */
/* which cope with backwards (pre-posix/X/open) unix compatility.       */
#  define COMPILING_ON_UNIX     1
#endif
#ifdef __helios
/* start improving parameterisation.  Maybe we should also set          */
/* COMPILING_ON_UNIX and use HELIOS as a special subcase?               */
#  define COMPILING_ON_HELIOS   1
#endif
#ifdef __acorn
#  define COMPILING_ON_ACORN_KIT  1
#endif
#ifdef __riscos
#  define COMPILING_ON_RISC_OS  1
#endif
#ifdef __arm
#  define COMPILING_ON_ARM      1  /* dying: unix/riscos/acorn suffice  */
#endif
#ifdef __ibm370
#  ifndef COMPILING_ON_UNIX
#     define __mvs 1               /* a hack to be removed soon        */
#  endif
#endif
#ifdef __mvs
#  define COMPILING_ON_MVS      1
#endif
#ifdef _MSDOS
#  define COMPILING_ON_MSDOS    1
#endif
#ifdef __ZTC__
#  define COMPILING_ON_MSDOS    1
#endif
#ifdef __WATCOMC__
#  define COMPILING_ON_MSDOS    1
#endif
#ifdef _MSC_VER
#  define COMPILING_ON_MSDOS    1
#endif
#ifdef macintosh
#  define COMPILING_ON_MACINTOSH 1 /* for things that depend on how Macintosh handles filenames, etc. */
#  define COMPILING_ON_MPW 1      /* for things that depend on how MPW handles arguments, i/o, etc. */
#endif

/*
 * The following typedefs may need alteration for obscure host machines.
 */
#ifdef __alpha
typedef                int   int32;
typedef unsigned       int   unsigned32;
#else
typedef          long  int   int32;
typedef unsigned long  int   unsigned32;
#endif
typedef          short int   int16;
typedef unsigned short int   unsigned16;
typedef   signed       char  int8;
typedef unsigned       char  unsigned8;
typedef                int   bool;

/*
 * The following sets ArgvType for 64-bit pointers so that
 * DEC Unix (OSF) cc can be used with the -xtaso_short compiler option
 * to force pointers to be 32-bit.
 */
#if defined(__alpha) && defined(__osf__) 
#pragma pointer_size (save) 
#pragma pointer_size (long) 
#endif
typedef char *ArgvType; 
#if defined(__alpha) && defined(__osf__) 
#pragma pointer_size (restore) 
#endif

#define YES   1
#define TRUE  1
#define NO    0
#define FALSE 0

/*
 * Rotate macros
 */
#define ROL_32(val, n) \
((((unsigned32)(val) << (n)) | ((unsigned32)(val) >> (32-(n)))) & 0xFFFFFFFFL)
#define ROR_32(val, n) \
((((unsigned32)(val) >> (n)) | ((unsigned32)(val) << (32-(n)))) & 0xFFFFFFFFL)

/* The following two lines are a safety play against using ncc with a   */
/* vendor supplied library, many of which refuse to accept "wb".  POSIX */
/* and other unix standards require "wb" and "w" to have the same       */
/* effect and so the following is safe.                                 */
#ifdef COMPILING_ON_UNIX
#  define FOPEN_WB     "w"
#  define FOPEN_RB     "r"
#  define FOPEN_RWB    "r+"
#  ifndef __STDC__                     /* caveat RISCiX... */
#    define remove(file) unlink(file)  /* a horrid hack, but probably best? */
#  endif
#else
#  define FOPEN_WB     "wb"
#  define FOPEN_RB     "rb"
#  define FOPEN_RWB    "rb+"
#endif

/* (defined __sparc && defined P_tmpdir) is to detect gcc on SunOS */
#if !defined __STDC__ || (defined __sparc && defined P_tmpdir)
/* Use bcopy rather than memmove, as memmove is not available.     */
#  define memmove(d,s,l) bcopy(s,d,l)
/* BSD/SUN don't have strtoul(), but then strtol() doesn't barf on */
/* overflow as required by ANSI... This bodge is horrid.           */
#  define strtoul(s, ptr, base) strtol(s, ptr, base)
/* strtod is present in the C-library but is not in stdlib.h       */
extern double strtod(const char *str, char **ptr);
#endif

#ifdef __CC_NORCROFT
#  ifndef COMPILING_ON_UNIX     /* suppress if maybe non-norcroft lib */
#    define LIBRARY_IS_NORCROFT 1
#  endif
#  ifdef LIBRARY_IS_ALIEN       /* unify with the 3 previous lines?   */
#    undef LIBRARY_IS_NORCROFT
#  endif
#endif

#ifdef LIBRARY_IS_NORCROFT
/*
 * Calls to all non-ansi functions are removable by macros here.
 */
#ifdef __cplusplus
extern "C" {
#endif
/* These routines have no floating point abilities.                     */
extern int _vfprintf(FILE *stream, const char *format, __va_list arg);
extern int _vsprintf(char *s, const char *format, __va_list arg);
extern int _fprintf(FILE *stream, const char *format, ...);
extern int _sprintf(char *s, const char *format, ...);
#ifdef __cplusplus
}
#endif
#else /* LIBRARY_IS_NORCROFT */
#  define _vfprintf vfprintf
#  define _vsprintf vsprintf
#  define _fprintf fprintf
#  define _sprintf sprintf
#endif /* LIBRARY_IS_NORCROFT */

#ifdef COMPILING_ON_MVS

#define HOST_USES_CCOM_INTERFACE 1
#define EXIT_warn                4
#define EXIT_error               8
#define EXIT_fatal              12
#define EXIT_syserr             16

/*
 * The following #included #define's ensure that external symbols are
 * limited to 6 chars without gratuitous changes to every file.
 */
#include "sixchar.h"

#else  /* ! COMPILING_ON_MVS */

#define EXIT_warn                0
#define EXIT_error               1
#define EXIT_fatal               1

#ifdef  COMPILING_ON_UNIX
#  define EXIT_syserr            100
#else
#  define EXIT_syserr            1
#endif

#endif /* ! COMPILING_ON_MVS */

/* For systems that do not define EXIT_SUCCESS and EXIT_FAILURE */
#ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS           0
#endif
#ifndef EXIT_FAILURE
#  define EXIT_FAILURE           EXIT_error
#endif

#endif

/* end of host.h */
