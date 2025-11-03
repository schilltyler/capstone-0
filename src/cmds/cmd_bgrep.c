/* cmd_bgrep.c - Binary grep command handler */
#include "cmds.h"
#include "syscalls.h"
#include "types.h"
#include "utils.h"

#define CHUNK_SIZE 4096
#define MAX_PATTERN_SIZE 256

static struct rpc_response g_resp;
int found_any;

void found_offset(int i) {
    //strcat(
    g_resp.status = STATUS_MORE_DATA;

    sys_write(1, "Found at offset: ", 18);
    //write_num(1, i);
    sys_write(1, "(", 1);
    //write_hex(1, i);
    sys_write(1, ")", 1);
    found_any ++;
}


int handle_bgrep(int sockfd, const char *path, const char *hex_pattern) {
#ifdef TODO_CMD_bgrep
  /* TODO: Implement binary grep with hex pattern search */
  struct rpc_response resp;
  memset(&resp, 0, sizeof(resp));
  resp.status = STATUS_ERROR;
  strcpy(resp.data, "bgrep not yet implemented");
  resp.data_len = strlen(resp.data);
  send_response(sockfd, &resp);
  return -1;
#else
    // Your solution here!

    const char *filename = path;
    //const char *hex_pattern = hex_pattern;

    int pattern_size = strlen(hex_pattern);
    if (pattern_size <= 0 || pattern_size > MAX_PATTERN_SIZE) {
        sys_write(2, "Pattern is too long\n", 21);
    }

    int fd = sys_openat(AT_FDCWD, filename, O_RDONLY, 0);
    if (fd < 0) {
        sys_write(2, "Could not open file\n", 21);
    }

    char buf[CHUNK_SIZE];
    off_t offset = 0;

    ssize_t bytes_read = sys_pread64(fd, buf, CHUNK_SIZE, offset);

    sys_write(1, buf, strlen(buf));
    offset += 4096;
    int pos = -1;

    while (bytes_read != 0) {
        //sys_write(1, "Entering while loop\n", strlen("Entering while loop\n"));

        if (pos != -1) {
            if (memcmp(buf, &hex_pattern[pattern_size - pos], pos) == 0) {
                found_offset(offset - (pattern_size - pos));
            }
        }


	    for (int i = 0; i <= bytes_read - pattern_size; i ++) {
            //sys_write(1, "Entering first for loop\n", strlen("Entering first for loop\n"));

		    if (memcmp(&buf[i], hex_pattern, pattern_size) == 0) {
                //sys_write(1, "Found", strlen("Found"));
			    found_offset(i);
		    }
  	    }


        for (int i = pattern_size; i > 0; i --) {
            //sys_write(1, "In this for loop\n", strlen("In this for loop\n"));

            if (strcmp(&buf[bytes_read - i], &hex_pattern[pattern_size - i]) == 0) {
                pos = i;
                break;
            }
        }

        //sys_write(1, "Reading bytes again\n", strlen("Reading bytes again\n"));
        bytes_read = sys_pread64(fd, buf, CHUNK_SIZE, offset);
        offset += 4096;

    }


    //return found_any ? 0 : 1;
    

    return 0;
#endif
}
