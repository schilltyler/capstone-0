/* cmd_append.c - File append command handler */
#include "cmds.h"
#include "syscalls.h"
#include "types.h"
#include "utils.h"

int handle_append_file(int sockfd, const char *path) {
#ifdef TODO_CMD_append
  /* TODO: Implement append - append data to file using multi-chunk protocol */
  struct rpc_response resp;
  memset(&resp, 0, sizeof(resp));
  resp.status = STATUS_ERROR;
  strcpy(resp.data, "append not yet implemented");
  resp.data_len = strlen(resp.data);
  send_response(sockfd, &resp);
  return -1;
#else
    // Your solution here!
    return 0;
#endif
}
