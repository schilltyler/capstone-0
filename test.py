#!/usr/bin/env python3
"""
Test script for agent functionality.
Spawns a listener, runs the agent, and tests all RPC commands.
"""

import socket
import struct
import sys
import os
import tempfile
import shutil
import time
import subprocess
import threading
from pathlib import Path

# Import constants from server.py
from server import (
    CMD_GET_FILE_STATS,
    CMD_LS,
    CMD_PWD,
    CMD_DOWNLOAD_FILE,
    CMD_BGREP,
    CMD_TAILF,
    CMD_CANCEL,
    CMD_EXIT,
    CMD_UPLOAD_FILE,
    CMD_APPEND_FILE,
    CMD_TIMESTOMP,
    CMD_WC,
    CMD_DJB2SUM,
    CMD_SED,
    CMD_RUNRWX,
    CMD_PROC_MAPS,
    STATUS_OK,
    STATUS_ERROR,
    STATUS_MORE_DATA,
    RPC_REQUEST_SIZE,
    RPC_RESPONSE_SIZE,
    RPC_DATA_SIZE,
    RPCRequest,
    RPCResponse,
    send_request,
    recv_response,
)

# Test configuration
SERVER_PORT = 4444
AUTH_PASSWORD = bytes.fromhex("DEADBEEF")
AGENT_BIN = "./bin/agent"


