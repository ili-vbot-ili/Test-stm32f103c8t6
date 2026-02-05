# Test-stm32f103c8t6

Project STM32F103C8T6 với thư viện W5500 Ethernet.

## Thông tin phần cứng

- **MCU**: STM32F103C8T6 (Blue Pill)
- **Core**: ARM Cortex-M3
- **Flash**: 64KB
- **RAM**: 20KB
- **Module Ethernet**: W5500
- **LCD**: LCD 1602 với I2C adapter (PCF8574)

## Kết nối phần cứng

### 1. W5500 Ethernet Module (SPI1)

| W5500 Pin | STM32F103C8T6 Pin | Chức năng |
|-----------|-------------------|-----------|
| VCC       | 3.3V              | Nguồn     |
| GND       | GND               | Mass      |
| MISO      | PA6 (SPI1_MISO)   | Master In Slave Out |
| MOSI      | PA7 (SPI1_MOSI)   | Master Out Slave In |
| SCK       | PA5 (SPI1_SCK)    | Serial Clock |
| CS        | PA4 (SPI1_NSS)    | Chip Select |
| RST       | PB0 (hoặc 3.3V)   | Reset (tùy chọn) |
| INT       | PB1 (tùy chọn)    | Interrupt (tùy chọn) |

### Sơ đồ kết nối

```
STM32F103C8T6                    W5500 Module
    ┌─────────┐                  ┌─────────┐
    │     3.3V├──────────────────┤VCC      │
    │      GND├──────────────────┤GND      │
    │      PA5├──────────────────┤SCK      │
    │      PA6├──────────────────┤MISO     │
    │      PA7├──────────────────┤MOSI     │
    │      PA4├──────────────────┤CS       │
    │      PB0├──────────────────┤RST      │
    │      PB1├──────────────────┤INT      │
    └─────────┘                  └─────────┘
```

### Lưu ý
- W5500 hoạt động ở mức điện áp **3.3V** (tương thích với STM32)
- Tốc độ SPI khuyến nghị: **<=33MHz** (W5500 hỗ trợ tối đa 80MHz)
- Nếu không dùng interrupt, có thể bỏ qua chân INT
- Chân RST có thể nối trực tiếp lên 3.3V nếu không cần hardware reset

### 2. LCD 1602 I2C Module (I2C1)

| LCD I2C Pin | STM32F103C8T6 Pin | Chức năng |
|-------------|-------------------|-----------|
| VCC         | 5V                | Nguồn (LCD cần 5V) |
| GND         | GND               | Mass      |
| SDA         | PB7 (I2C1_SDA)    | Data      |
| SCL         | PB6 (I2C1_SCL)    | Clock     |

#### Sơ đồ kết nối LCD

```
STM32F103C8T6                    LCD 1602 I2C
    ┌─────────┐                  ┌─────────┐
    │       5V├──────────────────┤VCC      │
    │      GND├──────────────────┤GND      │
    │      PB6├──────────────────┤SCL      │
    │      PB7├──────────────────┤SDA      │
    └─────────┘                  └─────────┘
```

#### Lưu ý LCD I2C
- **Địa chỉ I2C mặc định**: `0x27` hoặc `0x3F` (tùy module PCF8574/PCF8574A)
- LCD cần nguồn **5V**, nhưng I2C của STM32 là 5V tolerant
- Có thể cần điều chỉnh **biến trở** trên module I2C để chỉnh độ tương phản
- Tốc độ I2C khuyến nghị: **100kHz** (Standard mode) hoặc **400kHz** (Fast mode)

## Cấu trúc thư mục

```
├── CMakeLists.txt          # File cấu hình CMake chính
├── CMakePresets.json       # Presets cho CMake
├── stm32f103x8_flash.ld    # Linker script
├── Inc/                    # Header files
│   └── w5500_conf.h        # Cấu hình W5500
├── Src/                    # Source files
│   ├── main.cpp
│   ├── startup_stm32f103xx.S
│   ├── syscall.c
│   ├── sysmem.c
│   └── w5500_conf.c        # Implementation W5500
├── Library/
│   ├── W5500/              # Thư viện W5500 Ethernet
│   │   ├── w5500.c
│   │   ├── w5500.h
│   │   ├── socket.c
│   │   ├── socket.h
│   │   ├── wizchip_conf.c
│   │   └── wizchip_conf.h
│   └── lcd1602/            # Thư viện LCD 1602 I2C
│       ├── lcd_i2c.c
│       └── lcd_i2c.h
└── cmake/
    ├── gnu-tools-for-stm32.cmake
    └── vscode_generated.cmake
```

