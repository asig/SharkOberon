/*
 * config.h - modified specially for reentrant strlib.
 * Copyright (C) Advanced RISC Machines Limited 1994. All rights reserved.
 */

#define BYTESEX_EVEN   1    /* little-endian */  
/*#define BYTESEX_ODD    1  -- BIG-endian */

/*
 * Define this to get a shared library with an initialised data image in the
 * library. Undefine it to get the data image in the library stub. Remember
 * To modify strshl in step (the line containing + {}).
 */
#define DATA_IN_LIBRARY 1         /* needs +{} in strshl */

#include "interns.h"

#define memcpy_c
#define memmove_c
#define memchr_c
#define memcmp_c
#define memset_c
#define strcat_c
#define strchr_c
#define strcmp_c
#define strcoll_c
#define strcpy_c
#define strcspn_c
#define strerror_c
#define strlen_c
#define strncat_c
#define strncmp_c
#define strncpy_c
#define strpbrk_c
#define strrchr_c
#define strspn_c
#define strstr_c
#ifndef DATA_IN_LIBRARY
#define strtok_c
#endif
#define strxfrm_c
#define _strerror_c

#include <string.h>

static char *_hostos_error_string(unsigned int n, char *v)
{   char *s = v;
    int j;
    strcpy(s, "unknown shared string-library error 0x");
    s += strlen(s);
    for (j = 0;  j < 8;  ++ j)
    {   *s++ = "0123456789ABCDEF"[n >> 28];
        n <<= 4;
    }
    *s = 0;
    return v;
}


#ifdef DATA_IN_LIBRARY

static char *saves1 = NULL;

char *strtok(char *s1, const char *s2)
{   char *s0;
    if (s1 == 0) s1 = (saves1 == NULL) ? "" : saves1;  /* use saved pointer */
    if (*(s1 += strspn(s1,s2)) == 0) s0 = 0;             /* no tokens */
    else { s0 = s1;
           if (*(s1 += strcspn(s1,s2)) != 0) *s1++ = 0;  /* insert 0 if nec */
         }
    return (saves1 = s1, s0);
}

#endif
