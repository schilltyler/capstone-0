```
═══════════════════════════════════════════════════════════════════════════
                         FUNCLASSIFIED MATERIAL
                      TOP SECRET//NOBUTUS//CHINCHILLA
═══════════════════════════════════════════════════════════════════════════
```

# TECHNICAL SPECIFICATION DOCUMENT

**Program:** FLAMBOYANT CHINCHILLA
**Classification:** TOP SECRET//NOBUTUS/NOFUN
**Issuing Authority:** Network Observability Command (NOC)
**Document Version:** 1.0
**Date:** 2025-10-06

---

## 1. EXECUTIVE SUMMARY

The **FLAMBOYANT CHINCHILLA** capability is a lightweight remote file access and reconnaissance agent designed for deployment on hardened AArch64 Linux systems. The agent operates in austere environments where traditional tooling and libraries are unavailable, providing persistent file system access and manipulation capabilities to authorized NOC operators.

This document specifies the technical requirements for implementing the FLAMBOYANT CHINCHILLA agent. The accompanying Python-based command and control server (`server.py`) has been provided as a reference implementation of the operator interface.

**Objective:** Deploy a self-contained agent binary on target systems that provides comprehensive file system reconnaissance, exfiltration, and manipulation capabilities while evading security monitoring solutions.

*Swift. Silent. Fabulous.*
---

## 2. OPERATIONAL ENVIRONMENT

### 2.1 Target Platform Specification

The FLAMBOYANT CHINCHILLA agent MUST operate on the following platform:

- **Architecture:** AArch64 (ARM 64-bit)
- **Operating System:** Linux (Ubuntu-based distributions)
- **Kernel Version:** 5.15.0-144-generic (minimum)
- **Runtime Constraint:** NO libc AVAILABLE

**Critical Note:** Target systems do not provide access to standard C libraries. The agent must be compiled with `-nostdlib` and implement all required functionality using direct system calls only.

### 2.2 Target Landscape

Target systems are equipped with advanced security monitoring:

#### 2.2.1 Personal Security Product (PSP) / Intrusion Detection System (IDS)

Active monitoring includes:

- **Disk Access Monitoring:** File reads/writes to sensitive paths trigger alerts
- **Network Monitoring:** Unusual connection patterns flagged for review
- **Memory Monitoring:** Executable memory allocation tracked
- **Behavioral Analysis:** Anomalous syscall sequences detected

#### 2.2.2 Protected Files

Accessing certain file paths may trigger immediate alerts:

#### 2.2.3 Syscall Filtering

Target systems employ syscall filtering (seccomp-bpf potential). Only a limited subset of syscalls are guaranteed to be available. See **Appendix A** for the syscall allowlist.

---

## 3. SYSTEM ARCHITECTURE

### 3.1 High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                         OPERATOR STATION                            │
│                                                                     │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │                      server.py                               │   │
│  │  • Python-based C2 Server                                    │   │
│  │  • Interactive REPL for operator                             │   │
│  │  • Handles authentication & command dispatch                 │   │
│  │  • Saves exfiltrated data to disk                            │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              │ TCP (Network)                        │
│                              │ Port: Configurable                   │
└──────────────────────────────┼──────────────────────────────────────┘
                               │
                               │ RPC Protocol (4096-byte messages)
                               │
