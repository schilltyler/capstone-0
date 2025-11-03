/* cmd_timestomp.c - File timestamp modification handler */
#include "cmds.h"
#include "syscalls.h"
#include "types.h"
#include "utils.h"

int handle_timestomp(const char *args, struct rpc_response *resp) {
#ifdef TODO_CMD_timestomp
  /* TODO: Implement timestamp modification */
  resp->status = STATUS_ERROR;
  strcpy(resp->data, "timestomp not yet implemented");
  resp->data_len = strlen(resp->data);
  return -1;
#else
    // Your solution here!

    const char *filename = args[1];

    if (strcmp(args[2], "--at") == 0) {
        long atime = atol(args[3]);

        if (strcmp(args[4], "--mt") == 0) {
            long mtime = atol(args[5]);
            struct timespec times[2];

            times[0].tv_sec = atime;
            times[0].tv_nsec = 0;
            times[1].tv_sec = mtime;
            times[1].tv_nsec = 0;

            int result = sys_utimensat(AT_FDCWD, filename, times, 0);
            if (result == -1) {
                return 1;
            }

            return 0;
        }

        else {
            sys_write(2, usage, strlen(usage));
            return 1;
        }     

    }
    else if (strcmp(args[2], "--copy-from") == 0) {
        const char *other_filename = args[3];
        int fd = sys_openat(AT_FDCWD, other_filename, O_RDONLY, 0);

        if (fd < 0) {
            sys_write(2, "Could not open file\n", strlen("Could not open file\n"));
            return 1;
        }

        struct stat statbuf;

        int success = sys_fstat(fd, &statbuf);
        if (success == -1) {
            sys_write(2, "Error with sys_fstat\n", strlen("Error with sys_fstat\n"));
            return 1;
        }

        struct timespec times[2];
        times[0].tv_sec = statbuf.st_atim.tv_sec;
        times[0].tv_nsec = 0;
        times[1].tv_sec = statbuf.st_mtim.tv_sec;
        times[1].tv_nsec = 0;

        return sys_utimensat(AT_FDCWD, filename, times, 0);
    }

    else {
        return 1;
    }

    return 1;

#endif
}
