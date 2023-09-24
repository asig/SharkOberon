/*
 * Dummy interns.h sufficient to compile strlib.
 * Copyright (C) Advanced RISC Machines Limited 1994. All rights reserved.
 */

#ifndef __internals_h 
#define __internals_h

extern char *_strerror(int n, char *v);
extern void __set_strcoll_table(const unsigned char *table);

#endif
