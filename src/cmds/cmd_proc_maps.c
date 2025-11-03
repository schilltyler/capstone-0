/* cmd_proc_maps.c - Process memory maps handler */
#include "cmds.h"
#include "utils.h"
#include "syscalls.h"
#include "types.h"

int handle_proc_maps(struct rpc_response *resp) {
#ifdef TODO_CMD_proc_maps
  /* TODO: Implement proc_maps - display process memory map */
  resp->status = STATUS_ERROR;
  strcpy(resp->data, "proc_maps not yet implemented");
  resp->data_len = strlen(resp->data);
  return -1;
#else
    // Your solution here!

    const char *maps_path = "/proc/self/maps";

    int fd = sys_openat(AT_FDCWD, maps_path, O_RDONLY, 0);
    if (fd < 0) {
        resp->status = STATUS_ERROR;
        return 0;
    }

    char buf[CHUNK_SIZE];

    while (sys_read(fd, buf, CHUNK_SIZE) != 0) {
        // need to handle if there is more than buf_size bytes
        strcat(resp->data, buf);
    }
    
    resp->status = STATUS_OK;
    
    return 0;

#endif
}
