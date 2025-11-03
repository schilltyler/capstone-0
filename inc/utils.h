/* utils.h - Utility function declarations */
#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/* ========================================================================
 * String utilities
 * ======================================================================== */
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmem(const void *haystack, size_t haystack_len,
             const void *needle, size_t needle_len);

/* ========================================================================
 * Number formatting and parsing
 * ======================================================================== */
int int_to_str(int64_t val, char *buf, int bufsize);
int simple_format(char *buf, size_t size, const char *fmt, ...);

/* ========================================================================
 * Hex and hash utilities
 * ======================================================================== */
int parse_hex(const char *hex_str, unsigned char *bytes, size_t max_bytes);
int write_hex_to_buf(char *buf, size_t bufsize, unsigned long val);
unsigned long djb2_hash(const unsigned char *data, size_t len);

/* ========================================================================
 * Network byte order
 * ======================================================================== */
uint16_t htons(uint16_t n);
uint32_t htonl(uint32_t n);
#define ntohl htonl
#define ntohs htons

/* ========================================================================
 * Network I/O
 * ======================================================================== */
int recv_exact(int sockfd, void *buf, size_t len);
int send_exact(int sockfd, const void *buf, size_t len);
int send_response(int sockfd, struct rpc_response *resp);

#endif /* UTILS_H */
