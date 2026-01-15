/* Minimal ctype.h for ARM Cortex-M0 cross-compilation */

#ifndef __CTYPE_H
#define __CTYPE_H

#define _UPPER      0x01
#define _LOWER      0x02
#define _DIGIT      0x04
#define _SPACE      0x08
#define _PUNCT      0x10
#define _CNTRL      0x20
#define _BLANK      0x40
#define _HEX        0x80

extern unsigned char _ctype[];

#define isalnum(c)  ((_ctype + 1)[(unsigned char)(c)] & (_UPPER | _LOWER | _DIGIT))
#define isalpha(c)  ((_ctype + 1)[(unsigned char)(c)] & (_UPPER | _LOWER))
#define iscntrl(c)  ((_ctype + 1)[(unsigned char)(c)] & _CNTRL)
#define isdigit(c)  ((_ctype + 1)[(unsigned char)(c)] & _DIGIT)
#define isgraph(c)  ((_ctype + 1)[(unsigned char)(c)] & (_PUNCT | _UPPER | _LOWER | _DIGIT))
#define islower(c)  ((_ctype + 1)[(unsigned char)(c)] & _LOWER)
#define isprint(c)  ((_ctype + 1)[(unsigned char)(c)] & (_BLANK | _PUNCT | _UPPER | _LOWER | _DIGIT))
#define ispunct(c)  ((_ctype + 1)[(unsigned char)(c)] & _PUNCT)
#define isspace(c)  ((_ctype + 1)[(unsigned char)(c)] & _SPACE)
#define isupper(c)  ((_ctype + 1)[(unsigned char)(c)] & _UPPER)
#define isxdigit(c) ((_ctype + 1)[(unsigned char)(c)] & _HEX)
#define isblank(c)  ((_ctype + 1)[(unsigned char)(c)] & _BLANK)

extern int tolower(int);
extern int toupper(int);

#endif /* __CTYPE_H */
