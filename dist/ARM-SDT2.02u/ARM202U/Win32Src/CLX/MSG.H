/* msg.h -- interface to the message file and nls system. */
/* Copyright (C) Advanced RISC Machines Limited, 1995. All rights reserved. */

/*
 * RCS $Revision: 1.8 $
 * Checkin $Date: 1995/03/14 17:09:16 $
 * Revising $Author: mwilliam $
 */

#ifndef msg_h
#define msg_h

#include <stdio.h>

#ifdef NLS
#include <stdarg.h>

/*
 * Initialise a tool. Passed in the name of the tool, and it attempts to
 * locate the message file (in a host dependent way). Can be called
 * multiple times for various components of a tool - e.g.:
 *    msg_init("armdbg",NULL); msg_init("armsd",NULL); msg_init("armul",NULL);
 *
 * toolname may be a full pathname to the tool - e.g. argv[0] under DOS,
 * in which case that directory will be searched for the messagefile too.
 */
extern void msg_init(const char * /*toolname*/,
                     const char * /*default_name*/);

/* Open and "closes" a message file. Once closed, all attempts to lookup a
 * message will fail. close also frees any internally allocated store.
 */

/* It is possible, at the moment, to call msg_open() multiple times, and the
 * message files will be merged (new entries overwriting old). However any
 * call to msg_close() will destroy the entire hash table, unfortunately.
 */
extern char *msg_open(const char * /*filename*/); /* Returns NULL on error */
extern void msg_close(char * /*value_returned_by_msg_open*/);

/* At the moment if you call msg_lookup(), or any function which in turn
 * calls msg_lookup() will call msg_open("messages") if no msg_open call
 * has yet been called.
 */

typedef const unsigned char *msg_t;    /* type of message tag */

/* printf variants */
extern int msg_printf(msg_t /*tag*/, ...);
extern int msg_sprintf(char *, msg_t /*tag*/, ...);
extern int msg_fprintf(FILE *, msg_t /*tag*/, ...);
extern int msg_vsprintf(char *, msg_t /*tag*/, va_list);
extern int msg_vfprintf(FILE *, msg_t /*tag*/, va_list);

/* Low-level find-a-message-from-a-tag routine */
/* If the lookup fails, it tries the equivalent of returning the result
 * from msg_sprintf("msg_lookup_failed",tag);
 */
extern char *msg_lookup(msg_t /*tag*/);


/* ==== nls printf functions (take format strings, not tags) ==== */

extern int nls_printf(const char * /*fmt*/, ...);
extern int nls_sprintf(char *,const char * /*fmt*/, ...);
extern int nls_fprintf(FILE *,const char * /*fmt*/, ...);
extern int nls_vfprintf(FILE *,const char * /*fmt*/, va_list);

/* ==== Even lower-level things ==== */
typedef union {
  double floating;
  long cardinal;
  void *pointer;
} nls_type;

/* These functions take a *format* string, not a tag. */
extern nls_type *nls_va2type(const char * /*fmt*/, va_list);
extern int nls_tsprintf(char *,const char * /*fmt*/, nls_type *);
extern int nls_tfprintf(FILE *,const char * /*fmt*/, nls_type *);

#else

typedef char *msg_t;    /* type of message tag */

#  define msg_printf printf
#  define msg_sprintf sprintf
#  define msg_fprintf fprintf
#  define msg_vsprintf vsprintf
#  define msg_vfprintf vfprintf

#  define nls_printf printf
#  define nls_sprintf sprintf
#  define nls_fprintf fprintf
#  define nls_vfprintf vfprintf

#  define msg_init(X,Y)         /* null def'n */
#  define msg_open(X)
#  define msg_close(X)

#  define msg_lookup(X) X
#endif

#endif
