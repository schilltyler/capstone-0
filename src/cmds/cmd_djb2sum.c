/* cmd_djb2sum.c - DJB2 hash calculation handler */
#include "cmds.h"
#include "syscalls.h"
#include "types.h"
#include "utils.h"

int handle_djb2sum(const char *path, struct rpc_response *resp) {
#ifdef TODO_CMD_djb2sum
  /* TODO: Implement djb2 hash calculation - efficiently hash file using mmap */
  resp->status = STATUS_ERROR;
  strcpy(resp->data, "djb2sum not yet implemented");
  resp->data_len = strlen(resp->data);
  return -1;
#else
    // Your solution here!
    // do we just use djb2_hash funcion from utils.h??

    const char *filename = path;

    int fd = sys_openat(AT_FDCWD, filename, O_RDONLY, 0);
    if (fd < 0) {
        resp->status = STATUS_ERROR;
        return 1;
    }

    off_t len = sys_lseek(fd, 0, SEEK_END);

    void *addr = sys_mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if ((long) addr < 0) {
        resp->status = STATUS_ERROR;
        return 1;
    }

    unsigned char *buf = (unsigned char *)addr;
    unsigned long value = 5381;

    for (int i = 0; i < len; i ++) {
        value *= 33;
        value += (int) buf[i];
    }
   
    char str[30] = {}; // 30 is arbitrary
    write_hex_to_buf(str, 30, value);
    
    for (int i = 0; i < strlen(str); i ++) {
        resp->data[i] = str[i];
    }
    
    resp->status = STATUS_OK;

    return 0;

#endif
}
