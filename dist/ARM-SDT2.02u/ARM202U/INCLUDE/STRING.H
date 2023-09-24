#pragma force_top_level
#pragma include_only_once

/* string.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.11 */
/* Copyright (C) Codemist Ltd., 1988-1993.                        */
/* Copyright (C) Advanced Risc Machines Ltd., 1991-1993.          */
/* version 0.03 */

/*
 * string.h declares one type and several functions, and defines one macro
 * useful for manipulating character arrays and other objects treated as
 * character arrays. Various methods are used for determining the lengths of
 * the arrays, but in all cases a char * or void * argument points to the
 * initial (lowest addresses) character of the array. If an array is written
 * beyond the end of an object, the behaviour is undefined.
 */

#ifndef __string_h
#define __string_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __size_t
#define __size_t 1
typedef unsigned int size_t;   /* from <stddef.h> */
#endif

#ifndef NULL
#  define NULL 0
#endif

extern void *memcpy(void * /*s1*/, const void * /*s2*/, size_t /*n*/);
   /*
    * copies n characters from the object pointed to by s2 into the object
    * pointed to by s1. If copying takes place between objects that overlap,
    * the behaviour is undefined.
    * Returns: the value of s1.
    */
extern void *memmove(void * /*s1*/, const void * /*s2*/, size_t /*n*/);
   /*
    * copies n characters from the object pointed to by s2 into the object
    * pointed to by s1. Copying takes place as if the n characters from the
    * object pointed to by s2 are first copied into a temporary array of n
    * characters that does not overlap the objects pointed to by s1 and s2,
    * and then the n characters from the temporary array are copied into the
    * object pointed to by s1.
    * Returns: the value of s1.
    */
extern char *strcpy(char * /*s1*/, const char * /*s2*/);
   /*
    * copies the string pointed to by s2 (including the terminating nul
    * character) into the array pointed to by s1. If copying takes place
    * between objects that overlap, the behaviour is undefined.
    * Returns: the value of s1.
    */
extern char *strncpy(char * /*s1*/, const char * /*s2*/, size_t /*n*/);
   /*
    * copies not more than n characters (characters that follow a null
    * character are not copied) from the array pointed to by s2 into the array
    * pointed to by s1. If copying takes place between objects that overlap,
    * the behaviour is undefined.
    * Returns: the value of s1.
    */

extern char *strcat(char * /*s1*/, const char * /*s2*/);
   /*
    * appends a copy of the string pointed to by s2 (including the terminating
    * null character) to the end of the string pointed to by s1. The initial
    * character of s2 overwrites the null character at the end of s1.
    * Returns: the value of s1.
    */
extern char *strncat(char * /*s1*/, const char * /*s2*/, size_t /*n*/);
   /*
    * appends not more than n characters (a null character and characters that
    * follow it are not appended) from the array pointed to by s2 to the end of
    * the string pointed to by s1. The initial character of s2 overwrites the
    * null character at the end of s1. A terminating null character is always
    * appended to the result.
    * Returns: the value of s1.
    */

/*
 * The sign of a nonzero value returned by the comparison functions is
 * determined by the sign of the difference between the values of the first
 * pair of characters (both interpreted as unsigned char) that differ in the
 * objects being compared.
 */

extern int memcmp(const void * /*s1*/, const void * /*s2*/, size_t /*n*/);
   /*
    * compares the first n characters of the object pointed to by s1 to the
    * first n characters of the object pointed to by s2.
    * Returns: an integer greater than, equal to, or less than zero, according
    *          as the object pointed to by s1 is greater than, equal to, or
    *          less than the object pointed to by s2.
    */
extern int strcmp(const char * /*s1*/, const char * /*s2*/);
   /*
    * compares the string pointed to by s1 to the string pointed to by s2.
    * Returns: an integer greater than, equal to, or less than zero, according
    *          as the string pointed to by s1 is greater than, equal to, or
    *          less than the string pointed to by s2.
    */
extern int strncmp(const char * /*s1*/, const char * /*s2*/, size_t /*n*/);
   /*
    * compares not more than n characters (characters that follow a null
    * character are not compared) from the array pointed to by s1 to the array
    * pointed to by s2.
    * Returns: an integer greater than, equal to, or less than zero, according
    *          as the string pointed to by s1 is greater than, equal to, or
    *          less than the string pointed to by s2.
    */
extern int strcoll(const char * /*s1*/, const char * /*s2*/);
   /*
    * compares the string pointed to by s1 to the string pointed to by s2, both
    * interpreted as appropriate to the LC_COLLATE category of the current
    * locale.
    * Returns: an integer greater than, equal to, or less than zero, according
    *          as the string pointed to by s1 is greater than, equal to, or
    *          less than the string pointed to by s2 when both are interpreted
    *          as appropriate to the current locale.
    */

