/* Minimal string.h for ARM Cortex-M0 cross-compilation */

#ifndef __STRING_H
#define __STRING_H

#include <stdint.h>

extern void *memset(void *s, int c, unsigned long n);
extern void *memcpy(void *dest, const void *src, unsigned long n);
extern int memcmp(const void *s1, const void *s2, unsigned long n);
extern void *memmove(void *dest, const void *src, unsigned long n);
extern void *memchr(const void *s, int c, unsigned long n);
extern unsigned long strlen(const char *s);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, unsigned long n);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, unsigned long n);
extern char *strcat(char *dest, const char *src);
extern char *strncat(char *dest, const char *src, unsigned long n);
extern char *strchr(const char *s, int c);
extern char *strrchr(const char *s, int c);
extern char *strstr(const char *haystack, const char *needle);
extern char *strtok(char *s, const char *delim);
extern char *strtok_r(char *s, const char *delim, char **save_ptr);

#endif /* __STRING_H */
