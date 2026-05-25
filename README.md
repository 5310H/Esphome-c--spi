# ESPHome C API

A lightweight C implementation of the ESPHome native API protocol, designed for embedded systems. This project supports encrypted communication using Libsodium and uses Nanopb for efficient Protocol Buffer handling.

## Features

- **Nanopb Integration**: Minimal memory footprint for Protobuf messages.
- **Encrypted Communication**: Implements the ESPHome encryption layer using ChaCha20-Poly1305 (via Libsodium).
- **Frame-based Protocol**: Custom framing with 4-byte message types and 2-byte length headers.
- **Fake Device Simulator**: Includes a `fake_esphome_device` for testing the handshake and encrypted message exchange locally.

## Prerequisites

- `gcc`
- `make`
- `libsodium-dev`

## Building

To build both the client and the fake device simulator:

```bash
make
```

## Usage

1. **Start the Fake Device**:
   In one terminal, run the simulator:
   ```bash
   ./fake_esphome_device
   ```

2. **Run the Client**:
   In another terminal, connect to the simulator (or a real ESPHome device):
   ```bash
   ./esphome_client <ip_address> [port] [api_key]
   ```
   Example:
   ```bash
   ./esphome_client 127.0.0.1 6053 123456789
   ```

## Contributing
Refer to CONTRIBUTING.md for guidelines on how to help improve this implementation.

*Last Updated: 2026-05-24 20:45 EST*