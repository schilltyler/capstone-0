/* syscalls.h - Syscall numbers and wrappers for AArch64 */
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"

/* ========================================================================
 * Syscall wrappers (inline assembly for AArch64)
 * ======================================================================== */

static inline long syscall0(long num) {
  register long x8 __asm__("x8") = num;
  register long x0 __asm__("x0");
  __asm__ volatile("svc #0" : "=r"(x0) : "r"(x8) : "memory");
  return x0;
}

static inline long syscall1(long num, long arg1) {
  register long x8 __asm__("x8") = num;
  register long x0 __asm__("x0") = arg1;
  __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8) : "memory");
  return x0;
}

static inline long syscall2(long num, long arg1, long arg2) {
  register long x8 __asm__("x8") = num;
  register long x0 __asm__("x0") = arg1;
  register long x1 __asm__("x1") = arg2;
  __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1) : "memory");
  return x0;
}

static inline long syscall3(long num, long arg1, long arg2, long arg3) {
  register long x8 __asm__("x8") = num;
  register long x0 __asm__("x0") = arg1;
  register long x1 __asm__("x1") = arg2;
  register long x2 __asm__("x2") = arg3;
  __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
  return x0;
}

static inline long syscall4(long num, long arg1, long arg2, long arg3, long arg4) {
  register long x8 __asm__("x8") = num;
  register long x0 __asm__("x0") = arg1;
  register long x1 __asm__("x1") = arg2;
  register long x2 __asm__("x2") = arg3;
  register long x3 __asm__("x3") = arg4;
  __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3) : "memory");
  return x0;
}

static inline long syscall5(long num, long arg1, long arg2, long arg3, long arg4, long arg5) {
  register long x8 __asm__("x8") = num;
  register long x0 __asm__("x0") = arg1;
  register long x1 __asm__("x1") = arg2;
  register long x2 __asm__("x2") = arg3;
  register long x3 __asm__("x3") = arg4;
  register long x4 __asm__("x4") = arg5;
  __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4) : "memory");
  return x0;
}

static inline long syscall6(long num, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
  register long x8 __asm__("x8") = num;
  register long x0 __asm__("x0") = arg1;
  register long x1 __asm__("x1") = arg2;
  register long x2 __asm__("x2") = arg3;
  register long x3 __asm__("x3") = arg4;
  register long x4 __asm__("x4") = arg5;
  register long x5 __asm__("x5") = arg6;
  __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5) : "memory");
  return x0;
}

/* From /usr/include/asm-generic/unistd.h */
#define SYS_getcwd 17
#define SYS_fcntl 25
#define SYS_inotify_init1 26
#define SYS_inotify_add_watch 27
#define SYS_openat 56
#define SYS_unlinkat 35
#define SYS_close 57
#define SYS_lseek 62
#define SYS_read 63
#define SYS_write 64
#define SYS_readv 65
#define SYS_writev 66
#define SYS_pread64 67
#define SYS_sendfile 71
#define SYS_ppoll 73
#define SYS_newfstatat 79
#define SYS_fstat 80
#define SYS_exit 93
#define SYS_exit_group 94
#define SYS_nanosleep 101
#define SYS_getdents64 61
#define SYS_socket 198
#define SYS_bind 200
#define SYS_listen 201
#define SYS_accept 202
#define SYS_connect 203
#define SYS_sendto 206
#define SYS_recvfrom 207
#define SYS_shutdown 210
#define SYS_setsockopt 208
#define SYS_getsockopt 209
#define SYS_mmap 222
#define SYS_munmap 215
#define SYS_mprotect 226
#define SYS_brk 214
#define SYS_renameat2 276
#define SYS_utimensat 88

/* File flags */
#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_RDWR      00000002
#define O_CREAT     00000100
#define O_TRUNC     00001000
#define O_APPEND    00002000
#define O_NONBLOCK  00004000
#define O_DIRECTORY 00200000
#define O_CLOEXEC   02000000

/* AT flags */
#define AT_FDCWD -100
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR 0x200

/* mmap flags */
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MAP_FAILED ((void *) -1)

/* Socket flags */
#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_NONBLOCK 04000
#define SOL_SOCKET 1
#define SO_REUSEADDR 2
#define MSG_DONTWAIT 0x40

/* Seek flags */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* Poll flags */
#define POLLIN  0x0001
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010

