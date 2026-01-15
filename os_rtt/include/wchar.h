/* Minimal wchar.h for ARM Cortex-M0 cross-compilation */

#ifndef __WCHAR_H
#define __WCHAR_H

#include <stdint.h>
#include <stddef.h>

typedef int wint_t;

extern size_t wcslen(const wchar_t *s);
extern wchar_t *wcscpy(wchar_t *dest, const wchar_t *src);
extern wchar_t *wcsncpy(wchar_t *dest, const wchar_t *src, size_t n);
extern int wcscmp(const wchar_t *s1, const wchar_t *s2);
extern int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n);
extern wchar_t *wcscat(wchar_t *dest, const wchar_t *src);
extern wchar_t *wcsncat(wchar_t *dest, const wchar_t *src, size_t n);
extern wchar_t *wcschr(const wchar_t *s, wchar_t c);
extern wchar_t *wcsrchr(const wchar_t *s, wchar_t c);
extern int mblen(const char *s, size_t n);
extern int mbtowc(wchar_t *pwc, const char *s, size_t n);
extern int wctomb(char *s, wchar_t wc);
extern size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
extern size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);

#endif /* __WCHAR_H */
