# Illuminator Firmware

## Project Overview

This repository contains the embedded firmware for the LBR Illuminator device, an advanced lighting system with multiple independently controlled light sources. The firmware runs on an STM32L432KC microcontroller and provides comprehensive control, monitoring, and safety features.

## Features

- Control of three independent light sources with PWM intensity adjustment
- Real-time monitoring of current and temperature for each light source
- Automatic safety cutoffs to prevent damage from over-current or over-temperature conditions
- Error logging with persistent storage across power cycles
- JSON-based communication protocol over serial interface
- Support for remote control via companion PC application

## Hardware Requirements

- STM32L432KC microcontroller (NUCLEO-L432KC development board for testing)
- Three light sources with associated drivers
- Current and temperature sensors for each light
- Serial communication interface (UART)

## Development Environment

- STM32CubeIDE (recommended)
- STM32CubeMX for hardware configuration
- Git for version control

## Project Structure

```
illuminator-firmware/
├── Core/                 # Core application code
│   ├── Inc/              # Header files
│   └── Src/              # Source files
├── Drivers/              # STM32 HAL drivers
├── Middlewares/          # Third-party middleware
└── .gitignore            # Git ignore file
```

## Getting Started

### Prerequisites

1. Install STM32CubeIDE
2. Clone this repository

### Building the Project

1. Open the project in STM32CubeIDE
2. Build the project by clicking the hammer icon or pressing Ctrl+B

### Flashing the Firmware

1. Connect your NUCLEO-L432KC board via USB
2. In STM32CubeIDE, click the "Run" button or press F11

## Communication Protocol

The firmware implements a JSON-based communication protocol over UART (115200 bps, 8N1). The protocol supports:

- Setting light intensity (0-100%) for individual or all lights
- Querying current status including intensity levels
- Reading sensor data (current and temperature)
- Retrieving and clearing error logs

Detailed protocol documentation is available in the `docs` directory.

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- STMicroelectronics for the STM32 platform and development tools
- NimaLTD for the EEPROM emulation library