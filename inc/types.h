/* types.h - Basic type definitions (no libc) */
#ifndef TYPES_H
#define TYPES_H

/* Basic types */
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long      uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long        int64_t;

typedef unsigned long      size_t;
typedef long               ssize_t;
typedef long               off_t;
typedef unsigned int       mode_t;

/* NULL pointer */
#define NULL ((void *)0)

/* Boolean */
typedef int bool;
#define true 1
#define false 0

/* stat structure (simplified) */
struct stat {
    uint64_t st_dev;
    uint64_t st_ino;
    uint32_t st_mode;
    uint32_t st_nlink;
    uint32_t st_uid;
    uint32_t st_gid;
    uint64_t st_rdev;
    uint64_t __pad1;
    int64_t  st_size;
    int32_t  st_blksize;
    int32_t  __pad2;
    int64_t  st_blocks;
    int64_t  st_atime;
    uint64_t st_atime_nsec;
    int64_t  st_mtime;
    uint64_t st_mtime_nsec;
    int64_t  st_ctime;
    uint64_t st_ctime_nsec;
    uint32_t __unused4;
    uint32_t __unused5;
};

/* linux_dirent64 structure */
struct linux_dirent64 {
    uint64_t       d_ino;
    int64_t        d_off;
    uint16_t       d_reclen;
    uint8_t        d_type;
    char           d_name[];
};

/* sockaddr_in structure */
struct sockaddr_in {
    uint16_t sin_family;
    uint16_t sin_port;
    uint32_t sin_addr;
    uint8_t  sin_zero[8];
};

/* sockaddr generic */
struct sockaddr {
    uint16_t sa_family;
    char     sa_data[14];
};


/* pollfd structure */
struct pollfd {
    int   fd;
    short events;
    short revents;
};

/* timespec structure */
struct timespecc {
    int64_t tv_sec;
    int64_t tv_nsec;
};

/* inotify_event structure */
struct inotify_event {
    int32_t  wd;
    uint32_t mask;
    uint32_t cookie;
    uint32_t len;
    char     name[];
};

/* Additional typedefs for syscalls */
typedef unsigned long nfds_t;
typedef unsigned int socklen_t;
typedef struct {
    unsigned long __bits[16];
} sigset_tt;

#endif /* TYPES_H */