class TestAgent:
    """Test suite for agent functionality"""

    def __init__(self, port=SERVER_PORT, agent_bin=AGENT_BIN, use_strace=False):
        self.port = port
        self.agent_bin = agent_bin
        self.use_strace = use_strace
        self.server_sock = None
        self.client_sock = None
        self.agent_process = None
        self.test_dir = None
        self.download_dir = None
        self.passed = 0
        self.failed = 0

    def setup(self):
        """Set up test environment"""
        print("[*] Setting up test environment...")

        # Create temporary directories
        self.test_dir = tempfile.mkdtemp(prefix="agent_test_")
        self.download_dir = tempfile.mkdtemp(prefix="agent_downloads_")

        print(f"    Test directory: {self.test_dir}")
        print(f"    Download directory: {self.download_dir}")

        # Create test files in test_dir
        test_file = os.path.join(self.test_dir, "test.txt")
        with open(test_file, "w") as f:
            f.write("Hello World!\nThis is a test file.\nLine 3\n")

        test_file2 = os.path.join(self.test_dir, "grep_test.txt")
        with open(test_file2, "w") as f:
            f.write("apple\nbanana\napple pie\ncherry\napple juice\n")

        # Create binary test file
        binary_file = os.path.join(self.test_dir, "binary.dat")
        with open(binary_file, "wb") as f:
            f.write(b"\x00\x01\x02\x03\x04\x05" * 100)

        print("[+] Test environment ready")

    def teardown(self):
        """Clean up test environment"""
        print("\n[*] Cleaning up test environment...")

        # Kill agent process and capture output
        if self.agent_process:
            try:
                self.agent_process.terminate()
                self.agent_process.wait(timeout=2)
                print("    Agent process terminated")
            except:
                try:
                    self.agent_process.kill()
                    self.agent_process.wait(timeout=1)
                    print("    Agent process killed")
                except:
                    pass

            # Print agent output for debugging
            if self.agent_process.stdout:
                stdout = self.agent_process.stdout.read().decode('utf-8', errors='replace')
                if stdout:
                    print("\n[DEBUG] Agent stdout:")
                    print(stdout[:2000])  # First 2000 chars

            if self.agent_process.stderr:
                stderr = self.agent_process.stderr.read().decode('utf-8', errors='replace')
                if stderr:
                    print("\n[DEBUG] Agent stderr (last 2000 chars):")
                    print(stderr[-2000:])  # Last 2000 chars

        # Close sockets
        if self.client_sock:
            try:
                self.client_sock.close()
            except:
                pass

        if self.server_sock:
            try:
                self.server_sock.close()
            except:
                pass

        # Remove temporary directories
        if self.test_dir and os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir)
            print(f"    Removed: {self.test_dir}")

        if self.download_dir and os.path.exists(self.download_dir):
            shutil.rmtree(self.download_dir)
            print(f"    Removed: {self.download_dir}")

        print("[+] Cleanup complete")

    def start_listener(self):
        """Start TCP listener for agent connection"""
        print(f"\n[*] Starting listener on port {self.port}...")

        self.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_sock.bind(("127.0.0.1", self.port))
        self.server_sock.listen(1)

        print(f"[+] Listening on 127.0.0.1:{self.port}")

    def start_agent(self):
        """Start the agent binary"""
        print(f"\n[*] Starting agent: {self.agent_bin}")

        # Check if binary exists
        if not os.path.exists(self.agent_bin):
            raise FileNotFoundError(f"Agent binary not found: {self.agent_bin}")

        # Detect host architecture
        host_arch = os.uname().machine

        # Detect binary architecture
        file_output = subprocess.check_output(["file", self.agent_bin]).decode()

        if "x86-64" in file_output or "x86_64" in file_output:
            # x86-64 binary
            if host_arch in ["x86_64", "AMD64"]:
                # Native x86-64, run directly
                cmd = [self.agent_bin]
                print("    Running native x86-64 agent")
            else:
                raise RuntimeError(f"Cannot run x86-64 binary on {host_arch} host")
        elif "ARM aarch64" in file_output or "aarch64" in file_output:
            # AArch64 binary
            if host_arch in ["aarch64", "arm64"]:
                # Native AArch64, run directly
                cmd = [self.agent_bin]
                if self.use_strace:
                    # Use strace directly on native execution
                    cmd = ["strace", "-o", "/dev/stderr"] + cmd
                    print("    Running native AArch64 agent with strace")
                else:
                    print("    Running native AArch64 agent")
            else:
                # Cross-architecture, use qemu
                if not shutil.which("qemu-aarch64"):
                    raise RuntimeError(
                        "Agent is AArch64 but qemu-aarch64 not found. "
                        "Install with: sudo apt-get install qemu-user"
                    )
                # Get QEMU_LD_PREFIX from environment or use default
                ld_prefix = os.environ.get("QEMU_LD_PREFIX", "/usr/aarch64-linux-gnu")
                cmd = ["qemu-aarch64"]
                if self.use_strace:
                    cmd.append("--strace")
                cmd.extend(["-L", ld_prefix, self.agent_bin])
                strace_str = " --strace" if self.use_strace else ""
                print(f"    Running AArch64 agent with qemu-aarch64{strace_str} -L {ld_prefix}")
        else:
            raise RuntimeError(f"Unknown agent architecture: {file_output}")

        # Start agent in background
        self.agent_process = subprocess.Popen(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )

        print(f"[+] Agent started (PID: {self.agent_process.pid})")

    def accept_connection(self):
        """Accept connection from agent and authenticate"""
        print("\n[*] Waiting for agent connection...")

        # Set timeout for accept
        self.server_sock.settimeout(5.0)

        try:
            self.client_sock, addr = self.server_sock.accept()
            print(f"[+] Agent connected from {addr}")

            # Receive authentication password
            auth = self.client_sock.recv(len(AUTH_PASSWORD))

            if auth != AUTH_PASSWORD:
                raise RuntimeError(f"Authentication failed: {auth.hex()}")

            # Send authentication confirmation (0x01 = success)
            self.client_sock.sendall(b"\x01")

            print("[+] Agent authenticated")

        except socket.timeout:
            raise RuntimeError("Timeout waiting for agent connection")

        finally:
            self.server_sock.settimeout(None)

    def test_pwd(self):
        """Test CMD_PWD - Print working directory"""
        print("\n[TEST] pwd")

        try:
            req = RPCRequest(CMD_PWD, b"")
            send_request(self.client_sock, req)

            status, data = recv_response(self.client_sock)

            if status == STATUS_OK:
                cwd = data.decode("utf-8")
                print(f"  [✓] Current directory: {cwd}")
                self.passed += 1
            else:
                print(f"  [✗] Failed: {data.decode('utf-8')}")
                self.failed += 1
        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_stats(self):
        """Test CMD_GET_FILE_STATS - Get file statistics"""
        print("\n[TEST] stats")

        try:
            test_file = os.path.join(self.test_dir, "test.txt")
            req = RPCRequest(CMD_GET_FILE_STATS, test_file.encode("utf-8"))
            send_request(self.client_sock, req)

            status, data = recv_response(self.client_sock)

            if status == STATUS_OK:
                stats = data.decode("utf-8")
                print(f"  [✓] File stats:")
                for line in stats.strip().split("\n"):
                    print(f"      {line}")
                self.passed += 1
            else:
                print(f"  [✗] Failed: {data.decode('utf-8')}")
                self.failed += 1
        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_ls(self):
        """Test CMD_LS - List directory"""
        print("\n[TEST] ls")

        try:
            req = RPCRequest(CMD_LS, self.test_dir.encode("utf-8"))
            send_request(self.client_sock, req)

            status, data = recv_response(self.client_sock)

            if status == STATUS_OK:
                listing = data.decode("utf-8")
                print(f"  [✓] Directory listing:")
                for line in listing.strip().split("\n"):
                    print(f"      {line}")
                self.passed += 1
            else:
                print(f"  [✗] Failed: {data.decode('utf-8')}")
                self.failed += 1
        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_download(self):
        """Test CMD_DOWNLOAD_FILE - Download file from agent"""
        print("\n[TEST] download")

        try:
            test_file = os.path.join(self.test_dir, "test.txt")
            download_path = os.path.join(self.download_dir, "downloaded.txt")

            req = RPCRequest(CMD_DOWNLOAD_FILE, test_file.encode("utf-8"))
            send_request(self.client_sock, req)

            # Receive file chunks - agent sends multiple responses
            file_data = b""
            while True:
                status, data = recv_response(self.client_sock)

                if status == STATUS_ERROR:
                    print(f"  [✗] Download failed: {data.decode('utf-8')}")
                    self.failed += 1
                    return
                elif status == STATUS_MORE_DATA:
                    # More data coming, accumulate it
                    file_data += data
                elif status == STATUS_OK:
                    # Final chunk - STATUS_OK with data_len=0 means EOF
                    # Don't add data on STATUS_OK, file is complete
                    break

            # Save downloaded file
            with open(download_path, "wb") as f:
                f.write(file_data)

            print(f"  [✓] Downloaded {len(file_data)} bytes")

            # Verify content
            with open(test_file, "rb") as f:
                original = f.read()

            if file_data == original:
                print("  [✓] Content matches original")
                self.passed += 1
            else:
                print(f"  [✗] Content mismatch!")
                print(f"      Expected {len(original)} bytes, got {len(file_data)} bytes")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_upload(self):
        """Test CMD_UPLOAD_FILE - Upload file to agent"""
        print("\n[TEST] upload")

        try:
            upload_path = os.path.join(self.test_dir, "uploaded.txt")
            upload_content = b"This is uploaded content!\nLine 2\nLine 3\n"

            req = RPCRequest(CMD_UPLOAD_FILE, upload_path.encode("utf-8"))
            send_request(self.client_sock, req)

            # Wait for ready response
            status, data = recv_response(self.client_sock)
            if status != STATUS_OK:
                print(f"  [✗] Upload failed: {data.decode('utf-8')}")
                self.failed += 1
                return

            # Send file chunks
            chunk_size = RPC_DATA_SIZE
            offset = 0

            while offset < len(upload_content):
                chunk = upload_content[offset : offset + chunk_size]
                chunk_req = RPCRequest(CMD_UPLOAD_FILE, chunk)
                send_request(self.client_sock, chunk_req)

                status, data = recv_response(self.client_sock)
                if status != STATUS_MORE_DATA:
                    print(f"  [✗] Unexpected status during upload: {status} (expected {STATUS_MORE_DATA})")
                    print(f"      Response: {data.decode('utf-8', errors='replace')[:100]}")
                    self.failed += 1
                    return

                offset += len(chunk)

            # Send final empty chunk
            final_req = RPCRequest(CMD_UPLOAD_FILE, b"")
            send_request(self.client_sock, final_req)

            status, data = recv_response(self.client_sock)
            if status == STATUS_OK:
                print(f"  [✓] {data.decode('utf-8')}")

                # Verify uploaded file
                with open(upload_path, "rb") as f:
                    uploaded = f.read()

                if uploaded == upload_content:
                    print("  [✓] Upload verified")
                    self.passed += 1
                else:
                    print("  [✗] Uploaded content mismatch!")
                    self.failed += 1
            else:
                print(f"  [✗] Upload failed: {data.decode('utf-8')}")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_append(self):
        """Test CMD_APPEND_FILE - Append to file"""
        print("\n[TEST] append")

        try:
            append_path = os.path.join(self.test_dir, "test.txt")
            append_content = b"APPENDED LINE\n"

            # Get original content
            with open(append_path, "rb") as f:
                original = f.read()

            req = RPCRequest(CMD_APPEND_FILE, append_path.encode("utf-8"))
            send_request(self.client_sock, req)

            # Wait for ready response
            status, data = recv_response(self.client_sock)
            if status != STATUS_OK:
                print(f"  [✗] Append failed: {data.decode('utf-8')}")
                self.failed += 1
                return

            # Send append data
            chunk_req = RPCRequest(CMD_APPEND_FILE, append_content)
            send_request(self.client_sock, chunk_req)

            status, data = recv_response(self.client_sock)
            if status != STATUS_MORE_DATA:
                print(f"  [✗] Unexpected status during append: {status} (expected {STATUS_MORE_DATA})")
                print(f"      Response: {data.decode('utf-8', errors='replace')[:100]}")
                self.failed += 1
                return

            # Send final empty chunk
            final_req = RPCRequest(CMD_APPEND_FILE, b"")
            send_request(self.client_sock, final_req)

            status, data = recv_response(self.client_sock)
            if status == STATUS_OK:
                print(f"  [✓] {data.decode('utf-8')}")

                # Verify appended file
                with open(append_path, "rb") as f:
                    result = f.read()

                if result == original + append_content:
                    print("  [✓] Append verified")
                    self.passed += 1
                else:
                    print("  [✗] Appended content mismatch!")
                    self.failed += 1
            else:
                print(f"  [✗] Append failed: {data.decode('utf-8')}")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_bgrep(self):
        """Test CMD_BGREP - Search file for binary pattern"""
        print("\n[TEST] bgrep")

        try:
            test_file = os.path.join(self.test_dir, "grep_test.txt")
            pattern = "apple"

            # Convert pattern to hex (like server.py does)
            hex_pattern = pattern.encode('utf-8').hex()

            # Pack: path\0hex_pattern
            args = test_file.encode("utf-8") + b"\x00" + hex_pattern.encode('utf-8')
            req = RPCRequest(CMD_BGREP, args)
            send_request(self.client_sock, req)

            # Collect streaming results
            results = []
            while True:
                status, data = recv_response(self.client_sock)

                if status == STATUS_ERROR:
                    print(f"  [✗] Error: {data.decode('utf-8')}")
                    self.failed += 1
                    return
                elif status == STATUS_MORE_DATA:
                    # Accumulate results
                    text = data.decode("utf-8")
                    results.append(text)
                elif status == STATUS_OK:
                    # Final message
                    final_msg = data.decode("utf-8")
                    break

            # Display results
            if results:
                print(f"  [✓] Bgrep results:")
                for result in results[:3]:  # Show first 3
                    print(f"      {result.strip()}")

                # Should find 3 occurrences of "apple"
                if len(results) == 3:
                    print("  [✓] Found all expected matches (3 offsets)")
                    self.passed += 1
                else:
                    print(f"  [✗] Expected 3 matches, got {len(results)}")
                    self.failed += 1
            else:
                print("  [✗] No matches found")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_djb2sum(self):
        """Test CMD_DJB2SUM - Calculate DJB2 hash"""
        print("\n[TEST] djb2sum")

        try:
            test_file = os.path.join(self.test_dir, "test.txt")

            req = RPCRequest(CMD_DJB2SUM, test_file.encode("utf-8"))
            send_request(self.client_sock, req)

            status, data = recv_response(self.client_sock)

            if status == STATUS_OK:
                hash_value = data.decode("utf-8").strip()
                print(f"  [✓] DJB2 hash: {hash_value}")

                # Basic validation - should be hex string (uppercase or lowercase)
                # Remove any 'u' suffix that might be added by simple_format
                hash_clean = hash_value.rstrip('u')
                if len(hash_clean) > 0 and all(
                    c in "0123456789ABCDEFabcdef" for c in hash_clean
                ):
                    print("  [✓] Valid hash format")
                    self.passed += 1
                else:
                    print(f"  [✗] Invalid hash format: '{hash_value}'")
                    self.failed += 1
            else:
                print(f"  [✗] Failed: {data.decode('utf-8')}")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_sed(self):
        """Test CMD_SED - Find and replace in file"""
        print("\n[TEST] sed")

        try:
            test_file = os.path.join(self.test_dir, "sed_test.txt")

            # Create test file
            with open(test_file, "w") as f:
                f.write("foo bar foo\nfoo baz\nbar foo bar\n")

            # Pack: filename\0search\0replace
            args = test_file.encode("utf-8") + b"\x00" + b"foo" + b"\x00" + b"FOO"
            req = RPCRequest(CMD_SED, args)
            send_request(self.client_sock, req)

            status, data = recv_response(self.client_sock)

            if status == STATUS_OK:
                result = data.decode("utf-8")
                print(f"  [✓] Sed result: {result}")

                # Verify file was modified
                with open(test_file, "r") as f:
                    content = f.read()

                if "FOO" in content and "foo" not in content:
                    print("  [✓] Replacement verified")
                    self.passed += 1
                else:
                    print("  [✗] Replacement failed")
                    print(f"    Content: {content}")
                    self.failed += 1
            else:
                print(f"  [✗] Failed: {data.decode('utf-8')}")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_tailf(self):
        """Test CMD_TAILF - Tail file continuously"""
        print("\n[TEST] tailf")

        try:
            test_file = os.path.join(self.test_dir, "tailf_test.txt")

            # Create test file
            with open(test_file, "w") as f:
                f.write("Initial content\n")

            # Start tailf
            req = RPCRequest(CMD_TAILF, test_file.encode("utf-8"))
            send_request(self.client_sock, req)

            # Collect output for 2 seconds
            import time
            import threading

            output_lines = []
            end_time = time.time() + 2.0

            def append_to_file():
                """Append to file after 1 second"""
                time.sleep(1.0)
                with open(test_file, "a") as f:
                    f.write("Appended line 1\n")
                    f.write("Appended line 2\n")

            # Start appending thread
            append_thread = threading.Thread(target=append_to_file)
            append_thread.start()

            # Collect responses for 2 seconds
            while time.time() < end_time:
                # Set short timeout to check if we should exit
                self.client_sock.settimeout(0.2)
                try:
                    status, data = recv_response(self.client_sock)

                    if status == STATUS_ERROR:
                        print(f"  [✗] Tailf error: {data.decode('utf-8')}")
                        self.failed += 1
                        return
                    elif status == STATUS_MORE_DATA:
                        # Accumulate output
                        text = data.decode("utf-8", errors="ignore")
                        if text.strip():
                            output_lines.append(text.strip())
                    elif status == STATUS_OK:
                        # Tailf ended naturally
                        break
                except socket.timeout:
                    # Continue waiting
                    continue

            # Reset timeout
            self.client_sock.settimeout(None)

            # Send cancel command
            cancel_req = RPCRequest(CMD_CANCEL, b"")
            send_request(self.client_sock, cancel_req)

            # Wait for cancel acknowledgment
            try:
                self.client_sock.settimeout(2.0)
                status, data = recv_response(self.client_sock)
                cancel_msg = data.decode("utf-8", errors="ignore")
                self.client_sock.settimeout(None)
            except socket.timeout:
                cancel_msg = "(no response)"
                self.client_sock.settimeout(None)

            # Wait for append thread
            append_thread.join()

            # Verify we got some output
            if len(output_lines) > 0:
                print(f"  [✓] Received {len(output_lines)} update(s):")
                for line in output_lines[:3]:  # Show first 3
                    print(f"      {line}")
                print(f"  [✓] Cancel acknowledged: {cancel_msg}")
                self.passed += 1
            else:
                print(f"  [✗] No output received from tailf")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            import traceback
            traceback.print_exc()
            self.failed += 1

    def test_timestomp(self):
        """Test CMD_TIMESTOMP - Modify file timestamps"""
        print("\n[TEST] timestomp")

        try:
            test_file = os.path.join(self.test_dir, "timestamp_test.txt")

            # Create test file
            with open(test_file, "w") as f:
                f.write("timestamp test")

            # Get original timestamps
            orig_stat = os.stat(test_file)

            # Set new timestamps (atime and mtime to Unix epoch + 1000000)
            new_atime = 1000000
            new_mtime = 2000000

            # Format: filepath atime mtime
            args = f"{test_file} {new_atime} {new_mtime}".encode("utf-8")
            req = RPCRequest(CMD_TIMESTOMP, args)
            send_request(self.client_sock, req)

            status, data = recv_response(self.client_sock)

            if status == STATUS_OK:
                print(f"  [✓] {data.decode('utf-8')}")

                # Verify timestamps were modified
                new_stat = os.stat(test_file)

                if int(new_stat.st_atime) == new_atime and int(new_stat.st_mtime) == new_mtime:
                    print(f"  [✓] Timestamps verified (atime={new_atime}, mtime={new_mtime})")
                    self.passed += 1
                else:
                    print(f"  [✗] Timestamp mismatch")
                    print(f"      Expected: atime={new_atime}, mtime={new_mtime}")
                    print(f"      Got: atime={int(new_stat.st_atime)}, mtime={int(new_stat.st_mtime)}")
                    self.failed += 1
            else:
                print(f"  [✗] Failed: {data.decode('utf-8')}")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_wc(self):
        """Test CMD_WC - Word count using mmap"""
        print("\n[TEST] wc")

        try:
            test_file = os.path.join(self.test_dir, "wc_test.txt")

            # Create test file with known content
            content = "hello world\nthis is a test\nthree lines total\n"
            with open(test_file, "w") as f:
                f.write(content)

            # Expected counts
            expected_lines = 3
            expected_words = 9  # hello(1) world(2) this(3) is(4) a(5) test(6) three(7) lines(8) total(9)
            expected_chars = len(content)

            req = RPCRequest(CMD_WC, test_file.encode("utf-8"))
            send_request(self.client_sock, req)

            status, data = recv_response(self.client_sock)

            if status == STATUS_OK:
                result = data.decode("utf-8")
                print(f"  [✓] Word count result:")
                for line in result.strip().split("\n"):
                    print(f"      {line}")

                # Parse the result
                lines = result.strip().split("\n")
                counts = {}
                for line in lines:
                    if ":" in line:
                        key, val = line.split(":", 1)
                        counts[key.strip()] = int(val.strip())

                # Verify counts
                if (counts.get("Lines") == expected_lines and
                    counts.get("Words") == expected_words and
                    counts.get("Chars") == expected_chars):
                    print(f"  [✓] Counts verified")
                    self.passed += 1
                else:
                    print(f"  [✗] Count mismatch")
                    print(f"      Expected: Lines={expected_lines}, Words={expected_words}, Chars={expected_chars}")
                    print(f"      Got: Lines={counts.get('Lines')}, Words={counts.get('Words')}, Chars={counts.get('Chars')}")
                    self.failed += 1
            else:
                print(f"  [✗] Failed: {data.decode('utf-8')}")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def test_proc_maps(self):
        """Test CMD_PROC_MAPS - Read /proc/self/maps"""
        print("\n[TEST] proc_maps")

        try:
            req = RPCRequest(CMD_PROC_MAPS, b"")
            send_request(self.client_sock, req)

            status, data = recv_response(self.client_sock)

            if status == STATUS_OK:
                maps = data.decode("utf-8")
                lines = maps.strip().split("\n")
                print(f"  [✓] Proc maps (first 3 lines):")
                for line in lines[:3]:
                    print(f"      {line}")

                # Should contain memory addresses
                if "-" in maps and len(lines) > 0:
                    print("  [✓] Valid maps format")
                    self.passed += 1
                else:
                    print("  [✗] Invalid maps format")
                    self.failed += 1
            else:
                print(f"  [✗] Failed: {data.decode('utf-8')}")
                self.failed += 1

        except Exception as e:
            print(f"  [✗] Exception: {e}")
            self.failed += 1

    def run_all_tests(self):
        """Run all test cases"""
        print("=" * 60)
        print("AGENT FUNCTIONALITY TEST SUITE")
        print("=" * 60)

        try:
            self.setup()
            self.start_listener()
            self.start_agent()
            self.accept_connection()

            # Basic file operations
            self.test_pwd()
            self.test_ls()
            self.test_stats()

            # File transfer
            self.test_download()
            self.test_upload()
            self.test_append()

            # File processing
            self.test_bgrep()
            self.test_djb2sum()
            self.test_sed()
            self.test_wc()

            # File manipulation
            self.test_timestomp()

            # Streaming operations
            self.test_tailf()

            # System operations
            self.test_proc_maps()

        finally:
            self.teardown()

        # Print summary
        print("\n" + "=" * 60)
        print("TEST SUMMARY")
        print("=" * 60)
        print(f"Passed: {self.passed}")
        print(f"Failed: {self.failed}")
        print(f"Total:  {self.passed + self.failed}")
        print("=" * 60)

        return self.failed == 0


def main():
    """Main entry point"""
    import argparse

    parser = argparse.ArgumentParser(description="Test agent functionality")
    parser.add_argument(
        "agent_bin", nargs="?", default=AGENT_BIN, help="Path to agent binary"
    )
    parser.add_argument(
        "--port", type=int, default=SERVER_PORT, help="Server port (default: 4444)"
    )
    parser.add_argument(
        "--strace", action="store_true", help="Run with qemu --strace (for debugging)"
    )

    args = parser.parse_args()

    print(f"Agent binary: {args.agent_bin}")
    print(f"Port: {args.port}")
    print(f"Strace: {args.strace}\n")

    tester = TestAgent(args.port, args.agent_bin, args.strace)
    success = tester.run_all_tests()

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