┌──────────────────────────────┼──────────────────────────────────────┐
│                              │        TARGET SYSTEM                 │
│                              ▼                                      │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │                   FLAMBOYANT CHINCHILLA Agent                │   │
│  │  • Self-contained AArch64 binary                             │   │
│  │  • No libc dependencies                                      │   │
│  │  • Direct syscall implementation                             │   │
│  │  • File system reconnaissance & exfiltration                 │   │
│  └──────────────────────────────────────────────────────────────┘   │
│                              │                                      │
│                              ▼                                      │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │                    Target File System                        │   │
│  │  • /etc/passwd, /etc/hosts                                   │   │
│  │  • /var/log/* (logs)                                         │   │
│  │  • /home/*/.ssh/* (credentials)                              │   │
│  │  • /proc/* (process information)                             │   │
│  └──────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

### 3.2 Communication Flow

```
┌────────────┐                                          ┌────────────┐
│  Operator  │                                          │   Agent    │
│ (server.py)│                                          │  (target)  │
└──────┬─────┘                                          └──────┬─────┘
       │                                                       │
       │  1. TCP Connection Established                        │
       │◄───────────────────────────────────────────────────── ┤
       │                                                       │
       │  2. Agent sends AUTH_PASSWORD (4 bytes)               │
       │◄───────────────────────────────────────────────────── ┤
       │                                                       │
       │  3. Server validates & responds (0x01=success)        │
       ├──────────────────────────────────────────────────────►
       │                                                       │
       │  4. Server sends RPC_REQUEST (4096 bytes)             │
       ├──────────────────────────────────────────────────────►
       │                                                       │
       │  5. Agent executes command                            │
       │                                                  [EXECUTE]
       │                                                       │
       │  6. Agent sends RPC_RESPONSE (4096 bytes)             │
       │◄───────────────────────────────────────────────────── ┤
       │                                                       │
       │  7. (Optional) Multi-chunk transfer for large data    │
       │◄───────────────────────────────────────────────────── ┤
       │                                                       │
       │  8. Loop: More commands or EXIT                       │
       │                                                       │
       │  9. EXIT command received                             │
       ├──────────────────────────────────────────────────────►
       │                                                       │
       │ 10. Connection closed                                 │
       │                                                       │
       ├──────────────────────────────────────────────────────
```

---

## 4. RPC PROTOCOL SPECIFICATION

### 4.1 Protocol Overview

The FLAMBOYANT CHINCHILLA protocol uses fixed-size binary messages exchanged over TCP. All multi-byte integers are transmitted in **little-endian** byte order (network byte order conversion NOT required for little-endian systems, but use `htonl`/`htons` for portability).

### 4.2 Authentication Phase

Upon connection establishment, the agent MUST immediately send the authentication password.

**Authentication Sequence:**

```
Agent                          Server
  │                              │
  ├─────── AUTH_PASSWORD ────────►  (4 bytes: 0xDEADBEEF)
  │                              │
  │                         [VALIDATE]
  │                              │
  ◄────── AUTH_RESPONSE ─────────┤  (1 byte: 0x01=success, 0x00=fail)
  │                              │
```

- **Agent sends:** 4-byte password (default: `0xDEADBEEF`)
- **Server responds:** 1 byte (`0x01` = authenticated, `0x00` = rejected)
- **Timeout:** Server enforces 2-second timeout for authentication
- **Failure handling:** Connection closed immediately on authentication failure

### 4.3 RPC Request Structure

After authentication, the server sends commands using the RPC_REQUEST format.

```
┌─────────────────────────────────────────────────────────────┐
│                    RPC_REQUEST (4096 bytes)                 │
├──────────┬──────────┬──────────┬──────────┬─────────────────┤
│ cmd_type │ reserved │ reserved │ reserved │   data_len      │
│ (1 byte) │ (1 byte) │ (1 byte) │ (1 byte) │   (4 bytes)     │
│          │          │          │          │  (uint32_t)     │
├──────────┴──────────┴──────────┴──────────┴─────────────────┤
│                                                              │
│                    data (4088 bytes)                         │
│                                                              │
│  • Command-specific payload (paths, patterns, etc.)         │
│  • Null-terminated strings                                  │
│  • Unused bytes should be zero-padded                       │
│                                                              │
└──────────────────────────────────────────────────────────────┘

Total size: 1 + 3 + 4 + 4088 = 4096 bytes
```

**Field Descriptions:**

- **cmd_type (1 byte):** Command identifier (see section 4.5)
- **reserved (3 bytes):** Padding for alignment (must be zero)
- **data_len (4 bytes):** Length of valid data in `data` field (uint32_t, little-endian)
- **data (4088 bytes):** Command payload

### 4.4 RPC Response Structure

The agent responds using the RPC_RESPONSE format.

```
┌─────────────────────────────────────────────────────────────┐
│                   RPC_RESPONSE (4096 bytes)                 │
├──────────┬──────────┬──────────┬──────────┬─────────────────┤
│  status  │ reserved │ reserved │ reserved │   data_len      │
│ (1 byte) │ (1 byte) │ (1 byte) │ (1 byte) │   (4 bytes)     │
│          │          │          │          │  (uint32_t)     │
├──────────┴──────────┴──────────┴──────────┴─────────────────┤
│                                                             │
│                    data (4088 bytes)                        │
│                                                             │
│  • Command results (file contents, directory listings, etc) │
│  • Error messages (if status = STATUS_ERROR)                │
│  • Null-terminated strings where applicable                 │
│                                                             │
└─────────────────────────────────────────────────────────────┘

Total size: 1 + 3 + 4 + 4088 = 4096 bytes
```

**Field Descriptions:**

- **status (1 byte):** Result status code
  - `0x00` (STATUS_OK): Success, operation complete
  - `0x01` (STATUS_ERROR): Error occurred, `data` contains error message
  - `0x02` (STATUS_MORE_DATA): Partial data, more chunks follow
- **reserved (3 bytes):** Padding for alignment (must be zero)
- **data_len (4 bytes):** Length of valid data in `data` field (uint32_t, little-endian)
- **data (4088 bytes):** Response payload

### 4.5 Command Codes

| Code   | Name              | Description                                      |
|--------|-------------------|--------------------------------------------------|
| `0x01` | GET_FILE_STATS    | Retrieve file metadata (size, mode, timestamps) |
| `0x02` | LS                | List directory contents                          |
| `0x03` | PWD               | Get current working directory                    |
| `0x04` | DOWNLOAD_FILE     | Download file from agent (multi-chunk)          |
| `0x05` | BGREP             | Binary pattern search (hex-encoded pattern)     |
| `0x06` | TAILF             | Tail file continuously (streaming)              |
| `0x07` | CANCEL            | Cancel ongoing operation (e.g., tailf)          |
| `0x08` | EXIT              | Terminate agent connection                       |
| `0x09` | UPLOAD_FILE       | Upload file to agent (multi-chunk)              |
| `0x0A` | APPEND_FILE       | Append data to remote file                       |
| `0x0B` | TIMESTOMP         | Modify file timestamps                           |
| `0x0C` | WC                | Count words/lines/bytes in file                  |
| `0x0D` | DJB2SUM           | Calculate DJB2 hash of file                      |
| `0x0E` | SED               | Find and replace in file                         |
| `0x0F` | RUNRWX            | Execute code in RWX memory                       |
| `0x10` | PROC_MAPS         | Read /proc/self/maps                             |

### 4.6 Multi-Chunk Transfer Protocol

For operations that return data larger than 4088 bytes (e.g., file downloads), use the multi-chunk protocol:

```
Server                                    Agent
  │                                         │
  ├────── DOWNLOAD_FILE request ───────────►
  │                                         │
  │                                    [Read file]
  │                                         │
  ◄─── Chunk 1 (STATUS_MORE_DATA) ─────────┤  (4088 bytes of data)
  │                                         │
  ◄─── Chunk 2 (STATUS_MORE_DATA) ─────────┤  (4088 bytes of data)
  │                                         │
  ◄─── Chunk N (STATUS_OK) ────────────────┤  (remaining bytes)
  │                                         │
```

**Rules:**

- Agent sets `status = STATUS_MORE_DATA (0x02)` for all chunks except the last
- Last chunk uses `status = STATUS_OK (0x00)`
- Each chunk fills `data` field with up to 4088 bytes
- `data_len` indicates actual bytes in current chunk

---

## 5. CAPABILITY REQUIREMENTS

### 5.1 File Information Commands

#### 5.1.1 GET_FILE_STATS (0x01)

**Purpose:** Retrieve file metadata without reading content.

**Request Data:**

- Null-terminated file path

**Response Data (STATUS_OK):**

```
Size: 12345
Mode: 644
UID: 1000
GID: 1000
Atime: 1696340000
Mtime: 1696340000
```

**Implementation Requirements:**

- Use `fstat` or `newfstatat` syscall
- Format output as shown above
- Return STATUS_ERROR if file doesn't exist or permission denied

#### 5.1.2 LS (0x02)

**Purpose:** List directory contents.

**Request Data:**

- Null-terminated directory path

**Response Data (STATUS_OK):**

```
[F] file1.txt
[D] subdir
[L] symlink
```

**Implementation Requirements:**

- Use `getdents64` syscall
- Prefix entries with type: `[F]` file, `[D]` directory, `[L]` symlink
- Return STATUS_ERROR for non-existent or inaccessible directories

#### 5.1.3 PWD (0x03)

**Purpose:** Get current working directory.

**Request Data:**

- Empty (ignored)

**Response Data (STATUS_OK):**

- Null-terminated path (e.g., `/home/user`)

**Implementation Requirements:**

- Use `getcwd` syscall

### 5.2 File Transfer Commands

#### 5.2.1 DOWNLOAD_FILE (0x04)

**Purpose:** Exfiltrate file from target.

**Request Data:**

- Null-terminated file path

**Response Data (Multi-chunk):**

- Raw file bytes
- Use STATUS_MORE_DATA for all chunks except last
- Final chunk uses STATUS_OK

**Implementation Requirements:**

- Use `openat` + `read` syscalls
- Stream file in 4088-byte chunks
- Handle large files efficiently (don't load entire file into memory)
- Return STATUS_ERROR if file not accessible

#### 5.2.2 UPLOAD_FILE (0x09)

**Purpose:** Upload file to target.

**Protocol:**

1. Server sends initial request with remote path
2. Agent responds with STATUS_OK (ready to receive)
3. Server sends chunks (each as UPLOAD_FILE request with file data)
4. Agent responds STATUS_OK after each chunk written
5. Server sends empty chunk (data_len=0) to signal completion
6. Agent responds with STATUS_OK and completion message

**Implementation Requirements:**

- Use `openat` with `O_WRONLY | O_CREAT | O_TRUNC`
- Write chunks as received
- Close file on empty chunk

#### 5.2.3 APPEND_FILE (0x0A)

**Purpose:** Append data to existing file.

**Protocol:** Same as UPLOAD_FILE, but use `O_APPEND` flag instead of `O_TRUNC`.

### 5.3 Search and Analysis Commands

#### 5.3.1 BGREP (0x05)

**Purpose:** Search file for binary pattern. Server auto-converts text to hex before sending to agent.

**Request Data:**

```
/bin/ls\0454c46
(path\0hex_pattern)
```

Note: Server automatically converts text patterns like "ELF" to hex "454c46" before sending.

**Response Data (Streaming):**

- STATUS_MORE_DATA for each match: `"Found at offset: N (0xHEX)\n"`
- STATUS_OK when search complete: `"Search complete"` or `"No matches found"`

**Implementation Requirements:**

- Parse hex pattern to bytes using `parse_hex()`
- Use chunked file reading with `pread64()` (not mmap)
- Handle pattern boundaries across chunks with overlap buffer
- Stream results as STATUS_MORE_DATA for each match
- Send final STATUS_OK when complete

#### 5.3.2 TAILF (0x06)

**Purpose:** Monitor file for new content (like `tail -f`).

**Request Data:**

- Null-terminated file path

**Response Data (Streaming):**

- Initial: Send current file content
- Continuous: Use `inotify_add_watch` with `IN_MODIFY`
- Send new content as STATUS_MORE_DATA chunks
- Terminate on CANCEL (0x07) command or error

**Implementation Requirements:**

- Use `inotify_init1` + `inotify_add_watch` syscalls
- Use `ppoll` to wait for file modifications
- Handle CANCEL command gracefully (respond with STATUS_OK and message)

### 5.4 File Manipulation Commands

#### 5.4.1 SED (0x0F)

**Purpose:** Find and replace text in file.

**Request Data:**

```
/etc/config\0oldtext\0newtext
(path\0search\0replace)
```

**Response Data (Multi-chunk):**

- Modified file contents
- Stream as STATUS_MORE_DATA chunks
- Final chunk STATUS_OK

**Implementation Requirements:**

- Read file into buffer
- Perform find/replace operations
- Stream modified content back

#### 5.4.2 TIMESTOMP (0x0C)

**Purpose:** Modify file access/modification times (anti-forensics).

**Request Data:**

```
/path/to/file\0atime\0mtime
```

**Response Data (STATUS_OK):**

- Confirmation message

**Implementation Requirements:**

- Use `utimensat` syscall
- Parse timestamps from request

#### 5.4.3 WC (0x0D)

**Purpose:** Count lines, words, bytes in file.

**Request Data:**

- Null-terminated file path

**Response Data (STATUS_OK):**

```
Lines: 1234
Words: 5678
Bytes: 90123
```

#### 5.4.4 DJB2SUM (0x0E)

**Purpose:** Calculate DJB2 hash of file (non-cryptographic checksum).

**Request Data:**

- Null-terminated file path

**Response Data (STATUS_OK):**

```
DJB2: 5381a2b3c4d5e6f7
```

**Implementation Requirements:**

- DJB2 algorithm: `hash = hash * 33 + byte`
- Start with `hash = 5381`

### 5.5 Advanced Commands

#### 5.5.1 RUNRWX (0x10)

**Purpose:** Execute shellcode in RWX memory region.

**Request Data:**

- Raw shellcode bytes (AArch64 instructions)

**Response Data:**

- Output from shellcode (if any)

**Implementation Requirements:**

- Use `mmap` with `PROT_READ | PROT_WRITE | PROT_EXEC`
- Copy shellcode to mapped region
- Cast to function pointer and execute
- **WARNING:** Extremely dangerous, implement with caution

#### 5.5.2 PROC_MAPS (0x11)

**Purpose:** Read agent's own memory map.

**Request Data:**

- Empty (ignored)

**Response Data:**

- Contents of `/proc/self/maps`

**Implementation Requirements:**

- Open `/proc/self/maps`
- Read and return contents

---

## 6. IMPLEMENTATION REQUIREMENTS

### 6.1 Build Configuration

**Entry Point:**

- Function name: `_start` (not `main`)
- Signature: `void _start(void)`
- Must manually call `exit` syscall when done

### 6.2 Directory Structure

Students are provided with:

```
.
├── inc/
│   ├── types.h         # Provided: Basic types, structures
│   ├── syscalls.h      # Provided: Syscall numbers and wrappers
│   ├── rpc.h           # Provided: RPC protocol structures
│   ├── utils.h         # Provided: Utility function declarations
│   └── cmds.h          # Provided: Command handler declarations
├── src/
│   ├── agent.c         # TO IMPLEMENT: Main entry point
│   ├── utils.c         # String, network, formatting utilities
│   ├── minc.c          # Minimal libc replacements
│   └── cmds/
│       ├── cmd_stats.c      # TO IMPLEMENT
│       ├── cmd_ls.c         # TO IMPLEMENT
│       ├── cmd_pwd.c        # TO IMPLEMENT
│       ├── cmd_download.c   # TO IMPLEMENT
│       ├── cmd_upload.c     # TO IMPLEMENT
│       ├── cmd_append.c     # TO IMPLEMENT
│       ├── cmd_bgrep.c      # TO IMPLEMENT
│       ├── cmd_tailf.c      # TO IMPLEMENT
│       ├── cmd_sed.c        # TO IMPLEMENT
│       ├── cmd_timestomp.c  # TO IMPLEMENT
│       ├── cmd_wc.c         # TO IMPLEMENT
│       ├── cmd_djb2sum.c    # TO IMPLEMENT
│       ├── cmd_runrwx.c     # TO IMPLEMENT
│       └── cmd_proc_maps.c  # TO IMPLEMENT
├── builder.py      # Provided: Generate config.h
├── server.py       # Provided: C2 server
├── Makefile        # Provided: Build system
└── SPEC.md         # This document
```

### 6.3 Required Utilities

**String Functions:**

- `strlen()`, `strcpy()`, `strcmp()`, `memcpy()`, `memset()`, `memcmp()`, `memmem()`

**Formatting:**

- `int_to_str()` - Convert integer to string
- `simple_format()` - Basic sprintf-like formatting

**Network:**

- `htons()`, `htonl()`, `ntohl()`, `ntohs()` - Byte order conversion
- `recv_exact()` - Receive exact number of bytes
- `send_exact()` - Send exact number of bytes
- `send_response()` - Send RPC response structure

**Hashing:**

- `djb2_hash()` - DJB2 hash algorithm
- `write_hex_to_buf()` - Format hex output

### 6.4 Configuration

Connection parameters are defined in `config.h` (generated by `builder.py`):

```c
#define SERVER_IP_OCTET_0 127
#define SERVER_IP_OCTET_1 0
#define SERVER_IP_OCTET_2 0
#define SERVER_IP_OCTET_3 1
#define SERVER_PORT_NBO htons(4444)  // Port in network byte order
#define AUTH_PASSWORD {0xDE, 0xAD, 0xBE, 0xEF}
#define AUTH_PASSWORD_LEN 4
```

Generate with:

```bash
python3 builder.py <ip> <port> <password_hex>
```

---

## 7. TESTING AND VALIDATION

### 7.1 Reference Server

The provided `server.py` serves as the authoritative reference for protocol behavior. Test your agent implementation against it:

```bash
# Terminal 1: Start server
python3 server.py 4444

# Terminal 2: Run agent
./bin/agent
```

### 7.2 Checklist

- [ ] Agent connects and authenticates successfully
- [ ] All  commands respond without crashing
- [ ] Multi-chunk transfers work for large files (>4088 bytes)
- [ ] Error conditions return STATUS_ERROR with message
- [ ] Agent exits cleanly on EXIT command
- [ ] No libc dependencies (`ldd agent` shows "not a dynamic executable")
- [ ] Binary size is minimal (<50KB after stripping)

### 7.3 Automation

See the provided `example.rc` file for how to automate commands:

```bash
python3 server.py 4444 --rc example.rc --downloads-dir ./loot
```

This will:

1. Get current directory (PWD)
2. Download `/etc/passwd`
3. List `/var/log`
4. Tail `/var/log/syslog`

See how  `server.py` handles `--rc`  and `--oncon` connection hooks to optionally extend functionality through automation. I.e. Chain a call of ls and download to get all the files in a directory.

---

## APPENDIX A: APPROVED SYSCALL ALLOWLIST

The following syscalls are guaranteed to be available on target systems. **No other syscalls may be used.**

### Allowed Syscalls (AArch64 Linux)

```
# File Operations
openat        56      # Open file relative to directory
close         57      # Close file descriptor
read          63      # Read from file descriptor
write         64      # Write to file descriptor
readv         65      # Vectored read (scatter)
writev        66      # Vectored write (gather)
pread64       67      # Positional read (no offset change)
lseek         62      # Seek to position in file
fstat         80      # Get file status by fd
newfstatat    79      # Get file status by path
getdents64    61      # Get directory entries
unlinkat      35      # Delete file/directory
renameat2     276     # Rename file atomically
sendfile      71      # Copy between file descriptors
fcntl         25      # File control operations

# Memory Operations
mmap          222     # Map memory (files, anonymous, RWX)
munmap        215     # Unmap memory region
mprotect      226     # Change memory protection flags
brk           214     # Change data segment size (heap)

# Network Operations
socket        198     # Create socket endpoint
bind          200     # Bind socket to address
listen        201     # Listen for connections
accept        202     # Accept incoming connection
connect       203     # Connect to remote address
sendto        206     # Send data to socket
recvfrom      207     # Receive data from socket
setsockopt    208     # Set socket options
getsockopt    209     # Get socket options
shutdown      210     # Shutdown socket connection

# I/O Multiplexing
ppoll         73      # Poll file descriptors (with signal mask)
epoll_create1 20      # Create epoll instance
epoll_ctl     21      # Control epoll interface
epoll_pwait   22      # Wait for epoll events

# File Monitoring
inotify_init1      26  # Initialize inotify instance
inotify_add_watch  27  # Add file to inotify watch list
inotify_rm_watch   28  # Remove file from watch list

# File Permissions & Ownership
fchmod        52      # Change file mode (permissions)
fchmodat      53      # Change file mode (at path)
fchown        55      # Change file ownership (by fd)
fchownat      54      # Change file ownership (at path)

# Time Operations
nanosleep     101     # Sleep for specified nanoseconds
clock_gettime 113     # Get time from specified clock
utimensat     88      # Change file access/modification times

# Process & System Info
exit          93      # Terminate calling process
exit_group    94      # Terminate all threads
getcwd        17      # Get current working directory
getpid        172     # Get process ID
getuid        174     # Get real user ID
geteuid       175     # Get effective user ID
getgid        176     # Get real group ID
getegid       177     # Get effective group ID
uname         160     # Get system information

# Signal Handling (limited)
rt_sigaction  134     # Examine/change signal action
rt_sigprocmask 135    # Examine/change blocked signals
```

### Explicitly BLOCKED Syscalls

The following syscalls are **BLOCKED** by seccomp filtering on target systems:

```
# Process Creation & Execution (BLOCKED)
execve        221     # Execute program - BLOCKED
execveat      281     # Execute program at fd - BLOCKED
fork          1098    # Create child process - BLOCKED
vfork         1071    # Create child process (shared mem) - BLOCKED
clone         220     # Create child process/thread - BLOCKED
clone3        435     # Modern clone interface - BLOCKED
ptrace        117     # Process trace/debug - BLOCKED
```

### Notes

- All syscall numbers are for **AArch64** architecture
- The target uses **seccomp-bpf** to enforce this allowlist
- Attempting blocked syscalls will result in immediate process termination (SIGSYS)
- Use only the allowed syscalls for maximum compatibility
- Some advanced features (epoll, inotify_rm_watch) are optional but recommended

---

## APPENDIX B: AArch64 SYSCALL INVOCATION

### Register Conventions

AArch64 syscalls use the following registers:

- **x8:** Syscall number
- **x0-x5:** Arguments 1-6
- **x0:** Return value (after `svc #0`)

### Example: Write Syscall

```c
// Write "Hello\n" to stdout
static inline long syscall3(long num, long arg1, long arg2, long arg3) {
    register long x8 __asm__("x8") = num;
    register long x0 __asm__("x0") = arg1;
    register long x1 __asm__("x1") = arg2;
    register long x2 __asm__("x2") = arg3;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
    return x0;
}

// Usage
syscall3(SYS_write, 1, (long)"Hello\n", 6);
```

All syscall wrappers are provided in `syscalls.h`.

---

## APPENDIX C: OPERATIONAL SECURITY CONSIDERATIONS

### C.1 Unknown PSP

The target is using an unknown PSP. Out intel suggests we can avoid triggering the PSP/IDS by adhering to the following:

1. **Avoid rapid sequential access** to protected files
2. **Use legitimate syscalls** - all operations should appear as normal file I/O
3. **Limit network traffic** - avoid sending large bursts
4. **No string literals** - obfuscate sensitive strings if possible
5. **Minimal memory footprint** - avoid large allocations
6. **Watch out for monitored files** - avoid directly accessing protected files

### C.2 Anti-Forensics

- **Timestomping:** Use TIMESTOMP command to restore original timestamps after access
- **No logs:** Agent produces no local logs

### C.3 Failure Modes

Agent should fail gracefully:

- **Authentication failure:** Exit immediately, no retry
- **Connection loss:** Exit cleanly
- **Syscall failures:** Return STATUS_ERROR to server, continue operation
- **Invalid commands:** Respond with error, do not crash

---

## APPENDIX D: GRADING RUBRIC

Your FLAMBOYANT CHINCHILLA implementation will be evaluated on:

| Category                      | Points | Criteria                                          |
|-------------------------------|--------|---------------------------------------------------|
| **Compilation**               | 10     | Builds without errors, no libc dependencies       |
| **Authentication**            | 10     | Successful handshake with server                  |
| **Basic Commands (5)**        | 25     | PWD, LS, STATS, DOWNLOAD, UPLOAD                  |
| **Search Commands (2)**       | 15     | BGREP, TAILF                                      |
| **Advanced Commands (5)**     | 20     | SED, WC, DJB2SUM, TIMESTOMP, PROC_MAPS            |
| **RUNRWX**                    | 10     | Execute shellcode correctly                       |
| **Code Quality**              | 5      | Clean, readable, well-commented                   |
| **Error Handling**            | 5      | Graceful failure, appropriate error messages      |
| **Total**                     | 100    |                                                   |

---

## APPENDIX E: FREQUENTLY ASKED QUESTIONS

**Q: Can I use external libraries?**
A: No. The agent must be self-contained with `-nostdlib`. Only syscalls from Appendix A are permitted.

**Q: What if I can't get RUNRWX working?**
A: RUNRWX is the most advanced command. Implement it last. Partial credit for attempting.

**Q: How do I debug without printf?**
A: Use `write(2, msg, len)` to write to stderr. Or test individual functions with a temporary libc build, then remove it for final submission.

**Q: The server.py disconnects immediately. Why?**
A: Check authentication. Your agent must send exactly 4 bytes (0xDEADBEEF) immediately after connection.

**Q: How does BGREP work?**
A: BGREP searches for binary patterns. The server automatically converts text strings to hex before sending to the agent. For example, "ELF" becomes "454c46".

**Q: Do I need to implement the CANCEL command?**
A: The agent receives CANCEL to stop TAILF. Implement TAILF to listen for it. CANCEL is not a separate handler.

---

## DOCUMENT CONTROL

**Revision History:**

| Version | Date       | Author         | Changes                        |
|---------|------------|----------------|--------------------------------|
| 1.0     | 2025-10-06 | NOC-Alpha-6    | Initial release                |

**Approval:**

- **Technical Review:** ghost, NOC Cyber Operations Division
- **Classification Authority:** Deputy Director, Network Observability Command

---

```
═══════════════════════════════════════════════════════════════════════════
                    END OF FUNCLASSIFIED DOCUMENT
        UNAUTHORIZED DISCLOSURE SUBJECT TO CRIMINAL SANCTIONS
═══════════════════════════════════════════════════════════════════════════
```

**FLAMBOYANT CHINCHILLA**
