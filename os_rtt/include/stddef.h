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

/* wint_t: needed by newlib's sys/_types.h via __need_wint_t + #include <stddef.h>.
 * This header shadows GCC's stddef.h, so we must provide wint_t ourselves. */
#ifndef _WINT_T
#define _WINT_T
typedef int wint_t;
#endif

#endif /* __STDDEF_H */
