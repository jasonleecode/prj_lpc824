/* Minimal types.h for ARM Cortex-M0 cross-compilation */

#ifndef __SYS_TYPES_H
#define __SYS_TYPES_H

#include <stdint.h>

typedef int pid_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef long off_t;
typedef unsigned long size_t;
typedef long ssize_t;
typedef long mode_t;
typedef unsigned long ino_t;
typedef unsigned int nlink_t;
typedef unsigned long dev_t;
typedef unsigned long blksize_t;
typedef unsigned long blkcnt_t;
typedef long suseconds_t;
typedef unsigned int useconds_t;
typedef int32_t time_t;

#endif /* __SYS_TYPES_H */
