/* cmd_wc.c - Word count command handler */
#include "cmds.h"
#include "utils.h"
#include "syscalls.h"
#include "types.h"

int handle_wc(const char *args, struct rpc_response *resp) {
#ifdef TODO_CMD_wc
  /* TODO: Implement word count via mmap */
  resp->status = STATUS_ERROR;
  strcpy(resp->data, "wc not yet implemented");
  resp->data_len = strlen(resp->data);
  return -1;
#else
    
    // Find pathname (arg1)
    // but maybe we are able to just treat the data like argv??
    /*
    char filename[100]; // 100 is arbitrary
    int pos = 0;
    while (args[pos] != '\0') {
        filename[pos] = args[pos];
        pos ++;
    }
    */
    const char *filename;
    int lines_only = 0;
    int bytes_only = 0;

    if (strcmp(args[1], "-l") == 0) {
        lines_only = 1;
        filename = args[2];
    }
    else if (strcmp(args[1], "-c") == 0) {
        bytes_only = 1;
        filename = args[2];
    }
    else {
        filename = args[1];
    }
    
    // Your solution here!
    int fd = sys_openat(AT_FDCWD, filename, O_RDONLY, 0);
    if (fd < 0) {
        //sys_write(2, "Could not open file\n", strlen("Could not open file\n"));
        resp->status = STATUS_ERROR;
        return 0;
    }   
    
    off_t len = sys_lseek(fd, 0, SEEK_END);
    void *addr = sys_mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);

    if ((long) addr < 0) {
        //sys_write(2, "Could not mmap file\n", strlen("Could not mmap file\n"));
        resp->status = STATUS_ERROR;
        return 0;
    }

    unsigned char *buf = (unsigned char *)addr; 
    int newline_count = 0;
    int byte_count = 0;
    int word_count = 0;
    char data[RPC_DATA_SIZE] = {};

    if (lines_only == 0 && bytes_only == 0) {
        if (buf[0] != ' ' || buf[0] != '\n') {
            word_count ++;
        }

        for (int i = 0; i < len; i ++) {
            if (buf[i] == '\n') {
                newline_count ++;
            }

            if (buf[i] == ' ' || buf[i] == '\n') {
                if (len - i == 1) {
                    break;
                }
                if (buf[i + 1] != ' ' && buf[i + 1] != '\n') {
                    word_count ++;
                }
            }

            byte_count ++;
        }

        strcat(data, "Lines: ");
        char buf[30] = {}; // 30 is arbitrary
        int_to_str(newline_count, buf, 30);
        strcat(data, buf);
        strcat(data, "\nWords: ");
        int_to_str(word_count, buf, 30);
        strcat(data, buf);
        strcat(data, "\nBytes: ");
        int_to_str(byte_count, buf, 30);
        strcat(data, buf);
    }
    else if (lines_only == 1) {

        for (int i = 0; i < len; i ++) {
            if (buf[i] == '\n') {
                newline_count ++;
            }
        }

        strcat(data, "Lines: ");
        char buf[30] = {};
        int_to_str(newline_count, buf, 30);
        strcat(data, buf);       
    }
    else if (bytes_only == 1) {

        for (int i = 0; i < len; i ++) {
            byte_count ++;
        }

        strcat(data, "Bytes: ");
        char buf[30] = {};
        int_to_str(byte_count, buf);
        strcat(data, buf);
    }    
    
    resp->status = STATUS_OK;
    resp->data = data;
    resp->reserved = {};
    resp->data_len = strlen(data);                            

    return 0;

#endif
}
