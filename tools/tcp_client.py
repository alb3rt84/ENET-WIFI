#!/usr/bin/env python3
"""Prosty klient TCP do testu mostu ENET-WIFI (jak klient do ENET)."""

import argparse
import socket
import sys
import threading


def reader(sock: socket.socket) -> None:
    try:
        while True:
            data = sock.recv(4096)
            if not data:
                print("\n[disconnected]", flush=True)
                break
            sys.stdout.buffer.write(data)
            sys.stdout.buffer.flush()
    except OSError:
        pass


def main() -> int:
    p = argparse.ArgumentParser(description="TCP client for ENET-WIFI bridge")
    p.add_argument("host", help="IP płytki WT32-ETH01")
    p.add_argument("port", nargs="?", type=int, default=8080)
    args = p.parse_args()

    sock = socket.create_connection((args.host, args.port), timeout=5)
    sock.settimeout(None)
    print(f"Connected to {args.host}:{args.port} — type to send, Ctrl+C to quit",
          flush=True)
    t = threading.Thread(target=reader, args=(sock,), daemon=True)
    t.start()
    try:
        for line in sys.stdin.buffer:
            sock.sendall(line)
    except KeyboardInterrupt:
        print()
    finally:
        sock.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