/* inotify flags */
#define IN_MODIFY 0x00000002
#define IN_NONBLOCK 04000

/* Directory entry types */
#define DT_UNKNOWN 0
#define DT_REG     8
#define DT_DIR     4
#define DT_LNK     10

/* Limits */
#define PATH_MAX 4096

/* ========================================================================
 * Syscall wrapper functions
 * ======================================================================== */

static inline int sys_openat(int dirfd, const char *pathname, int flags, mode_t mode) {
  return (int)syscall4(SYS_openat, dirfd, (long)pathname, flags, mode);
}

static inline int sys_close(int fd) {
  return (int)syscall1(SYS_close, fd);
}

static inline ssize_t sys_read(int fd, void *buf, size_t count) {
  return (ssize_t)syscall3(SYS_read, fd, (long)buf, count);
}

static inline ssize_t sys_write(int fd, const void *buf, size_t count) {
  return (ssize_t)syscall3(SYS_write, fd, (long)buf, count);
}

static inline ssize_t sys_pread64(int fd, void *buf, size_t count, off_t offset) {
  return (ssize_t)syscall4(SYS_pread64, fd, (long)buf, count, offset);
}

static inline off_t sys_lseek(int fd, off_t offset, int whence) {
  return (off_t)syscall3(SYS_lseek, fd, offset, whence);
}

static inline int sys_fstat(int fd, struct stat *statbuf) {
  return (int)syscall2(SYS_fstat, fd, (long)statbuf);
}

static inline void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
  return (void *)syscall6(SYS_mmap, (long)addr, length, prot, flags, fd, offset);
}

static inline int sys_munmap(void *addr, size_t length) {
  return (int)syscall2(SYS_munmap, (long)addr, length);
}

static inline int sys_mprotect(void *addr, size_t len, int prot) {
  return (int)syscall3(SYS_mprotect, (long)addr, len, prot);
}

static inline int sys_unlinkat(int dirfd, const char *pathname, int flags) {
  return (int)syscall3(SYS_unlinkat, dirfd, (long)pathname, flags);
}

static inline int sys_renameat2(int olddirfd, const char *oldpath, int newdirfd,
                                const char *newpath, unsigned int flags) {
  return (int)syscall5(SYS_renameat2, olddirfd, (long)oldpath, newdirfd,
                       (long)newpath, flags);
}

static inline void sys_exit(int status) {
  syscall1(SYS_exit, status);
  __builtin_unreachable();
}

static inline long sys_getcwd(char *buf, size_t size) {
  return syscall2(SYS_getcwd, (long)buf, size);
}

static inline int sys_newfstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags) {
  return (int)syscall4(SYS_newfstatat, dirfd, (long)pathname, (long)statbuf, flags);
}

static inline ssize_t sys_getdents64(int fd, void *dirp, size_t count) {
  return (ssize_t)syscall3(SYS_getdents64, fd, (long)dirp, count);
}

static inline int sys_inotify_init1(int flags) {
  return (int)syscall1(SYS_inotify_init1, flags);
}

static inline int sys_inotify_add_watch(int fd, const char *pathname, uint32_t mask) {
  return (int)syscall3(SYS_inotify_add_watch, fd, (long)pathname, mask);
}

static inline int sys_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespecc *timeout, const sigset_tt *sigmask) {
  return (int)syscall4(SYS_ppoll, (long)fds, nfds, (long)timeout, (long)sigmask);
}

static inline ssize_t sys_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
  return (ssize_t)syscall6(SYS_recvfrom, sockfd, (long)buf, len, flags, (long)src_addr, (long)addrlen);
}

static inline int sys_utimensat(int dirfd, const char *pathname, const struct timespecc times[2], int flags) {
  return (int)syscall4(SYS_utimensat, dirfd, (long)pathname, (long)times, flags);
}

static inline int sys_socket(int domain, int type, int protocol) {
    return (int) syscall3(SYS_socket, domain, type, protocol);
}

static inline int sys_connect(int socket, const struct sockaddr *address, socklen_t address_len) {
    return (int) syscall3(SYS_connect, socket, (long) address, address_len);
}

static inline int sys_sendto(int socket, const void *message, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t dest_len) {
    return (int) syscall6(SYS_sendto, socket,(long) message, len, flags, (long)dest_addr, dest_len);
}
#endif /* SYSCALLS_H */
