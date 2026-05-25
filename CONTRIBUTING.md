# Contributing

Contributions are welcome!

This project aims to build a clean, portable C implementation of the encrypted
ESPHome API. Help is appreciated in all areas — from Noise handshake work to
protobuf integration, ESP32 support, and documentation.

## How to help

- Implement missing API features
- Refine the internal Nanopb implementation (alignment and pointer safety)
- Improve Noise-PSK handshake correctness
- Add tests for message framing and encryption
- Add ESP32 examples or ESP-IDF integration
- Improve documentation and comments
- Review pull requests

## Development workflow

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request
5. Include tests when possible

## Coding style

- C99
- Keep modules small and focused
- Avoid global state
- Avoid dynamic allocation on ESP32 where possible
- Prefer clear, explicit code over clever code

## Communication

Open an issue if you want to discuss a feature or design change.

*Last Updated: 2026-05-24 21:55 EST*
