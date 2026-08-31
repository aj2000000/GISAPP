#!/usr/bin/env python3
"""
Simulate UDP Message for Sensor Telemetry
=========================================
Sends binary telemetry UDP packets (Message ID: 902) to BrahmaxisGIS application.

Packet Structure:
  - STRUCT_MESSAGE_HEADER (20 bytes, packed)
  - SensorMessageHeader (4 bytes, packed: numberOfEntity)
  - SensorStruct[] (packed: id, name[100], sensorType, sensorHeight, sensorCoverage, sensorAxis, sensorBearing, source, loc(lat, lon, alt), deployedTime(day, month, year, hour, min, sec))
"""

import socket
import struct
import time
import argparse
import random

# Protocol Constants
SENSOR_MSG_ID = 902
DEFAULT_TARGET_IP = "127.0.0.1"
DEFAULT_TARGET_PORT = 8540

# Base Sensor Entities Data
INITIAL_SENSORS = [
    {
        "id": 2001,
        "name": "EOTS Optronic Array Alpha",
        "sensorType": 1,
        "sensorHeight": 15,
        "sensorCoverage": 120,
        "sensorAxis": 0,
        "sensorBearing": 45,
        "source": 10,
        "lat": 32.5500,
        "lon": 74.1500,
        "alt": 180.0
    },
    {
        "id": 2002,
        "name": "Thermal Imaging Mast Bravo",
        "sensorType": 2,
        "sensorHeight": 25,
        "sensorCoverage": 360,
        "sensorAxis": 0,
        "sensorBearing": 180,
        "source": 11,
        "lat": 32.6200,
        "lon": 74.3500,
        "alt": 210.0
    },
    {
        "id": 2003,
        "name": "Acoustic Surveillance Node Delta",
        "sensorType": 3,
        "sensorHeight": 5,
        "sensorCoverage": 90,
        "sensorAxis": 0,
        "sensorBearing": 270,
        "source": 12,
        "lat": 32.4200,
        "lon": 73.8500,
        "alt": 150.0
    },
    {
        "id": 2004,
        "name": "Perimeter Radar Sensor Echo",
        "sensorType": 4,
        "sensorHeight": 30,
        "sensorCoverage": 180,
        "sensorAxis": 0,
        "sensorBearing": 90,
        "source": 13,
        "lat": 32.7500,
        "lon": 74.6000,
        "alt": 320.0
    },
    {
        "id": 2005,
        "name": "Seismic Array Station Foxtrot",
        "sensorType": 5,
        "sensorHeight": 2,
        "sensorCoverage": 360,
        "sensorAxis": 0,
        "sensorBearing": 0,
        "source": 14,
        "lat": 32.3500,
        "lon": 74.8000,
        "alt": 190.0
    },
    {
        "id": 2006,
        "name": "Long-Range IR Surveillance Golf",
        "sensorType": 2,
        "sensorHeight": 40,
        "sensorCoverage": 120,
        "sensorAxis": 0,
        "sensorBearing": 135,
        "source": 15,
        "lat": 32.8200,
        "lon": 73.6500,
        "alt": 450.0
    },
    {
        "id": 2007,
        "name": "High-Altitude Optical Turret Hotel",
        "sensorType": 1,
        "sensorHeight": 20,
        "sensorCoverage": 240,
        "sensorAxis": 0,
        "sensorBearing": 315,
        "source": 16,
        "lat": 32.4800,
        "lon": 75.1000,
        "alt": 280.0
    },
    {
        "id": 2008,
        "name": "Border SIGINT Intercept Tower India",
        "sensorType": 6,
        "sensorHeight": 50,
        "sensorCoverage": 360,
        "sensorAxis": 0,
        "sensorBearing": 0,
        "source": 17,
        "lat": 32.6800,
        "lon": 73.4000,
        "alt": 350.0
    },
    {
        "id": 2009,
        "name": "Forward Air Guard Sensor Juliet",
        "sensorType": 4,
        "sensorHeight": 18,
        "sensorCoverage": 180,
        "sensorAxis": 0,
        "sensorBearing": 60,
        "source": 18,
        "lat": 32.2500,
        "lon": 74.4500,
        "alt": 220.0
    },
    {
        "id": 2010,
        "name": "Tactical Multi-Sensor Pod Kilo",
        "sensorType": 1,
        "sensorHeight": 12,
        "sensorCoverage": 360,
        "sensorAxis": 0,
        "sensorBearing": 210,
        "source": 19,
        "lat": 32.5900,
        "lon": 74.9000,
        "alt": 260.0
    }
]

