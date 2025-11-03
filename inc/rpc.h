/* rpc.h - RPC protocol definitions */
#ifndef RPC_H
#define RPC_H

#include "types.h"

/* RPC Commands */
#define CMD_GET_FILE_STATS  0x01
#define CMD_LS              0x02
#define CMD_PWD             0x03
#define CMD_DOWNLOAD_FILE   0x04
#define CMD_BGREP           0x05
#define CMD_TAILF           0x06
#define CMD_CANCEL          0x07
#define CMD_EXIT            0x08
#define CMD_UPLOAD_FILE     0x09
#define CMD_APPEND_FILE     0x0A
#define CMD_TIMESTOMP       0x0B
#define CMD_WC              0x0C
#define CMD_DJB2SUM         0x0D
#define CMD_SED             0x0E
#define CMD_RUNRWX          0x0F
#define CMD_PROC_MAPS       0x10

/* RPC Status codes */
#define STATUS_OK           0x00
#define STATUS_ERROR        0x01
#define STATUS_MORE_DATA    0x02

/* RPC message sizes */
#define RPC_DATA_SIZE       4088
#define RPC_REQUEST_SIZE    4096
#define RPC_RESPONSE_SIZE   4096

/* RPC Request structure */
struct rpc_request {
    uint8_t  cmd_type;
    uint8_t  reserved[3];
    uint32_t data_len;
    char     data[RPC_DATA_SIZE];
} __attribute__((packed));

/* RPC Response structure */
struct rpc_response {
    uint8_t  status;
    uint8_t  reserved[3];
    uint32_t data_len;
    char     data[RPC_DATA_SIZE];
} __attribute__((packed));

#endif /* RPC_H */
