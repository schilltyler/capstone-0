/* cmd_pwd.c - Print working directory command handler */
#include "cmds.h"
#include "utils.h"
#include "syscalls.h"
#include "types.h"

int handle_pwd(struct rpc_response *resp) {
#ifdef TODO_CMD_pwd
  /* TODO: Implement pwd - print working directory using getcwd syscall */
  resp->status = STATUS_ERROR;
  strcpy(resp->data, "pwd not yet implemented");
  resp->data_len = strlen(resp->data);
  return -1;
#else
    // Your solution here!
    return 0;
#endif
}
