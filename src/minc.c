/* minc.c - Minimal C library implementation */

#include "../inc/minc.h"
#include "../inc/syscall_utils.h"


/* =============================================================================

 * STRING FUNCTIONS

 * =============================================================================

 */


// Works

size_t strlen(const char *s) {
    size_t count = 0;

    while (s[count] != '\0') {
        count ++;
    }

    return count;
}


// Works

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


// Works

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


// Works

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


// Works

char *strstr(const char *haystack, const char *needle) {
    if (needle[0] == '\0') {
        return haystack;
    }

    int hpos = 0;
    while (haystack[hpos] != '\0') {
        int npos = 0;
        int temppos = hpos;

        if (haystack[temppos] == needle[npos]) {

            while (needle[npos] != '\0') {

                if (needle[npos] != haystack[temppos]) {

                    break;

                }

                npos ++;
                temppos ++;

            }

            return &haystack[hpos];

        }

	    hpos ++;

    }

    return NULL;
}


// Works

char *strchr(const char *s, int c) {
    int pos = 0;

    while (s[pos] != '\0') {

        if (s[pos] == (char)c) {
            return &s[pos];
        }

        pos ++;

    }

    if ((char)c == '\0') {
        return &s[pos];
    }

    return NULL;
}



/* =============================================================================

 * MEMORY FUNCTIONS

 * =============================================================================

 */


// Works

void *memcpy(void *dest, const void *src, size_t n) {

    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *) src;

    for (size_t i = 0; i < n; i ++) {
        d[i] = s[i];
    }

    return dest;
}


// Works

void *memset(void *s, int c, size_t n) {

    unsigned char *mem = (unsigned char *)s;

    for (size_t i = 0; i < n; i ++) {
        mem[i] = (char)c;
    }

    return s;
}


// Works

int memcmp(const void *s1, const void *s2, size_t n) {
    unsigned char *mem1 = (unsigned char *)s1;
    unsigned char *mem2 = (unsigned char *)s2;

    size_t i = 0;
    for (i; i < n; i ++) {

        if (mem1[i] != mem2[i]) {
            return mem1[i] - mem2[i];
        }
    }

    return 0;
}



/* =============================================================================

 * CONVERSION FUNCTIONS

 * =============================================================================

 */


// Works

int atoi(const char *str) {

    int len = 0;
    while (str[len] != '\0') {
        len ++;
    }

    // see if there is a negative
    int last_pos;

    if (str[0] == '-') {
	    last_pos = 0;
    }
    else {
	    last_pos = -1;
    }

    int value = 0;
    int multiplier = 1;

    for (int pos = len - 1; pos > last_pos; pos --) {

        if (str[pos] == '0') {
            value += 0 * multiplier;
        }
        if (str[pos] == '1') {
            value += 1 * multiplier;
        }
        if (str[pos] == '2') {
            value += 2 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '3') {
            value += 3 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '4') {
            value += 4 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '5') {
            value += 5 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '6') {
            value += 6 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '7') {
            value += 7 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '8') {
            value += 8 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '9') {
            value += 9 * multiplier;
            multiplier * 10;
        }

        multiplier *= 10;
    }

    // account for negative number
    if (last_pos == 0) {
	    value *= -1;
    }

    return value;
}


// Works

long atol(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len ++;
    }

    int last_pos;
    if (str[0] == '-') {
	    last_pos = 0;
    }
    else {
	    last_pos = -1;
    }


    long value = 0;
    int multiplier = 1;

    for (int pos = len - 1; pos > last_pos; pos --) {

        if (str[pos] == '0') {
            value += 0 * multiplier;
        }
        if (str[pos] == '1') {
            value += 1 * multiplier;
        }

        if (str[pos] == '2') {
            value += 2 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '3') {
            value += 3 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '4') {
            value += 4 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '5') {
            value += 5 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '6') {
            value += 6 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '7') {
            value += 7 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '8') {
            value += 8 * multiplier;
            multiplier * 10;
        }
        if (str[pos] == '9') {
            value += 9 * multiplier;
            multiplier * 10;
        }

        multiplier *= 10;

    }

    if (last_pos == 0) {
    	value *= -1;
    }

    return value;
}



/* =============================================================================

 * OUTPUT HELPER FUNCTIONS

 * =============================================================================

 */


// Works