def pack_sensor_struct(sensor: dict) -> bytes:
    """Packs a single SensorStruct into bytes."""
    name_bytes = sensor["name"].encode('utf-8')[:99] # max 100 chars (null padded)
    
    # STRUCT_DATE_TIME: day(B), month(B), year(H), hour(B), minute(B), second(B)
    day, month, year, hour, minute, second = 31, 8, 2026, 21, 25, 0

    return struct.pack(
        '<i100sBiiiiidddBBHBBB',
        sensor["id"],
        name_bytes,
        sensor["sensorType"],
        sensor["sensorHeight"],
        sensor["sensorCoverage"],
        sensor["sensorAxis"],
        sensor["sensorBearing"],
        sensor["source"],
        sensor["lat"],
        sensor["lon"],
        sensor["alt"],
        day, month, year, hour, minute, second
    )

def pack_udp_message(sensors: list, seq_no: int = 1) -> bytes:
    """Packs complete UDP datagram: Header + MsgHeader + Sensor List."""
    num_entities = len(sensors)
    
    # 1. Pack Sensor Array Body
    entity_body = b''.join([pack_sensor_struct(s) for s in sensors])
    
    # 2. Pack SensorMessageHeader (4 bytes: numberOfEntity)
    msg_header = struct.pack('<i', num_entities)
    
    body_payload = msg_header + entity_body
    body_length = len(body_payload)
    
    # 3. Pack Outer STRUCT_MESSAGE_HEADER
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
        SENSOR_MSG_ID,
        body_length,
        precedence,
        ws_index,
        sucomt_index,
        seq_no,
        no_of_packets
    )
    
    return outer_header + body_payload

def main():
    parser = argparse.ArgumentParser(description="Simulate UDP Telemetry Packets for Sensor (ID: 902)")
    parser.add_argument("--ip", default=DEFAULT_TARGET_IP, help="Target UDP IP address (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=DEFAULT_TARGET_PORT, help="Target UDP Port (default: 8540)")
    parser.add_argument("--once", action="store_true", help="Send only a single packet and exit")
    parser.add_argument("--interval", type=float, default=2.0, help="Interval in seconds for continuous mode")
    
    args = parser.parse_args()
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    print("==========================================================")
    print(" 📡 BrahmaxisGIS UDP Telemetry Simulator (Sensor)")
    print(f" 🎯 Target: {args.ip}:{args.port}")
    print(f" 📦 Message ID: {SENSOR_MSG_ID}")
    print("==========================================================\n")
    
    sensors = INITIAL_SENSORS
    seq_no = 1
    
    try:
        while True:
            packet = pack_udp_message(sensors, seq_no)
            sock.sendto(packet, (args.ip, args.port))
            
            timestamp = time.strftime("%H:%M:%S")
            print(f"[{timestamp}] 📤 Sent Datagram #{seq_no} ({len(packet)} bytes) to {args.ip}:{args.port}")
            for s in sensors:
                print(f"   • ID: {s['id']} | Name: {s['name']:<28} | Lat: {s['lat']:.5f} | Lon: {s['lon']:.5f} | Alt: {s['alt']}m")
            
            if args.once:
                break
                
            # Randomize movement slightly for live dynamic updates on map
            for s in sensors:
                s["lat"] += random.uniform(0.001, 0.005)
                s["lon"] += random.uniform(0.001, 0.005)
                s["sensorBearing"] = (s["sensorBearing"] + 5) % 360
                
            seq_no += 1
            time.sleep(args.interval)
            
    except KeyboardInterrupt:
        print("\n[!] Simulation stopped by user.")
    finally:
        sock.close()

if __name__ == "__main__":
    main()