## Sử dụng LCD I2C

### Ví dụ cơ bản

```cpp
#include "lcd_i2c.h"

int main(void)
{
    // Khởi tạo LCD
    LCD_Init();
    
    // In text lên LCD
    LCD_SetCursor(0, 0);      // Cột 0, Dòng 0
    LCD_Print("Hello World!");
    
    LCD_SetCursor(0, 1);      // Cột 0, Dòng 1
    LCD_Print("STM32 + W5500");
    
    while(1) {
        // Main loop
    }
}
```

### Các hàm LCD

| Hàm | Mô tả |
|-----|-------|
| `LCD_Init()` | Khởi tạo LCD |
| `LCD_Clear()` | Xóa màn hình |
| `LCD_SetCursor(col, row)` | Đặt vị trí con trỏ |
| `LCD_Print(str)` | In chuỗi ký tự |
| `LCD_PrintChar(c)` | In 1 ký tự |
| `LCD_PrintInt(num)` | In số nguyên |
| `LCD_Backlight(on)` | Bật/tắt đèn nền |
| `LCD_Cursor(on)` | Hiện/ẩn con trỏ |
| `LCD_Blink(on)` | Bật/tắt nhấp nháy con trỏ |
| `LCD_CreateChar(loc, map)` | Tạo ký tự tùy chỉnh |

### Thay đổi địa chỉ I2C

Nếu LCD không hiển thị, thử đổi địa chỉ trong `Library/lcd1602/lcd_i2c.h`:

```c
// Thử các địa chỉ phổ biến:
#define LCD_I2C_ADDR  0x27    // PCF8574
// hoặc
#define LCD_I2C_ADDR  0x3F    // PCF8574A
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
- **2026-02-05**: Cấu hình IntelliSense và build project
  - Cập nhật `.vscode/c_cpp_properties.json` với include paths
  - Build thành công: FLASH 10008B (15.27%), RAM 4136B (20.20%)
  - Output: `build/Debug/Test-stm32f103c6t8.elf`, `.hex`, `.bin`
- **2026-02-05**: Di chuyển LCD driver vào Library/lcd1602
  - Di chuyển `lcd_i2c.c` và `lcd_i2c.h` từ Inc/Src vào `Library/lcd1602/`
  - Cập nhật CMakeLists.txt với đường dẫn mới
- **2026-02-05**: Hiển thị dữ liệu nhận được từ W5500 lên LCD
  - Cập nhật `main.cpp` - thêm include `lcd_i2c.h`
  - Khởi tạo LCD khi khởi động, hiển thị IP address
  - Sau khi nhận và echo dữ liệu, hiển thị lên LCD (dòng 1: số bytes, dòng 2: nội dung)
- **2026-02-05**: Thêm LCD 1602 I2C driver
  - Thêm `lcd_i2c.c` và `lcd_i2c.h` - driver LCD 16x2 qua I2C (PCF8574)
  - Kết nối I2C1: PB6 (SCL), PB7 (SDA)
  - Địa chỉ I2C mặc định: 0x27
  - Các hàm: LCD_Init, LCD_Print, LCD_SetCursor, LCD_Backlight, LCD_CreateChar...
  - Cập nhật README với sơ đồ kết nối và hướng dẫn sử dụng
- **2026-02-05**: Thêm TCP Echo Server với socket library
  - Thêm `socket.c` và `socket.h` vào Library/W5500
  - Thêm `w5500_conf.c` và `w5500_conf.h` - cấu hình GPIO/SPI cho STM32F103C8T6
  - Cập nhật `main.cpp` với TCP Echo Server trên port 5000
  - Cấu hình mạng: IP 192.168.1.100, Gateway 192.168.1.1
- **2026-02-05**: Tạo project mới với STM32F103C8T6, thêm thư viện W5500, fix lỗi khai báo thư viện W5500

## License

Copyright (c) 2025 STMicroelectronics. All rights reserved.
