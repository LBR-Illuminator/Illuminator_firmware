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
├── Drivers/              # Project specific drivers
│   ├── VAL/              # Vendor abstraction layer
│   └── HAL/              # STM32 hardware abstraction layer 
├── Middlewares/          # Third-party middleware
└── .gitignore            # Git ignore file
```

## Coding Style Guidelines

The project follows a modified version of ST's HAL coding style with the following conventions:

### File Organization
- Header files (.h) in `/Core/Inc` and `/Drivers/VAL/Inc`
- Source files (.c) in `/Core/Src` and `/Drivers/VAL/Src`
- All files must have a consistent header block with copyright notice and description

### Naming Conventions
- **Functions**: Module prefix with underscore separation (e.g., `VAL_Analog_Init`)
- **Constants**: ALL_CAPS with underscores (e.g., `PWM_CHANNEL_COUNT`)
- **Variables**: snake_case for local and static variables (e.g., `light_sensor_data`)
- **Types**: PascalCase with "_t" suffix (e.g., `LightSensorData_t`)
- **Enums**: PascalCase with "_t" suffix (e.g., `ErrorType_t`)

### Code Structure
- Two space indentation
- Opening braces on the same line for functions and control statements
- Closing braces on a separate line
- Space after keywords (if, for, while, switch)
- No space after function names in declarations or calls
- Space around operators (e.g., `a + b`, not `a+b`)

### Documentation
- Functions must be documented with:
  ```c
  /**
    * @brief  Brief description
    * @param  paramName: Parameter description
    * @retval Description of return value
    */
  ```
- Section headers should use:
  ```c
  /* Private define ------------------------------------------------------------*/
  ```

### Error Handling
- Functions should validate parameters at the beginning
- Use `VAL_Status` return values consistently
- Check status returns from function calls

### Header File Structure
- Include guards with `#ifndef __FILE_H`
- Group related functions and types together
- Include minimal necessary headers only

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

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- STMicroelectronics for the STM32 platform and development tools
- NimaLTD for the EEPROM emulation library