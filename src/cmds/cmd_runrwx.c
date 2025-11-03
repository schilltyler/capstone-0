/* cmd_runrwx.c - Execute raw machine code handler */
#include "cmds.h"
#include "syscalls.h"
#include "types.h"
#include "utils.h"

int handle_runrwx(int sockfd, const char *path) {
#ifdef TODO_CMD_runrwx
  /* TODO: Implement runrwx - execute raw machine code */
  struct rpc_response resp;
  memset(&resp, 0, sizeof(resp));
  resp.status = STATUS_ERROR;
  strcpy(resp.data, "runrwx not yet implemented");
  resp.data_len = strlen(resp.data);
  send_response(sockfd, &resp);
  return -1;
#else
    // Your solution here!

    const char *codefile = argv[1];

    int fd = sys_openat(AT_FDCWD, codefile, O_RDONLY, 0);
    if (fd < 0) {
        sys_write(2, "Could not open file\n", strlen("Could not open file\n"));
        return 1;
    }

    off_t len = sys_lseek(fd, 0, SEEK_END);

    void *addr = sys_mmap(NULL, len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);
    if ((long)addr < 0) {
        sys_write(2, "Could not mmap\n", strlen("Could not mmap\n"));
        return 1;
    }

    int (*func)(int, char **) = (int (*)(int, char **))addr;

    int result = func(argc, argv);

    return result;

    
#endif
}
