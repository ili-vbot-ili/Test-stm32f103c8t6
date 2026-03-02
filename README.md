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
│   └── P10/                # Thư viện P10 LED Matrix (HUB12)
│       ├── p10.c
│       └── p10.h
└── cmake/
    ├── gnu-tools-for-stm32.cmake
    └── vscode_generated.cmake
```

### 3. P10 LED Matrix Module (HUB12 Interface)

| P10 Pin | STM32F103C8T6 Pin | Chức năng |
|---------|-------------------|-----------|
| VCC     | 5V                | Nguồn     |
| GND     | GND               | Mass      |
| OE      | PB10              | Output Enable (Active LOW) |
| A       | PB11              | Row Select A |
| B       | PB12              | Row Select B |
| CLK     | PB13              | Clock |
| LAT/STB | PB14              | Latch |
| DATA/R  | PB15              | Data |

#### Sơ đồ kết nối P10

```
STM32F103C8T6                    P10 LED Matrix (HUB12)
    ┌─────────┐                  ┌─────────┐
    │       5V├──────────────────┤VCC      │
    │      GND├──────────────────┤GND      │
    │     PB10├──────────────────┤OE       │
    │     PB11├──────────────────┤A        │
    │     PB12├──────────────────┤B        │
    │     PB13├──────────────────┤CLK      │
    │     PB14├──────────────────┤LAT/STB  │
    │     PB15├──────────────────┤DATA/R   │
    └─────────┘                  └─────────┘
```

#### Lưu ý P10
- Sử dụng **GPIOB** (PB10-PB15) để tránh xung đột với W5500 trên GPIOA
- Panel kích thước: **32x16 pixels**, quét 1/4 (1/4 scan)
- Cần gọi `P10_Refresh()` định kỳ (mỗi 1-2ms) hoặc dùng `P10_SetupTimer()` để tự động refresh
- Timer sử dụng: **TIM3** với interrupt 1ms

## Sử dụng P10 LED Matrix

### Ví dụ cơ bản

```cpp
#include "p10.h"

int main(void)
{
    // Khởi tạo P10
    P10_Init();
    P10_SetupTimer();  // Tự động refresh bằng Timer
    
    // Xóa màn hình và hiển thị text
    P10_Clear();
    P10_DrawString(0, 0, "HELLO");
    P10_DrawString(0, 8, "WORLD");
    
    while(1) {
        // Main loop
    }
}
```

### Các hàm P10

| Hàm | Mô tả |
|-----|-------|
| `P10_Init()` | Khởi tạo P10 |
| `P10_SetupTimer()` | Cấu hình Timer tự động refresh |
| `P10_Clear()` | Xóa màn hình (tắt tất cả LED) |
| `P10_Fill()` | Bật tất cả LED |
| `P10_SetPixel(x, y, value)` | Đặt pixel tại (x, y) |
| `P10_GetPixel(x, y)` | Lấy giá trị pixel |
| `P10_DrawChar(x, y, c)` | Vẽ ký tự |
| `P10_DrawString(x, y, str)` | Vẽ chuỗi ký tự |
| `P10_DrawLine(x0, y0, x1, y1)` | Vẽ đường thẳng |
| `P10_DrawRect(x, y, w, h, fill)` | Vẽ hình chữ nhật |
| `P10_ScrollText(y, str, delay)` | Cuộn text ngang |
| `P10_SetBrightness(brightness)` | Đặt độ sáng (0-100%) |
| `P10_Refresh()` | Refresh màn hình (gọi trong interrupt) |

### Cấu hình số lượng panel

Thay đổi trong `Library/P10/p10.h`:

```c
#define P10_PANELS_X        1       // Số panel theo chiều ngang
#define P10_PANELS_Y        1       // Số panel theo chiều dọc
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

## Flash Firmware

### Phương pháp 1: ST-Link (SWD) - Khuyến nghị

**Kết nối ST-Link với STM32:**

| ST-Link | STM32F103C8T6 |
|---------|---------------|
| SWDIO   | PA13 (SWDIO)  |
| SWCLK   | PA14 (SWCLK)  |
| GND     | GND           |
| 3.3V    | 3.3V          |

**Sơ đồ kết nối ST-Link:**

```
STM32F103C8T6                    ST-Link V2
    ┌─────────┐                  ┌─────────┐
    │     3.3V├──────────────────┤3.3V     │
    │      GND├──────────────────┤GND      │
    │     PA13├──────────────────┤SWDIO    │
    │     PA14├──────────────────┤SWCLK    │
    └─────────┘                  └─────────┘
```

**Nạp firmware:**

```powershell
# Sử dụng script có sẵn
.\flash.ps1

# Hoặc sử dụng trực tiếp STM32CubeProgrammer CLI
& "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=SWD -w build\Debug\Test-stm32f103c6t8.hex -v -rst
```

