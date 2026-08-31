#!/usr/bin/env python3
"""
Simulate UDP Message for SampleEntity Telemetry
================================================
Sends binary telemetry UDP packets (Message ID: 901) to BrahmaxisGIS application.

Packet Structure:
  - STRUCT_MESSAGE_HEADER (20 bytes, packed)
  - SampleEntityMessageHeader (4 bytes, packed: numberOfEntity)
  - SampleEntityStruct[] (128 bytes each, packed: id, name[100], lat, lon, alt)
"""

import socket
import struct
import time
import argparse
import random

# Protocol Constants
SAMPLE_ENTITY_MSG_ID = 901
DEFAULT_TARGET_IP = "127.0.0.1"
DEFAULT_TARGET_PORT = 8540

# Base Sample Entities Data
INITIAL_ENTITIES = [
    {"id": 1001, "name": "Tactical Radar Alpha", "lat": 28.6139, "lon": 77.2090, "alt": 250.0},
    {"id": 1002, "name": "Recon Drone Unit 7",  "lat": 28.6200, "lon": 77.2150, "alt": 500.5},
    {"id": 1003, "name": "Command Vessel Bravo", "lat": 28.6250, "lon": 77.2200, "alt": 120.0},
    {"id": 1004, "name": "Ground Station West",  "lat": 28.6100, "lon": 77.1950, "alt": 310.2},
]

def pack_sample_entity(entity_id: int, name: str, lat: float, lon: float, alt: float) -> bytes:
    """Packs a single SampleEntityStruct into 128 bytes."""
    name_bytes = name.encode('utf-8')[:99] # max 100 chars (null padded)
    return struct.pack('<i100sddd', entity_id, name_bytes, lat, lon, alt)

def pack_udp_message(entities: list, seq_no: int = 1) -> bytes:
    """Packs complete UDP datagram: Header + MsgHeader + Entity List."""
    num_entities = len(entities)
    
    # 1. Pack Sample Entity Array Body
    entity_body = b''.join([
        pack_sample_entity(e["id"], e["name"], e["lat"], e["lon"], e["alt"])
        for e in entities
    ])
    
    # 2. Pack SampleEntityMessageHeader (4 bytes: numberOfEntity)
    msg_header = struct.pack('<i', num_entities)
    
    body_payload = msg_header + entity_body
    body_length = len(body_payload)
    
    # 3. Pack Outer STRUCT_MESSAGE_HEADER (15 bytes)
    # Fields: source_id(H), destination_id(H), message_id(H), message_len(H), precedence(B), ws_index(B), sucomt_index(B), packet_seq_no(H), no_of_packets(H)
    source_id = 1
    destination_id = 2
    precedence = 0
    ws_index = 0
    sucomt_index = 0
    no_of_packets = 1
    
    outer_header = struct.pack(
        '<HHHHBBBHH',
        source_id,
        destination_id,
        SAMPLE_ENTITY_MSG_ID,
        body_length,
        precedence,
        ws_index,
        sucomt_index,
        seq_no,
        no_of_packets
    )
    
    return outer_header + body_payload

def main():
    parser = argparse.ArgumentParser(description="Simulate UDP Telemetry Packets for SAMPLE_ENTITY (ID: 901)")
    parser.add_argument("--ip", default=DEFAULT_TARGET_IP, help="Target UDP IP address (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=DEFAULT_TARGET_PORT, help="Target UDP Port (default: 8540)")
    parser.add_argument("--count", type=int, default=len(INITIAL_ENTITIES), help="Number of entities to send")
    parser.add_argument("--once", action="store_true", help="Send only a single packet and exit")
    parser.add_argument("--interval", type=float, default=2.0, help="Interval in seconds for continuous mode")
    
    args = parser.parse_args()
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    print("==========================================================")
    print(" 🚀 BrahmaxisGIS UDP Telemetry Simulator (SampleEntity)")
    print(f" 📡 Target: {args.ip}:{args.port}")
    print(f" 📦 Message ID: {SAMPLE_ENTITY_MSG_ID}")
    print("==========================================================\n")
    
    entities = INITIAL_ENTITIES[:args.count]
    seq_no = 1
    
    try:
        while True:
            packet = pack_udp_message(entities, seq_no)
            sock.sendto(packet, (args.ip, args.port))
            
            print(f"[{time.strftime('%H:%M:%S')}] 📤 Sent Datagram #{seq_no} ({len(packet)} bytes) to {args.ip}:{args.port}")
            for e in entities:
                print(f"   • ID: {e['id']} | Name: {e['name']:<22} | Lat: {e['lat']:.5f} | Lon: {e['lon']:.5f} | Alt: {e['alt']}m")
            
            if args.once:
                break
                
            # Simulate movement for continuous stream (increase lat/lon randomly between 0.1 and 0.5)
            for e in entities:
                e["lat"] += random.uniform(0.1, 0.5)
                e["lon"] += random.uniform(0.1, 0.5)
                e["alt"] += random.uniform(-2.0, 2.0)
                
            seq_no += 1
            time.sleep(args.interval)
            
    except KeyboardInterrupt:
        print("\n🛑 Simulation stopped by user.")
    finally:
        sock.close()
        print("Done.")

if __name__ == "__main__":
    main()
