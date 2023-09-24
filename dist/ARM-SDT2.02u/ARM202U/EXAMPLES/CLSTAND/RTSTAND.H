/*
 * Standalone C run-time kernel.
 * Copyright (C) 1991 Advanced RISC Machines Limited.
 */

#ifndef __rtstand_h
#define __rtstand_h


extern void __rt_exit(int /* code */); /*
 * Terminate execution; equivalent to returning from main.
 * NOTE: all other details are determined by your implementation.
 */

typedef struct {unsigned errnum;  char errmess[252];} __rt_error;
typedef struct {int r[16];} __rt_registers;

extern void __err_handler(__rt_error *, __rt_registers *);

extern int __rt_fpavailable(void);
/*
 * Return non-0 iff there is support for the floating-point instruction set.
 */

extern unsigned __rt_alloc(unsigned /*minwords*/, void ** /*block*/);
/*
 *  Tries to allocate a block of sensible size >= minwords.  Failing that,
 *  it allocates the largest possible block (may have size zero).
 *  Sensible size is determined by your implementation (default: 256 words).
 *  *block is set to a pointer to the start of the allocated block
 *  (NULL if 'a block of size zero' has been allocated).
 */

#ifdef __JMP_BUF_SIZE
typedef int jmp_buf[__JMP_BUF_SIZE];
#else
typedef int jmp_buf[22];    /* size suitable for the ARM                   */
#endif                      /* an array type suitable for holding the data */
                            /* needed to restore a calling environment.    */
#ifdef __STDC__
/* setjmp is a macro so that it cannot be used other than directly called. */
/* NB that ANSI declare that anyone who undefined the setjmp macro or uses */
/* (or defines) the name setjmp without including this header will get     */
/* what they deserve. NOTE: -pcc mode doesn't allow circular definitions...*/
#define setjmp(jmp_buf) (setjmp(jmp_buf))
#endif

extern int setjmp(jmp_buf /*env*/);

extern int longjmp(jmp_buf /*env*/, int /*val*/);

#endif
