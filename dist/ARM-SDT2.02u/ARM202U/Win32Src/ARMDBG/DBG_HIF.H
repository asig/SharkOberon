/*
 * ARM debugger toolbox : dbg_hif.c
 * Copyright (C) 1992 Advanced Risc Machines Ltd. All rights reserved.
 * Description of the Dbg_HostosInterface structure.  This is *NOT*
 * part of the debugger toolbox, but it is required by 2 back ends
 * (armul & pisd) and two front ends (armsd & wdbg), so putting it
 * in the toolbox is the only way of avoiding multiple copies.
 */

/*
 * RCS $Revision: 1.3 $
 * Checkin $Date: 1993/10/22 17:41:07 $
 * Revising $Author: hmeekings $
 */

#ifdef __STDC__
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

typedef void Hif_DbgPrint(void *arg, const char *format, va_list ap);
typedef void Hif_DbgPause(void *arg);

typedef void Hif_WriteC(void *arg, int c);
typedef int Hif_ReadC(void *arg);
typedef int Hif_Write(void *arg, char const *buffer, int len);
typedef char *Hif_GetS(void *arg, char *buffer, int len);

typedef void Hif_RDIResetProc(void *arg);

struct Dbg_HostosInterface {
    Hif_DbgPrint *dbgprint;
    Hif_DbgPause *dbgpause;
    void *dbgarg;

    Hif_WriteC *writec;
    Hif_ReadC *readc;
    Hif_Write *write;
    Hif_GetS *gets;
    void *hostosarg;

    Hif_RDIResetProc *reset;
    void *resetarg;
};
