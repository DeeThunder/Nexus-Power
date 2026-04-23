# Smart Power Bank 🔋

A bare-metal firmware implementation for the **ESP32-C3** designed to control high-power sources with precision and efficiency.

## 🚀 Features

- **Bare-Metal Architecture**: Built on ESP-IDF for maximum performance and low-level hardware control.
- **Dual Framework Support**: Compatible with both **ESP-IDF** and **Arduino** frameworks for flexible development and testing.
- **Power Control Engine**: Encapsulated C++ logic for reliable GPIO switching.
- **Low Latency**: Direct register manipulation (where applicable) and optimized FreeRTOS tasks.

## 🛠 Hardware Configuration

| Component | Pin | Description |
|-----------|-----|-------------|
| Control GPIO | `GPIO 2` | Digital output for power source switching |
| MCU | ESP32-C3 | RISC-V Single-Core CPU |

## 📦 Project Structure

- `src/main.cpp`: Entry point handling the power cycle logic.
- `src/PowerController.cpp`: Implementation of the `PowerController` class.
- `include/PowerController.hpp`: Header file defining the power control interface.
- `platformio.ini`: PlatformIO configuration for multiple environments.

## 🚦 Getting Started

### Prerequisites
- [PlatformIO Core](https://platformio.org/install/cli) or VS Code with PlatformIO extension.

### Installation
1. Clone the repository.
2. Open the project in PlatformIO.
3. Select your preferred environment (`esp32-c3-arduino` or `esp32-c3-devkitm-1`).
4. Build and upload.

## 📅 Development Roadmap

- [x] Basic GPIO Power Toggling (5s Interval)
- [x] Dual-Framework Compatibility
- [ ] PWM-based Voltage Regulation
- [ ] Battery Level Monitoring
- [ ] Over-current Protection Logic

---
*Developed with precision for the next generation of power management.*
