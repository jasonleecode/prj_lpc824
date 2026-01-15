/* Minimal C library implementations for ARM Cortex-M0 bare metal */

#include <stdint.h>
#include <stddef.h>

/* ARM EABI unsigned integer divide and modulo function */
typedef struct {
    uint32_t quotient;
    uint32_t remainder;
} arm_div_result;

arm_div_result __aeabi_uidivmod(uint32_t dividend, uint32_t divisor) {
    arm_div_result result;
    result.quotient = dividend / divisor;
    result.remainder = dividend % divisor;
    return result;
}

/* ARM EABI signed integer divide and modulo function */
typedef struct {
    int32_t quotient;
    int32_t remainder;
} arm_sdiv_result;

arm_sdiv_result __aeabi_idivmod(int32_t dividend, int32_t divisor) {
    arm_sdiv_result result;
    result.quotient = dividend / divisor;
    result.remainder = dividend % divisor;
    return result;
}

/* memcpy implementation */
void *memcpy(void *dest, const void *src, unsigned long n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    
    while (n--) {
        *d++ = *s++;
    }
    
    return dest;
}

/* memmove implementation */
void *memmove(void *dest, const void *src, unsigned long n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    
    return dest;
}

/* memset implementation */
void *memset(void *s, int c, unsigned long n) {
    unsigned char *p = (unsigned char *)s;
    
    while (n--) {
        *p++ = (unsigned char)c;
    }
    
    return s;
}

/* memcmp implementation */
int memcmp(const void *s1, const void *s2, unsigned long n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

/* memchr implementation */
void *memchr(const void *s, int c, unsigned long n) {
    const unsigned char *p = (const unsigned char *)s;
    unsigned char ch = (unsigned char)c;
    
    while (n--) {
        if (*p == ch) {
            return (void *)p;
        }
        p++;
    }
    
    return NULL;
}

/* strlen implementation */
unsigned long strlen(const char *s) {
    unsigned long len = 0;
    
    while (*s++) {
        len++;
    }
    
    return len;
}

/* strcpy implementation */
char *strcpy(char *dest, const char *src) {
    char *d = dest;
    
    while ((*d++ = *src++)) {
        ;
    }
    
    return dest;
}

/* strncpy implementation */
char *strncpy(char *dest, const char *src, unsigned long n) {
    char *d = dest;
    
    while (n-- && (*d++ = *src++)) {
        ;
    }
    
    while (n--) {
        *d++ = '\0';
    }
    
    return dest;
}

/* strcmp implementation */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/* strncmp implementation */
int strncmp(const char *s1, const char *s2, unsigned long n) {
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    
    if (n == 0) {
        return 0;
    }
    
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/* strcat implementation */
char *strcat(char *dest, const char *src) {
    char *d = dest;
    
    while (*d) {
        d++;
    }
    
    while ((*d++ = *src++)) {
        ;
    }
    
    return dest;
}

/* strncat implementation */
char *strncat(char *dest, const char *src, unsigned long n) {
    char *d = dest;
    
    while (*d) {
        d++;
    }
    
    while (n-- && (*d++ = *src++)) {
        ;
    }
    
    *d = '\0';
    
    return dest;
}

/* strchr implementation */
char *strchr(const char *s, int c) {
    unsigned char ch = (unsigned char)c;
    
    while (*s) {
        if ((unsigned char)*s == ch) {
            return (char *)s;
        }
        s++;
    }
    
    if (ch == '\0') {
        return (char *)s;
    }
    
    return NULL;
}

/* strrchr implementation */
char *strrchr(const char *s, int c) {
    unsigned char ch = (unsigned char)c;
    const char *last = NULL;
    
    while (*s) {
        if ((unsigned char)*s == ch) {
            last = s;
        }
        s++;
    }
    
    if (ch == '\0') {
        return (char *)s;
    }
    
    return (char *)last;
}

/* strstr implementation */
char *strstr(const char *haystack, const char *needle) {
    unsigned long needle_len;
    
    if (*needle == '\0') {
        return (char *)haystack;
    }
    
    needle_len = strlen(needle);
    
    while (*haystack) {
        if (strncmp(haystack, needle, needle_len) == 0) {
            return (char *)haystack;
        }
        haystack++;
    }
    
    return NULL;
}

/* Forward declaration of strtok_r */
char *strtok_r(char *s, const char *delim, char **save_ptr);

/* strtok implementation */
static char *strtok_static = NULL;

char *strtok(char *s, const char *delim) {
    return strtok_r(s, delim, &strtok_static);
}

/* strtok_r implementation */
char *strtok_r(char *s, const char *delim, char **save_ptr) {
    char *token;
    
    if (s == NULL) {
        s = *save_ptr;
    }
    
    /* Skip leading delimiters */
    while (*s && strchr(delim, *s) != NULL) {
        s++;
    }
    
    if (*s == '\0') {
        *save_ptr = s;
        return NULL;
    }
    
    token = s;
    
    /* Find end of token */
    while (*s && strchr(delim, *s) == NULL) {
        s++;
    }
    
    if (*s != '\0') {
        *s = '\0';
        s++;
    }
    
    *save_ptr = s;
    
    return token;
}

