/*
 * ieeeflt.h: interface to host-arithmetic-independent IEEE fp package.
 * Copyright (C) Codemist Ltd., 1994.
 * Copyright (C) Advanced RISC Machines Limited, 1994.
 */

/*
 * RCS $Revision: 1.2.2.1 $
 * Checkin $Date: 1995/05/13 17:04:14 $
 * Revising $Author: kwelton $
 */

#ifndef _ieeeflt_LOADED
#define _ieeeflt_LOADED

/*
 * The following types describe the representation of floating-point
 * values by the compiler in both binary and source-related forms.
 * The order of fields in DbleBin in is exploited only in object-code
 * formatters and assembly code generators.
 */

#include "host.h"

typedef struct DbleBin {
#ifdef TARGET_HAS_OTHER_IEEE_ORDER
  int32 lsd,msd;                /* e.g. clipper */
#else
  int32 msd,lsd;                /* e.g. arm, 370 (not really ieee) */
#endif
} DbleBin;

typedef struct FloatBin {
  int32 val;
} FloatBin;

#define flt_ok 0
#define flt_very_small 1
#define flt_very_big 2
#define flt_big_single 3
#define flt_small_single 4
#define flt_negative 5
#define flt_divide_by_zero 6
#define flt_invalidop 7 /* inf - int, inf / inf, anything involving NaNs */
#define flt_bad 8       /* invalid string for stod */

extern int fltrep_stod(const char *s, DbleBin *p, char **endp);
extern int fltrep_narrow(DbleBin const *d, FloatBin *e);
extern int fltrep_narrow_round(DbleBin const *d, FloatBin *e);
extern void fltrep_widen(FloatBin const *e, DbleBin *d);

extern int flt_add(DbleBin *a, DbleBin const *b, DbleBin const *c);
extern int flt_subtract(DbleBin *a, DbleBin const *b, DbleBin const *c);
extern int flt_multiply(DbleBin *a, DbleBin const *b, DbleBin const *c);
extern int flt_divide(DbleBin *a, DbleBin const *b, DbleBin const *c);
extern int flt_compare(DbleBin const *b, DbleBin const *c);
extern int flt_move(DbleBin *a, DbleBin const *b);
extern int flt_negate(DbleBin *a, DbleBin const *b);
extern int flt_abs(DbleBin *a, DbleBin const *b);

extern int flt_dtoi(int32 *n, DbleBin const *a);
extern int flt_dtou(unsigned32 *u, DbleBin const *a);

extern int flt_itod(DbleBin *a, int32 n);
extern int flt_utod(DbleBin *a, unsigned32 n);

void flt_frexp(DbleBin *res, DbleBin const *dp, int *lvn);
void flt_ldexp(DbleBin *res, DbleBin const *dp, int n);

int fltrep_dtos(char *p, int flags, int ch, int precision, int width,
                char *prefix, DbleBin const *d);
extern void fltrep_sprintf(char *b, char const *fmt, DbleBin const *dp);

#endif