long parse_hex(const char *hex_str) {

    // assume string starts with 0x
    /*
    int len = 2;

    while (hex_str[len] != '\0') {
        len ++;
    }
    */
    // assume string does not start with 0x

    int len = 0;

    while (hex_str[len] != '\0') {
        len ++;
    }

    long value = 0;
    int multiplier = 1;

    for (int i = len - 1; i > 1; i --) {

        if (hex_str[i] == '0') {
            value += 0 * multiplier;
        }
        if (hex_str[i] == '1') {
            value += 1 * multiplier;
        }
        if (hex_str[i] == '2') {
            value += 2 * multiplier;
        }
        if (hex_str[i] == '3') {
            value += 3 * multiplier;
        }
        if (hex_str[i] == '4') {
            value += 4 * multiplier;
        }
        if (hex_str[i] == '5') {
            value += 5 * multiplier;
        }
        if (hex_str[i] == '6') {
            value += 6 * multiplier;
        }
        if (hex_str[i] == '7') {
            value += 7 * multiplier;
        }
        if (hex_str[i] == '8') {
            value += 8 * multiplier;
        }
        if (hex_str[i] == '9') {
            value += 9 * multiplier;
        }
        if (hex_str[i] == 'a') {
            value += 10 * multiplier;
        }
        if (hex_str[i] == 'b') {
            value += 11 * multiplier;
        }
        if (hex_str[i] == 'c') {
            value += 12 * multiplier;
        }
        if (hex_str[i] == 'd') {
            value += 13 * multiplier;
        }
        if (hex_str[i] == 'e') {
            value += 14 * multiplier;
        }
        if (hex_str[i] == 'f') {
            value += 15 * multiplier;
        }

        multiplier *= 16;
    }

    return value;
}


// Works

// debug: write the str representation of an integer

void write_num(int fd, long num) {

    // max 19 digits plus null terminator and negative sign
    char str[21];
    int sign = 0;

    if (num < 0) {
        sign -= 1;
    }

    int pos = 0;
    int new_num = num;

    str[pos] = '\0';
    pos ++;

    while (num != 0) {

        new_num %= 10;

	    if (new_num == 0) {
             str[pos] = '0';

	    }
	    if (new_num == 1) {
            str[pos] = '1';
        }
	    if (new_num == 2) {
            str[pos] = '2';
        }
	    if (new_num == 3) {
            str[pos] = '3';
        }
	    if (new_num == 4) {
            str[pos] = '4';
        }
	    if (new_num == 5) {
            str[pos] = '5';
        }
	    if (new_num == 6) {
            str[pos] = '6';
        }
	    if (new_num == 7) {
            str[pos] = '7';
        }
	    if (new_num == 8) {
            str[pos] = '8';
        }
	    if (new_num == 9) {
            str[pos] = '9';
        }

        pos ++;
        num /= 10;
        new_num = num;
    }

    if (sign < 0) {
        str[pos] = '-';
    }

    for (int i = pos; i > -1; i --) {
        sys_write(fd, &str[i], 1);
    }

}


// Works

// debug write the hex value of an integer

void write_hex(int fd, unsigned long num) {

    // max 16 digits plus null terminator
    char str[17];
    int pos = 0;
    int new_num = num;

    while (num != 0) {

        new_num %= 16;

        if (new_num == 0) {
            str[pos] = '0';
        }
        else if (new_num == 1) {
            str[pos] = '1';
        }
        else if (new_num == 2) {
            str[pos] = '2';
        }
        else if (new_num == 3) {
            str[pos] = '3';
        }
        else if (new_num == 4) {
            str[pos] = '4';
        }
        else if (new_num == 5) {
            str[pos] = '5';
        }
        else if (new_num == 6) {
            str[pos] = '6';
        }
        else if (new_num == 7) {
            str[pos] = '7';
        }
        else if (new_num == 8) {
            str[pos] = '8';
        }
        else if (new_num == 9) {
            str[pos] = '9';
        }
        else if (new_num == 10) {
            str[pos] = 'a';
        }
        else if (new_num == 11) {
            str[pos] = 'b';
        }
        else if (new_num == 12) {
            str[pos] = 'c';
        }
        else if (new_num == 13) {
            str[pos] = 'd';
        }
        else if (new_num == 14) {
            str[pos] = 'e';
        }
        else if (new_num == 15) {
            str[pos] = 'f';
        }

        pos ++;
        num /= 16;
        new_num = num;
    }


    str[pos] = 'x';
    pos ++;
    str[pos] = '0';
    pos ++;

    for (int i = pos - 1; i > -1; i --) {
        sys_write(fd, &str[i], 1);
    }

    str[pos] = '\0';
    sys_write(fd, &str[pos], 1);

}