extern size_t strxfrm(char * /*s1*/, const char * /*s2*/, size_t /*n*/);
   /*
    * transforms the string pointed to by s2 and places the resulting string
    * into the array pointed to by s1. The transformation function is such that
    * if the strcmp function is applied to two transformed strings, it returns
    * a value greater than, equal to or less than zero, corresponding to the
    * result of the strcoll function applied to the same two original strings.
    * No more than n characters are placed into the resulting array pointed to
    * by s1, including the terminating null character. If n is zero, s1 is
    * permitted to be a null pointer. If copying takes place between objects
    * that overlap, the behaviour is undefined.
    * Returns: The length of the transformed string is returned (not including
    *          the terminating null character). If the value returned is n or
    *          more, the contents of the array pointed to by s1 are
    *          indeterminate.
    */


extern void *memchr(const void * /*s*/, int /*c*/, size_t /*n*/);
   /*
    * locates the first occurence of c (converted to an unsigned char) in the
    * initial n characters (each interpreted as unsigned char) of the object
    * pointed to by s.
    * Returns: a pointer to the located character, or a null pointer if the
    *          character does not occur in the object.
    */
extern char *strchr(const char * /*s*/, int /*c*/);
   /*
    * locates the first occurence of c (converted to an char) in the string
    * pointed to by s (including the terminating null character).
    * Returns: a pointer to the located character, or a null pointer if the
    *          character does not occur in the string.
    */
extern size_t strcspn(const char * /*s1*/, const char * /*s2*/);
   /*
    * computes the length of the initial segment of the string pointed to by s1
    * which consists entirely of characters not from the string pointed to by
    * s2. The terminating null character is not considered part of s2.
    * Returns: the length of the segment.
    */
extern char *strpbrk(const char * /*s1*/, const char * /*s2*/);
   /*
    * locates the first occurence in the string pointed to by s1 of any
    * character from the string pointed to by s2.
    * Returns: returns a pointer to the character, or a null pointer if no
    *          character form s2 occurs in s1.
    */
extern char *strrchr(const char * /*s*/, int /*c*/);
   /*
    * locates the last occurence of c (converted to a char) in the string
    * pointed to by s. The terminating null character is considered part of
    * the string.
    * Returns: returns a pointer to the character, or a null pointer if c does
    *          not occur in the string.
    */
extern size_t strspn(const char * /*s1*/, const char * /*s2*/);
   /*
    * computes the length of the initial segment of the string pointed to by s1
    * which consists entirely of characters from the string pointed to by S2
    * Returns: the length of the segment.
    */
extern char *strstr(const char * /*s1*/, const char * /*s2*/);
   /*
    * locates the first occurence in the string pointed to by s1 of the
    * sequence of characters (excluding the terminating null character) in the
    * string pointed to by s2.
    * Returns: a pointer to the located string, or a null pointer if the string
    *          is not found.
    */
extern char *strtok(char * /*s1*/, const char * /*s2*/);
   /*
    * A sequence of calls to the strtok function breaks the string pointed to
    * by s1 into a sequence of tokens, each of which is delimited by a
    * character from the string pointed to by s2. The first call in the
    * sequence has s1 as its first argument, and is followed by calls with a
    * null pointer as their first argument. The separator string pointed to by
    * s2 may be different from call to call.
    * The first call in the sequence searches for the first character that is
    * not contained in the current separator string s2. If no such character
    * is found, then there are no tokens in s1 and the strtok function returns
    * a null pointer. If such a character is found, it is the start of the
    * first token.
    * The strtok function then searches from there for a character that is
    * contained in the current separator string. If no such character is found,
    * the current token extends to the end of the string pointed to by s1, and
    * subsequent searches for a token will fail. If such a character is found,
    * it is overwritten by a null character, which terminates the current
    * token. The strtok function saves a pointer to the following character,
    * from which the next search for a token will start.
    * Each subsequent call, with a null pointer as the value for the first
    * argument, starts searching from the saved pointer and behaves as
    * described above.
    * Returns: pointer to the first character of a token, or a null pointer if
    *          there is no token.
    */

extern void *memset(void * /*s*/, int /*c*/, size_t /*n*/);
   /*
    * copies the value of c (converted to an unsigned char) into each of the
    * first n charactes of the object pointed to by s.
    * Returns: the value of s.
    */
extern char *strerror(int /*errnum*/);
   /*
    * maps the error number in errnum to an error message string.
    * Returns: a pointer to the string, the contents of which are
    *          implementation-defined (under RISCOS/Arthur/Brazil the strings
    *          for the given ernums are as follows
    *          0       "No error (errno = 0)"
    *          EDOM    "EDOM - function argument out of range"
    *          ERANGE  "ERANGE - function result not representable"
    *          ESIGNUM "ESIGNUM - illegal signal number to signal() or raise()"
    *          others  "Error code (errno) %d has no associated message"
    *          ). The array pointed to shall not be
    *          modified by the program, but may be overwritten by a subsequent
    *          call to the strerror function.
    */
extern size_t strlen(const char * /*s*/);
   /*
    * computes the length of the string pointed to by s.
    * Returns: the number of characters that precede the terminating null
    *          character.
    */

#ifdef __cplusplus
}
#endif

#endif

/* end of string.h */
