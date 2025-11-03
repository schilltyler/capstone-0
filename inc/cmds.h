/* cmds.h - Command handler declarations */
#ifndef CMDS_H
#define CMDS_H

#include "rpc.h"
#include "types.h"

/* Forward declarations */
struct rpc_response;
struct rpc_request;

/* ========================================================================
 * Basic file operations
 * ======================================================================== */
int handle_get_file_stats(const char *path, struct rpc_response *resp);
int handle_ls(const char *path, struct rpc_response *resp);
int handle_pwd(struct rpc_response *resp);

/* ========================================================================
 * File transfer operations
 * ======================================================================== */
int handle_download_file(int sockfd, const char *path);
int handle_upload_file(int sockfd, const char *path);
int handle_append_file(int sockfd, const char *path);

/* ========================================================================
 * Search and analysis operations
 * ======================================================================== */
int handle_bgrep(int sockfd, const char *path, const char *hex_pattern);
int handle_tailf(int sockfd, const char *path);

/* ========================================================================
 * File manipulation utilities
 * ======================================================================== */
int handle_timestomp(const char *args, struct rpc_response *resp);
int handle_wc(const char *args, struct rpc_response *resp);
int handle_djb2sum(const char *path, struct rpc_response *resp);
int handle_sed(int sockfd, const char *args);

/* ========================================================================
 * System information and execution
 * ======================================================================== */
int handle_proc_maps(struct rpc_response *resp);
int handle_runrwx(int sockfd, const char *path);

#endif /* CMDS_H */
