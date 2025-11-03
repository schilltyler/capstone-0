#!/usr/bin/env python3
"""
Client script to upload and run an ELF file on the fork server.

Usage:
    ./run.py <elf_file> [host] [port]

Arguments:
    elf_file    Path to the local ELF file to upload and run
    host        Server hostname or IP (default: localhost)
    port        Server port (default: 9999)

The script uploads the ELF file to the fork server and displays the output.
"""

import os
import sys
import socket
import struct
from pathlib import Path

DEFAULT_HOST = 'localhost'
DEFAULT_PORT = 9999
MAX_FILE_SIZE = 1024 * 1024  # 1 MB

def log(msg):
    """Simple logging function"""
    print(f"[run.py] {msg}", file=sys.stderr, flush=True)

def upload_and_run(elf_path, host, port):
    """Upload ELF file to server and display output"""

    # Validate file exists
    if not os.path.exists(elf_path):
        log(f"ERROR: File not found: {elf_path}")
        return 1

    # Read ELF file
    try:
        with open(elf_path, 'rb') as f:
            elf_data = f.read()
    except Exception as e:
        log(f"ERROR: Failed to read {elf_path}: {e}")
        return 1

    file_size = len(elf_data)
    log(f"Read {file_size} bytes from {elf_path}")

    # Validate size
    if file_size > MAX_FILE_SIZE:
        log(f"ERROR: File size {file_size} exceeds maximum {MAX_FILE_SIZE} bytes (1 MB)")
        return 1

    if file_size == 0:
        log(f"ERROR: File is empty")
        return 1

    # Connect to server
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        log(f"Connecting to {host}:{port}...")
        sock.connect((host, port))
        log(f"Connected!")
    except Exception as e:
        log(f"ERROR: Failed to connect to {host}:{port}: {e}")
        return 1

    try:
        # Send size (4 bytes, little-endian)
        size_bytes = struct.pack('<I', file_size)
        sock.sendall(size_bytes)
        log(f"Sent size: {file_size} bytes")

        # Send ELF data
        sock.sendall(elf_data)
        log(f"Sent ELF data")

        # Receive and display server response and program output
        log("Receiving output from server...")
        print("=" * 60, flush=True)

        while True:
            try:
                data = sock.recv(4096)
                if not data:
                    break
                # Print raw output to stdout
                sys.stdout.buffer.write(data)
                sys.stdout.buffer.flush()
            except socket.timeout:
                break
            except Exception as e:
                log(f"Error receiving data: {e}")
                break

        print("\n" + "=" * 60, flush=True)
        log("Connection closed")

    except Exception as e:
        log(f"ERROR: Communication error: {e}")
        return 1
    finally:
        sock.close()

    return 0

def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <elf_file> [host] [port]", file=sys.stderr)
        print(f"\nExample:", file=sys.stderr)
        print(f"  {sys.argv[0]} ./bin/agent", file=sys.stderr)
        print(f"  {sys.argv[0]} ./bin/agent localhost 9999", file=sys.stderr)
        return 1

    elf_path = sys.argv[1]
    host = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_HOST
    port = int(sys.argv[3]) if len(sys.argv) > 3 else DEFAULT_PORT

    return upload_and_run(elf_path, host, port)

if __name__ == '__main__':
    sys.exit(main())