### Phương pháp 2: Serial (UART via USB-TTL)

**Kết nối USB-TTL với STM32:**

| USB-TTL | STM32F103C8T6 |
|---------|---------------|
| TX      | PA10 (RX)     |
| RX      | PA9 (TX)      |
| GND     | GND           |
| 3.3V    | 3.3V          |

**Cấu hình Boot pins:**
- **BOOT0 = 1** (nối lên 3.3V)
- **BOOT1 = 0** (nối xuống GND)
- Nhấn nút **RESET**

**Nạp firmware:**

```powershell
# Thay COM3 bằng cổng COM của bạn
& "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=COM3 -w build\Debug\Test-stm32f103c6t8.hex -v -rst
```

**Sau khi nạp xong:**
- **BOOT0 = 0** (nối xuống GND)
- Nhấn nút **RESET** để chạy chương trình

### Phương pháp 3: Sử dụng VS Code

1. Cài đặt extension **STM32 for VS Code**
2. Kết nối ST-Link
3. Nhấn `F5` hoặc chọn **Run > Start Debugging**

## History

- **2026-03-02**: Debug, sửa lỗi và hoàn thiện firmware TCP Echo Server
  - **Sửa lỗi SystemInit()**: Startup code gọi `bl SystemInit` nhưng không có implementation → MCU crash. Thêm `extern "C" void SystemInit(void) {}` trong `main.cpp`
  - **Thêm SystemClock_Config()**: Cấu hình HSE 8MHz + PLL x9 = 72MHz bằng raw register (không dùng CMSIS macros). Có fallback về HSI 8MHz nếu HSE timeout
  - **Thêm FLASH_TypeDef** trong `w5500_conf.h`: Định nghĩa FLASH register base address và struct để truy cập Flash ACR (cần cho cấu hình wait states ở 72MHz)
  - **Thêm LED debug (PC13)**: Blink pattern để chẩn đoán: 2=MCU started, 3=Clock OK, 4=W5500 init, 5=SPI OK, 6=TCP listening, heartbeat trong main loop
  - **Phát hiện lỗi BOOT0**: BOOT0=1 khiến MCU boot vào system memory bootloader thay vì Flash → firmware không chạy. Fix: BOOT0=0 (GND)
  - **LCD I2C auto-detect với timeout**: Thêm I2C timeout (5000 cycles), `_lcd_connected` flag, auto-probe address trong `LCD_Init()`. Nếu LCD không kết nối → tự động skip, không block MCU
  - **Thêm `LCD_IsConnected()`**: Hàm kiểm tra LCD có kết nối hay không
  - **Tối ưu thứ tự khởi tạo**: Mở TCP socket + listen **trước** LCD/P10 init, tránh bị block bởi I2C timeout khi LCD chưa kết nối
  - **Verify W5500 SPI**: Đọc chip version register (phải = 0x04) để xác nhận SPI hoạt động
  - **Nâng cấp ST-Link firmware**: V2J46S7 qua STM32CubeIDE GUI
  - **Kết quả**: Ping OK, TCP Echo Server hoạt động trên 192.168.1.100:5000
- **2026-02-10**: Thêm Flash Script và hướng dẫn nạp firmware
  - Thêm `flash.ps1` - Script PowerShell để nạp firmware
  - Hỗ trợ ST-Link (SWD) và Serial (UART)
  - Cập nhật README với hướng dẫn chi tiết kết nối và nạp firmware
- **2026-02-06**: Thêm P10 LED Matrix driver (HUB12 Interface)
  - Thêm `Library/P10/p10.c` và `Library/P10/p10.h`
  - Kết nối sử dụng GPIOB (PB10-PB15) để tránh xung đột với W5500
  - Hỗ trợ panel 32x16 pixels, quét 1/4
  - Timer TIM3 cho tự động refresh
  - Các hàm: P10_Init, P10_DrawString, P10_SetPixel, P10_ScrollText...
  - Font 5x7 tích hợp sẵn (ASCII 32-127)
  - Cập nhật `main.cpp` để hiển thị dữ liệu TCP lên cả LCD và P10
- **2026-02-05**: Cấu hình IntelliSense và build project
  - Cập nhật `.vscode/c_cpp_properties.json` với include paths
  - Build thành công: FLASH 10008B (15.27%), RAM 4136B (20.20%)
- **2026-02-05**: Di chuyển LCD driver vào Library/lcd1602
- **2026-02-05**: Hiển thị dữ liệu nhận được từ W5500 lên LCD
- **2026-02-05**: Thêm LCD 1602 I2C driver
- **2026-02-05**: Thêm TCP Echo Server với socket library
- **2026-02-05**: Tạo project mới với STM32F103C8T6, thêm thư viện W5500

## License

Copyright (c) 2025 STMicroelectronics. All rights reserved.
