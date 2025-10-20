
<p align="center"><img alt="espDragonFruit logo" src="https://github.com/Alexxdal/espDragonFruit/blob/master/data/assets/logo.svg?raw=true" width="300"></p>
<p align="center">

# espDragonFruit — Embedded Pentest Platform

## Short Description
- **espDragonFruit** is a compact, multi-radio penetration testing platform based on ESP32-family modules. It brings together multiple radios and a browser-based control interface to support research, auditing and defensive testing of wireless and networked systems.

**Important:** intended for authorized security testing, research and defensive use only. Obtain explicit permission before interacting with any systems you do not own. Misuse may be illegal.

## Key features
- Multi-radio hardware: two **ESP32-S3 N16R8** modules, one **ESP-WROOM-32**, and one **ESP32-C5**.
- Master device (ESP32-S3) hosts a web-based control interface for configuration and operation.
- All four modules communicate over an SPI bus for coordination and payload distribution.
- Protocol support (for authorized testing and research):
    - Wi‑Fi (monitoring, testing, and auditing modes)
    - Bluetooth Classic and Bluetooth Low Energy (BLE)
    - Thread
    - Zigbee
- Network-level utilities for testing and assessment (examples): port scanning, service discovery and other non-destructive network tests.
- Modular architecture to allow adding new analysis modules, protocol parsers and test plugins.

## Hardware
- 2× ESP32-S3 (N16R8)
- 1× ESP32-WROOM-32 (N8)
- 1× ESP32-C5 (N8R4)
- SPI bus connecting all modules
- Master device exposes a web UI (HTTP) for user interaction

Plentry of RAM and FLASH to work with.

## Architecture overview
- Master (ESP32-S3): web-based UI, command orchestration, logging and configuration.
- Workers (ESP32-S3, ESP32-WROOM-32, ESP32-C5): radio-specific handlers and DUT-specific functionality.
- SPI bus for low-latency messaging and coordination between master and worker modules.
- Plugin layer to add protocol handlers and network tests without changing core firmware.

Usage (high-level)
- Flash firmware to the devices (firmware images and flashing instructions maintained in the repo).
- Start the master and connect to its web UI from a browser on the same network.
- Use the UI to enable/disable radios, select permitted tests, and collect logs and results.
- Follow all legal and organizational policies before running any tests.

## Security & ethics
- Use only on systems for which you have explicit authorization.
- Avoid destructive testing unless explicitly permitted and scheduled.
- Keep firmware and tooling up to date; avoid using the platform to harm infrastructure or violate privacy.

## Contribution
- Contributions are welcome. Open an issue to discuss features or file a pull request with focused changes.
- Provide tests and documentation for new modules or protocol handlers.

## Documentation
- See the docs/ directory (or the project wiki) for flashing steps, API details and module development guidelines (TODO).
- Detailed test procedures and module-specific operation are documented separately; readers must follow the legal/ethical guidelines before use (TODO).

## License
- Refer to LICENSE file in the repository for full licensing information.

## Contact
- For project questions and contribution coordination, open an issue in this repository.
