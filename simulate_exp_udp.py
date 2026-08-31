#!/usr/bin/env python3
"""
simulate_exp_udp.py
Simulates EXP_MESSAGE_ID (903) telemetry packets sent to BrahmaxisGIS over UDP port 8540.
Sends dynamic entity updates every 10 seconds.
"""

import socket
import struct
import time
import math
import random

# Target configuration
UDP_IP = "127.0.0.1"
UDP_PORT = 8540
INTERVAL_SECONDS = 10

# Base coordinates for telemetry sensors (Jammu & Kashmir / Sector Alpha region)
BASE_SENSORS = [
    {"id": 9001, "name": "EXP_Radar_Alpha",   "lat": 32.5347, "lon": 74.1987, "alt": 450.0, "radius": 0.05, "speed": 0.1},
    {"id": 9002, "name": "EXP_Thermal_Beta",  "lat": 32.6120, "lon": 74.3100, "alt": 320.0, "radius": 0.04, "speed": 0.15},
    {"id": 9003, "name": "EXP_Optic_Gamma",   "lat": 32.4800, "lon": 74.0500, "alt": 510.0, "radius": 0.06, "speed": 0.08},
    {"id": 9004, "name": "EXP_Acoustic_Delta","lat": 32.7000, "lon": 74.4500, "alt": 280.0, "radius": 0.03, "speed": 0.12},
    {"id": 9005, "name": "EXP_Drone_Epsilon", "lat": 32.4000, "lon": 74.2500, "alt": 850.0, "radius": 0.08, "speed": 0.20},
]

def build_udp_packet(seq_no, tick):
    num_entities = len(BASE_SENSORS)
    exp_header_size = 4
    entity_size = 128
    exp_payload_size = exp_header_size + (num_entities * entity_size)

    # 1. System Message Header (15 bytes packed)
    # STRUCT_MESSAGE_HEADER: source_id(H), dest_id(H), msg_id(H), msg_len(H), precedence(B), ws_index(B), sucomt_index(B), seq_no(H), no_packets(H)
    sys_header = struct.pack(
        "<HHHHBBBHH",
        1,                  # source_id
        2,                  # destination_id
        903,                # message_id (EXP_MESSAGE_ID)
        exp_payload_size,   # message_len
        0,                  # precedence
        0,                  # ws_index
        0,                  # sucomt_index
        seq_no,             # packet_seq_no
        1                   # no_of_packets
    )

    # 2. ExpMessageHeader (4 bytes int32)
    exp_header = struct.pack("<i", num_entities)

    # 3. ExpEntityStruct list
    entity_bytes = bytearray()
    for sensor in BASE_SENSORS:
        # Calculate dynamic orbital position based on tick
        angle = tick * sensor["speed"]
        lat = sensor["lat"] + (sensor["radius"] * math.sin(angle))
        lon = sensor["lon"] + (sensor["radius"] * math.cos(angle))
        alt = sensor["alt"] + (random.uniform(-5.0, 5.0))

        name_encoded = sensor["name"].encode("utf-8").ljust(100, b"\x00")[:100]
        entity_struct = struct.pack(
            "<i100sddd",
            sensor["id"],
            name_encoded,
            lat,
            lon,
            alt
        )
        entity_bytes.extend(entity_struct)

    return sys_header + exp_header + entity_bytes

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print(f"📡 EXP Telemetry Simulator started.")
    print(f"🎯 Target: UDP {UDP_IP}:{UDP_PORT} | Interval: {INTERVAL_SECONDS}s")
    
    seq_no = 1
    tick = 0
    try:
        while True:
            packet = build_udp_packet(seq_no, tick)
            sock.sendto(packet, (UDP_IP, UDP_PORT))
            print(f"[{time.strftime('%H:%M:%S')}] 📤 Sent EXP_MESSAGE_ID (903) packet #{seq_no} ({len(packet)} bytes, {len(BASE_SENSORS)} entities)")
            
            seq_no = (seq_no + 1) % 65535
            tick += 1
            time.sleep(INTERVAL_SECONDS)
    except KeyboardInterrupt:
        print("\n🛑 Simulator stopped by user.")
    finally:
        sock.close()

if __name__ == "__main__":
    main()
