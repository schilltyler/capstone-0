/* cmd_download.c - File download command handler */
#include "cmds.h"
#include "utils.h"
#include "syscalls.h"
#include "types.h"

int handle_download_file(int sockfd, const char *path) {
#ifdef TODO_CMD_download
  /* TODO: Implement download - send file to server using multi-chunk protocol */
  struct rpc_response resp;
  memset(&resp, 0, sizeof(resp));
  resp.status = STATUS_ERROR;
  strcpy(resp.data, "download not yet implemented");
  resp.data_len = strlen(resp.data);
  send_response(sockfd, &resp);
  return -1;
#else
    // Your solution here!
    
    struct rpc_response resp;
    
    int fd = sys_openat(AT_FDCWD, path, O_RDONLY, 0);
    if (fd < 0) {
        resp.status = STATUS_ERROR;
        return 0;
    }
    
    char buf[4088];
    ssize_t bytes_read = sys_read(fd, buf, sizeof(buf));

    while (bytes_read == 4088) {
        memset(&resp, 0, sizeof(resp));
        resp.status = STATUS_MORE_DATA;
        resp.data_len = (uint32_t)bytes_read;
        memcpy(resp.data, buf, 4088);
        send_response(sockfd, &resp);
        bytes_read = sys_read(fd, buf, sizeof(buf));
        sys_write(1, "In while loop\n", strlen("In while loop\n"));
    }

    memset(&resp, 0, sizeof(resp));
    resp.status = STATUS_OK;
    resp.data_len = (uint32_t)bytes_read;
    memcpy(resp.data, buf, (size_t)bytes_read);

    send_response(sockfd, &resp);
    sys_write(1, "Out of while loop\n", strlen("Out of while loop\n"));

    return 0;
#endif
}
