# Esphome-c-api ESPHome C API (Encrypted Client)

A portable C client library for the encrypted ESPHome Native API, supporting:

- Noise-PSK handshake
- Encrypted transport
- Protobuf message encoding/decoding
- ESPHome API message framing
- Linux + ESP32-S3 (ESP-IDF)
- No MQTT, no Python required at runtime

This project aims to provide the first fully open-source C implementation of the
encrypted ESPHome API, enabling embedded systems, alarm panels, and industrial
controllers to communicate directly with ESPHome devices.

## Status

🚧 Early development  
✔ Project skeleton  
✔ Public headers  
✔ Example clients  
⬜ Noise handshake  
⬜ Encrypted transport  
⬜ Protobuf integration  
⬜ Entity discovery  
⬜ State subscriptions  
⬜ Commands (switch, light, sensor, etc.)

## Goals

- One codebase for Linux + ESP32-S3
- Minimal dependencies
- Clean modular architecture
- Community contributions encouraged

See `ROADMAP.md` for detailed milestones.
