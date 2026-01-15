/* Minimal stddef.h for ARM Cortex-M0 cross-compilation */

#ifndef __STDDEF_H
#define __STDDEF_H

#include <stdint.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned long size_t;
typedef long ptrdiff_t;
typedef int wchar_t;

#endif /* __STDDEF_H */
