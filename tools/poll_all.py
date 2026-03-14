#!/usr/bin/env python3

import argparse
import json
import sys
import time

import serial


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Poll the IOFusion reference firmware with all? once per interval."
    )
    parser.add_argument("port", help="Serial port, for example /dev/ttyACM0 or COM4")
    parser.add_argument(
        "--baud",
        type=int,
        default=115200,
        help="Serial baud rate (default: 115200)",
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=1.0,
        help="Polling interval in seconds (default: 1.0)",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=1.0,
        help="Serial read timeout in seconds (default: 1.0)",
    )
    parser.add_argument(
        "--pretty",
        action="store_true",
        help="Pretty-print JSON responses instead of single-line output.",
    )
    return parser.parse_args()


def read_response(ser: serial.Serial) -> str:
    line = ser.readline()
    if not line:
        return ""
    return line.decode("utf-8", errors="replace").strip()


def main() -> int:
    args = parse_args()

    try:
        with serial.Serial(args.port, args.baud, timeout=args.timeout) as ser:
            ser.reset_input_buffer()
            ser.reset_output_buffer()

            print(
                f"Polling {args.port} at {args.baud} baud every {args.interval:.3f} s. Press Ctrl+C to stop.",
                flush=True,
            )

            next_poll = time.monotonic()
            while True:
                now = time.monotonic()
                if now < next_poll:
                    time.sleep(next_poll - now)
                next_poll += args.interval

                ser.write(b"all?\n")
                ser.flush()
                response = read_response(ser)
                timestamp = time.strftime("%Y-%m-%d %H:%M:%S")

                if not response:
                    print(f"[{timestamp}] timeout waiting for response", flush=True)
                    continue

                try:
                    payload = json.loads(response)
                except json.JSONDecodeError:
                    print(f"[{timestamp}] {response}", flush=True)
                    continue

                if args.pretty:
                    print(f"[{timestamp}]", flush=True)
                    print(json.dumps(payload, indent=2, sort_keys=True), flush=True)
                else:
                    print(
                        f"[{timestamp}] {json.dumps(payload, separators=(',', ':'))}",
                        flush=True,
                    )
    except KeyboardInterrupt:
        print("Stopped.", flush=True)
        return 0
    except serial.SerialException as exc:
        print(f"Serial error: {exc}", file=sys.stderr, flush=True)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())