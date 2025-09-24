# Irrigation Controller – ESP32 Firmware

Firmware for an ESP32-based irrigation system.  
It controls solenoid valves, reads soil moisture sensor data, and syncs with Firebase Realtime Database.

## Features
- Manual valve control (on/off)
- Scheduled automatic irrigation
- Soil moisture sensor readings
- Threshold-based irrigation logic
- Realtime sync with Firebase

## Hardware
- ESP32 Dev Board
- Relay/Solenoid valve
- Soil moisture sensor
- Power supply

## Firebase Structure
```json
{
  "valve1": true,
  "manual": false,
  "sensor": {
    "moisture": 450
  },
  "settings": {
    "interval_days": 2,
    "duration_minutes": 15,
    "time": "08:30",
    "threshold": 500
  },
  "last_irrigation": {
    "datetime": "2025-09-22T08:30:00",
    "duration": 15
  }
}
