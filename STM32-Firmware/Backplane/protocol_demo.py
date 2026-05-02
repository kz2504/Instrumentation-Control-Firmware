#!/usr/bin/env python3
"""
Small pyserial smoke test for the STM32 USB CDC protocol.

Usage:
    python protocol_demo.py COM7
"""

from __future__ import annotations

import struct
import sys
import time
from dataclasses import dataclass

import serial

SOF = 0xA5
VERSION = 0x01
MAX_PAYLOAD = 256

MSG_PING = 0x01
MSG_ACK = 0x02
MSG_ERROR = 0x03
MSG_CLEAR_SCHEDULE = 0x10
MSG_UPLOAD_EVENT = 0x11
MSG_START_SCHEDULE = 0x12
MSG_STOP_SCHEDULE = 0x13
MSG_GET_STATUS = 0x14

MODULE_PUMP_PERISTALTIC = 0x01
MODULE_GPIO_FPGA = 0x02
PUMP_SET_STATE = 0x01
GPIO_SET_WAVEFORM = 0x01


def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def build_frame(msg_type: int, seq: int, payload: bytes = b"") -> bytes:
    if len(payload) > MAX_PAYLOAD:
        raise ValueError("payload too large")

    body = struct.pack("<BBHH", VERSION, msg_type, seq, len(payload)) + payload
    crc = crc16_ccitt_false(body)
    return bytes([SOF]) + body + struct.pack("<H", crc)


@dataclass
class Frame:
    msg_type: int
    seq: int
    payload: bytes


class FrameReader:
    def __init__(self) -> None:
        self.buffer = bytearray()

    def feed(self, data: bytes) -> list[Frame]:
        self.buffer.extend(data)
        frames: list[Frame] = []

        while True:
            try:
                sof_index = self.buffer.index(SOF)
            except ValueError:
                self.buffer.clear()
                break

            if sof_index:
                del self.buffer[:sof_index]

            if len(self.buffer) < 9:
                break

            version = self.buffer[1]
            msg_type = self.buffer[2]
            seq = struct.unpack_from("<H", self.buffer, 3)[0]
            payload_len = struct.unpack_from("<H", self.buffer, 5)[0]

            if version != VERSION or payload_len > MAX_PAYLOAD:
                del self.buffer[0]
                continue

            frame_len = 9 + payload_len
            if len(self.buffer) < frame_len:
                break

            payload = bytes(self.buffer[7 : 7 + payload_len])
            crc_rx = struct.unpack_from("<H", self.buffer, 7 + payload_len)[0]
            crc_calc = crc16_ccitt_false(bytes(self.buffer[1 : 7 + payload_len]))
            if crc_rx == crc_calc:
                frames.append(Frame(msg_type=msg_type, seq=seq, payload=payload))

            del self.buffer[:frame_len]

        return frames


def build_upload_event(event_id: int, timestamp_us: int, actions: list[bytes]) -> bytes:
    payload = struct.pack("<IQB", event_id, timestamp_us, len(actions))
    payload += b"".join(actions)
    return payload


def build_action(module_type: int, module_id: int, action_type: int, action_payload: bytes) -> bytes:
    return struct.pack("<BBBB", module_type, module_id, action_type, len(action_payload)) + action_payload


def pump_action(module_id: int, enable: int, direction: int, flow_nl_min: int) -> bytes:
    payload = struct.pack("<BBHI", enable, direction, 0, flow_nl_min)
    return build_action(MODULE_PUMP_PERISTALTIC, module_id, PUMP_SET_STATE, payload)


def gpio_waveform_action(module_id: int, polarity: int, idle_state: int, frequency_milli_hz: int, duty_ppm: int, num_periods: int) -> bytes:
    payload = struct.pack("<BBHIII", polarity, idle_state, 0, frequency_milli_hz, duty_ppm, num_periods)
    return build_action(MODULE_GPIO_FPGA, module_id, GPIO_SET_WAVEFORM, payload)


def decode_frame(frame: Frame) -> str:
    if frame.msg_type == MSG_ACK and len(frame.payload) == 3:
        ack_seq, status = struct.unpack("<HB", frame.payload)
        return f"ACK seq={frame.seq} ack_seq={ack_seq} status=0x{status:02X}"

    if frame.msg_type == MSG_ERROR and len(frame.payload) == 4:
        failed_seq, error_code, detail = struct.unpack("<HBB", frame.payload)
        return f"ERROR seq={frame.seq} failed_seq={failed_seq} code=0x{error_code:02X} detail=0x{detail:02X}"

    if frame.msg_type == MSG_GET_STATUS and len(frame.payload) == 16:
        scheduler_state, last_error, event_count, last_event_id, current_time_us = struct.unpack("<BBHIQ", frame.payload)
        return (
            "STATUS "
            f"seq={frame.seq} state={scheduler_state} last_error=0x{last_error:02X} "
            f"event_count={event_count} last_event_id={last_event_id} "
            f"time_us={current_time_us}"
        )

    return f"FRAME type=0x{frame.msg_type:02X} seq={frame.seq} payload={frame.payload.hex()}"


def send_and_print(port: serial.Serial, reader: FrameReader, msg_type: int, seq: int, payload: bytes = b"", expected_frames: int = 1) -> None:
    port.write(build_frame(msg_type, seq, payload))
    deadline = time.time() + 1.0
    frames_seen = 0

    while time.time() < deadline and frames_seen < expected_frames:
        data = port.read(port.in_waiting or 1)
        if not data:
            continue

        for frame in reader.feed(data):
            print(decode_frame(frame))
            if frame.seq == seq:
                frames_seen += 1

    if frames_seen < expected_frames:
        print(f"timeout waiting for response to seq={seq}")


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: python protocol_demo.py COM7")
        return 1

    seq = 1
    reader = FrameReader()

    with serial.Serial(sys.argv[1], 115200, timeout=0.1) as port:
        time.sleep(0.2)

        send_and_print(port, reader, MSG_PING, seq)
        seq += 1

        send_and_print(port, reader, MSG_CLEAR_SCHEDULE, seq)
        seq += 1

        pump_event = build_upload_event(
            event_id=1,
            timestamp_us=1000,
            actions=[pump_action(module_id=0, enable=1, direction=0, flow_nl_min=500000)],
        )
        send_and_print(port, reader, MSG_UPLOAD_EVENT, seq, pump_event)
        seq += 1

        gpio_event = build_upload_event(
            event_id=2,
            timestamp_us=2000,
            actions=[gpio_waveform_action(module_id=0, polarity=0, idle_state=0, frequency_milli_hz=1000, duty_ppm=500000, num_periods=10)],
        )
        send_and_print(port, reader, MSG_UPLOAD_EVENT, seq, gpio_event)
        seq += 1

        send_and_print(port, reader, MSG_START_SCHEDULE, seq)
        seq += 1

        send_and_print(port, reader, MSG_GET_STATUS, seq, expected_frames=2)
        seq += 1

        send_and_print(port, reader, MSG_STOP_SCHEDULE, seq)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
