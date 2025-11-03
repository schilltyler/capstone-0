/* cmd_tailf.c - Tail -f command handler */
#include "cmds.h"
#include "utils.h"
#include "syscalls.h"
#include "types.h"

int handle_tailf(int sockfd, const char *path) {
#ifdef TODO_CMD_tailf
  /* TODO: Implement tailf - tail file continuously with inotify */
  struct rpc_response resp;
  memset(&resp, 0, sizeof(resp));
  resp.status = STATUS_ERROR;
  strcpy(resp.data, "tailf not yet implemented");
  resp.data_len = strlen(resp.data);
  send_response(sockfd, &resp);
  return -1;
#else
    // Your solution here!
#endif
}
