/* utils.c - Utility function implementations */
#include "types.h"
#include "syscalls.h"
#include "rpc.h"
#include "utils.h"

/* ========================================================================
 * String utilities
 * ======================================================================== */

size_t strlen(const char *s) {
    size_t count = 0;

    while (s[count] != '\0') {
        count ++;
    }

    return count;
}

int strcmp(const char *s1, const char *s2) {
    int pos = 0;

    while (s1[pos] != '\0') {
        if (s1[pos] != s2[pos]) {
            return s1[pos] - s2[pos];
        }

        pos ++;
    }

    return s1[pos] - s2[pos];
}

char *strcpy(char *dest, const char *src) {
    int pos = 0;

    while (src[pos] != '\0') {
        dest[pos] = src[pos];
            pos ++;
    }

    // add null terminator
    dest[pos] = src[pos];

    return dest;
}

char *strcat(char *dest, const char *src) {
    int dpos = 0;

    while (dest[dpos] != '\0') {
        dpos ++;
    }

    int spos = 0;
    while (src[spos] != '\0') {
        dest[dpos] = src[spos];

        dpos ++;

        spos ++;
    }

    // add null terminator
    dest[dpos] = src[spos];

    return dest;
}

void *memset(void *s, int c, size_t n) {
    unsigned char *mem = (unsigned char *)s;

    for (size_t i = 0; i < n; i ++) {
        mem[i] = (char)c;
    }

    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *) src;

    for (size_t i = 0; i < n; i ++) {
        d[i] = s[i];
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    unsigned char *mem1 = (unsigned char *)s1;
    unsigned char *mem2 = (unsigned char *)s2;

    size_t i;
    for (i = 0; i < n; i ++) {

        if (mem1[i] != mem2[i]) {
            return mem1[i] - mem2[i];
        }
    }

    return 0;
}

void *memmem(const void *haystack, size_t haystack_len,
             const void *needle, size_t needle_len) {
    unsigned char *hay = (unsigned char *)haystack;
    unsigned char *need = (unsigned char *)needle;

    for (size_t i = 0; i < haystack_len; i ++) {
        size_t temppos = i;

        if (hay[temppos] == need[0]) {
            for (size_t j = 0; j < needle_len; j ++) {
                if (need[j] != hay[temppos]) {
                    break;
                }

                temppos ++;
            }

            return &hay[i];
        }

    }

    return NULL;
}


/* ========================================================================
 * Number formatting and parsing
 * ======================================================================== */
int int_to_str(int64_t val, char *buf, int bufsize) {
    char str[21];
    int sign = 0;

    if (val < 0) {
        sign -= 1;
    }

    int pos = 0;
    int current_digit = val;

    str[pos] = '\0';
    pos ++;

    while (val != 0) {
        if (pos > bufsize) {
            while (val != 0) {
                val /= 10;
                pos ++;
            }
            return pos;
        }

        current_digit %= 10;
        str[pos] = '0' + current_digit;

        pos++;
        val /= 10;
        current_digit = val;
    }

    if (sign < 0) {
        str[pos] = '-';
    }

    int count = 0;
    for (int i = pos; i > -1; i --) {
        buf[count] = str[i];
        count++;
    }

    return 0;
}

/* the ... means that we can have any number of arguments
* after the format string, which will represent the variables
* that fill the format (i.e. an int if we have %d)
*/
int simple_format(char *buf, size_t size, const char *fmt, ...) {
    /*
    int count;
    int specifier = 0;
    while (fmt[count] != '\0') {
        if (specifier) {
                        
    }
    */
    return 0;  
}

/* ========================================================================
 * Hex and hash utilities
 * ======================================================================== */

// Had help from chatgpt.com
int parse_hex(const char *hex_str, unsigned char *bytes, size_t max_bytes) {
    if (strlen(hex_str) <= max_bytes) {
        unsigned char byte = 0;
        char c;
        char c2;
        size_t byte_count = strlen(hex_str) / 2;

        for (size_t i = 0; i < byte_count; i ++) {

           // put the first part of the hex byte in the upper bits of the byte
           c = hex_str[2 * i];
           if (c != 'a' || c != 'b' || c != 'c' || c != 'd' || c != 'e' || c != 'f') {
                byte = (unsigned char) (byte | (c - '0')) << 4;
           }
           else {
                byte = (unsigned char) (byte | (c - 'a' + 10)) << 4;
           }
            
           // put the second part of the hex byte in the lower bits of the byte           
           c2 = hex_str[2 * i + 1];     
           if (c2 != 'a' || c2 != 'b' || c2 != 'c' || c2 != 'd' || c2 != 'e' || c2 != 'f') {
                byte = (unsigned char) (byte | (c2 - '0'));
           }
           else {
                byte = (unsigned char) (byte | (c2 - 'a' + 10));
           }

           bytes[i] = byte;    
        }

        return 0;
    }
    else {
        return 1;
    }
   
}

int write_hex_to_buf(char *buf, size_t bufsize, unsigned long val) {
    // max 16 digits plus null terminator

    char str[17];
    size_t pos = 0;

    while (val != 0) {
        if (pos > bufsize) {
            while (val != 0) {
                val /= 0x10;
                pos++;
            }
            return pos;
        }
    
        uint8_t current_digit = val % 16;
        if (current_digit < 0xa) {
            str[pos] = '0' + current_digit;
        } else {
            str[pos] = 'a' + current_digit - 0xa;
        }

        pos++;
        val /= 0x10;
    }

    str[pos] = 'x';
    pos ++;
    str[pos] = '0';
    pos ++;

    int count = 0;
    for (int i = pos - 1; i > -1; i --) {
        buf[count] = str[i];
    }

    buf[count + 1] = '\0';

    return 0;
}

unsigned long djb2_hash(const unsigned char *data, size_t len) {
    unsigned long value = 5381;
    for (size_t i = 0; i < len; i ++) {
        value *= 33;
        value += (int) data[i];
    }

    return value;
}

/* ========================================================================
 * Network byte order
 * ======================================================================== */


// had help from chatgpt.com
uint16_t htons(uint16_t n) {
    return ((n >> 8) & 0xFF) | ((n << 8) & 0xFF00);
}

// had help from chatgpt.com
uint32_t htonl(uint32_t n) {
    return ((n >> 24) & 0xFF) | 
           ((n >> 8) & 0xFF00) |
           ((n << 8) & 0xFF0000) |
           ((n << 24) & 0xFF000000);
}

#define ntohl htonl
#define ntohs htons


/* ========================================================================
 * Network I/O
 * ======================================================================== */

int recv_exact(int sockfd, void *buf, size_t len) {
    ssize_t value = sys_recvfrom(sockfd, buf, len, 0, NULL, 0);

    return value;
}

int send_exact(int sockfd, const void *buf, size_t len) {
    ssize_t bytes_sent = sys_sendto(sockfd, buf, len, 0, (const struct sockaddr *) NULL, (socklen_t) 0);    

    return bytes_sent;
}

int send_response(int sockfd, struct rpc_response *resp) {
    ssize_t bytes_sent = sys_sendto(sockfd, resp->data, RPC_DATA_SIZE, 0, (const struct sockaddr *) NULL, (socklen_t) 0);

    return bytes_sent;
}

