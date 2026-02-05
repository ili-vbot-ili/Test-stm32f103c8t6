# Test-stm32f103c8t6

Project STM32F103C8T6 với thư viện W5500 Ethernet.

## Thông tin phần cứng

- **MCU**: STM32F103C8T6 (Blue Pill)
- **Core**: ARM Cortex-M3
- **Flash**: 64KB
- **RAM**: 20KB
- **Module Ethernet**: W5500

## Cấu trúc thư mục

```
├── CMakeLists.txt          # File cấu hình CMake chính
├── CMakePresets.json       # Presets cho CMake
├── stm32f103x8_flash.ld    # Linker script
├── Inc/                    # Header files
├── Src/                    # Source files
│   ├── main.cpp
│   ├── startup_stm32f103xx.S
│   ├── syscall.c
│   └── sysmem.c
├── Library/
│   └── W5500/              # Thư viện W5500 Ethernet
│       ├── w5500.c
│       ├── w5500.h
│       ├── wizchip_conf.c
│       └── wizchip_conf.h
└── cmake/
    ├── gnu-tools-for-stm32.cmake
    └── vscode_generated.cmake
```

## Yêu cầu

- [STM32CubeIDE for VS Code Extension](https://marketplace.visualstudio.com/items?itemName=stmicroelectronics.stm32-vscode-extension)
- GNU Arm Embedded Toolchain
- CMake >= 3.20
- Ninja Build System

## Build

### Sử dụng CMake Presets

```bash
# Configure (Debug)
cmake --preset Debug

# Build
cmake --build build/Debug
```

### Sử dụng VS Code

1. Mở project trong VS Code
2. Chọn preset "Debug" hoặc "Release"
3. Build project (Ctrl+Shift+B)

## Output

Sau khi build thành công, các file output sẽ được tạo trong `build/Debug/`:

- `Test-stm32f103c6t8.elf` - File ELF
- `Test-stm32f103c6t8.hex` - File HEX cho flash
- `Test-stm32f103c6t8.bin` - File Binary
- `Test-stm32f103c6t8.map` - Memory map

## History
- Tạo project mới với stm32f103c6t8 thêm thư viện w5500, fix lỗi khai báo thư viện w5500

## License

Copyright (c) 2025 STMicroelectronics. All rights reserved.
