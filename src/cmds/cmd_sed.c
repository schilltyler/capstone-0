/* cmd_sed.c - Sed-lite in-place text replacement handler (counts replacements) */
#include "cmds.h"
#include "utils.h"
#include "syscalls.h"
#include "types.h"

#define WRITE_BUFFER_SIZE 4096

static int flush_buffer(int fd, char *buffer, size_t *pos, size_t *total) {
  if (*pos == 0)
    return 0;

  long wrote = sys_write(fd, buffer, *pos);
  if (wrote != (long)*pos)
    return -1;

  *total += *pos;
  *pos = 0;
  return 0;
}

int handle_sed(int sockfd, const char *args) {
#ifdef TODO_CMD_sed
  /* TODO: Implement sed - find and replace in file */
  struct rpc_response resp;
  memset(&resp, 0, sizeof(resp));
  resp.status = STATUS_ERROR;
  strcpy(resp.data, "sed not yet implemented");
  resp.data_len = strlen(resp.data);
  send_response(sockfd, &resp);
  return -1;
#else
    // Your solution here!
    return 0;
#endif
}
