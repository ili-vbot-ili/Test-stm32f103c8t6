# Test-stm32f103c8t6

Project STM32F103C8T6 với thư viện W5500 Ethernet.

## Thông tin phần cứng

- **MCU**: STM32F103C8T6 (Blue Pill)
- **Core**: ARM Cortex-M3
- **Flash**: 64KB
- **RAM**: 20KB
- **Module Ethernet**: W5500
- **LCD**: LCD 1602 với I2C adapter (PCF8574)
- **Flash Memory**: W25Q128 SPI Flash (16MB)
- **RFID Reader**: Wiegand 26/34/37-bit × 2 cổng (multi-port RFID)
- **Relay**: 4 kênh relay điều khiển (cửa/thiết bị)

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
│   └── P10/                # Thư viện P10 LED Matrix (tạm tắt)
│       ├── p10.c
│       └── p10.h
│   └── W25Q/               # Thư viện W25Q128 SPI Flash
│       ├── w25q128.c
│       └── w25q128.h
│   └── Wiegand/             # Thư viện Wiegand RFID Reader (2-port, JTAG preserved)
│       ├── wiegand.c
│       └── wiegand.h
│   └── Relay/               # Thư viện điều khiển Relay (4 kênh)
│       ├── relay.c
│       └── relay.h
└── cmake/
    ├── gnu-tools-for-stm32.cmake
    └── vscode_generated.cmake
```

### 3. W25Q128 SPI Flash Module (Software SPI - GPIOB)

> ⚠️ **P10 LED Matrix tạm tắt** — chân PB12-PB15 được chuyển cho W25Q128 Flash. File P10 vẫn giữ trong `Library/P10/` để bật lại khi cần.

| W25Q128 Pin | STM32F103C8T6 Pin | Chức năng |
|-------------|-------------------|-----------|
| VCC         | 3.3V              | Nguồn     |
| GND         | GND               | Mass      |
| CS          | PB12              | Chip Select |
| CLK         | PB13              | Clock |
| DO (MISO)   | PB14              | Data Out (Flash → MCU) |
| DI (MOSI)   | PB15              | Data In (MCU → Flash) |

#### Sơ đồ kết nối W25Q128

```
STM32F103C8T6                    W25Q128 SPI Flash
    ┌─────────┐                  ┌─────────┐
    │     3.3V├──────────────────┤VCC      │
    │      GND├──────────────────┤GND      │
    │     PB12├──────────────────┤CS       │
    │     PB13├──────────────────┤CLK      │
    │     PB14├──────────────────┤DO(MISO) │
    │     PB15├──────────────────┤DI(MOSI) │
    │         │           3.3V──┤WP       │
    │         │           3.3V──┤HOLD     │
    └─────────┘                  └─────────┘
```

#### Lưu ý W25Q128
- Điện áp: **3.3V** (tương thích trực tiếp với STM32)
- Chân **WP** và **HOLD** nối lên 3.3V nếu không dùng
- Dung lượng: **16MB** (128Mbit), đủ lưu ~262,000 log entries
- Sử dụng **Software SPI** (bit-bang) trên GPIOB, không xung đột với W5500 (SPI1 hardware)
- Ghi/xóa: **100,000 lần** mỗi sector

### 5. Wiegand RFID Reader — 2 Cổng (GPIOA)

> ℹ️ **JTAG được giữ nguyên** — Chỉ sử dụng 2 cổng Wiegand (không dùng PA2/PA15), giữ nguyên cổng JTAG đầy đủ.

| Reader | D0 Pin | D1 Pin | Ghi chú |
|--------|--------|--------|---------|
| Port 0 (chính) | PA3 | PA8 | Cổng chính |
| Port 1 (phụ)   | PA0 | PA1 | Cổng phụ |

Mỗi reader cần thêm: VCC (12V/5V), GND (chung với STM32).

#### Sơ đồ kết nối 2 Wiegand Readers

```
STM32F103C8T6                    Wiegand RFID Readers
    ┌─────────┐
    │      GND├────────┬──────── GND (chung)
    │      PA3├────────┤         Reader 0 D0
    │      PA8├────────┤         Reader 0 D1
    │      PA0├────────┼──────── Reader 1 D0
    │      PA1├────────┼──────── Reader 1 D1
    └─────────┘
```

#### Lưu ý Wiegand Multi-Port
- **JTAG được giữ nguyên**: PA15/PB3/PB4 không bị sử dụng, hỗ trợ debug JTAG đầy đủ
- Mỗi reader cần **GND chung** với STM32
- **Nguồn reader**: thường **12V DC** (một số model dùng 5V)
- D0/D1 idle ở mức **HIGH**, xung **LOW ~50µs** mỗi bit
- Hỗ trợ: **Wiegand 26-bit** (phổ biến nhất), **34-bit**, **37-bit**
- PA0, PA1, PA3, PA8 đều là **5V tolerant**
- `WG_Process()` trả về bitmask: bit0=Port0, bit1=Port1

### 6. Relay Module — 4 Kênh (GPIOA)

| Relay | STM32 Pin | Chức năng |
|-------|-----------|-----------|
| Relay 0 | PA9  | Cửa 1 / Thiết bị 1 |
| Relay 1 | PA10 | Cửa 2 / Thiết bị 2 |
| Relay 2 | PA11 | Cửa 3 / Thiết bị 3 |
| Relay 3 | PA12 | Cửa 4 / Thiết bị 4 |

#### Sơ đồ kết nối 4 Relay

```
STM32F103C8T6                    4-Channel Relay Module
    ┌─────────┐                  ┌───────────────────┐
    │       5V├──────────────────┤VCC                │
    │      GND├──────────────────┤GND                │
    │      PA9├──────────────────┤IN1 (Relay 0)      │
    │     PA10├──────────────────┤IN2 (Relay 1)      │
    │     PA11├──────────────────┤IN3 (Relay 2)      │
    │     PA12├──────────────────┤IN4 (Relay 3)      │
    └─────────┘                  └───────────────────┘
```

#### Lưu ý Relay
- **Active LOW** (mặc định): hầu hết relay module có optocoupler, IN=LOW → relay ON
  - Nếu module active-high, đổi `RELAY_ACTIVE_HIGH` thành `1` trong `relay.h`
- **Nguồn relay module**: dùng **5V** (VCC riêng hoặc qua USB STM32)
- Tín hiệu 3.3V từ STM32 đủ điều khiển hầu hết relay module optocoupler
- Hỗ trợ **auto-off** (pulse): relay tự tắt sau thời gian đặt trước
- Khi quẹt thẻ hợp lệ, relay tương ứng port sẽ tự bật ~3 giây rồi tắt
- PA9-PA12 đều là **5V tolerant**, output Push-Pull 2MHz

### Tổng hợp GPIO đã sử dụng

```
GPIOA:
  PA0  - Wiegand Port 1 D0 (RFID Reader 1)
  PA1  - Wiegand Port 1 D1 (RFID Reader 1)
  PA2  - (trống)
  PA3  - Wiegand Port 0 D0 (RFID Reader 0 — chính)
  PA4  - W5500 CS (SPI1_NSS)
  PA5  - W5500 SCK (SPI1_SCK)
  PA6  - W5500 MISO (SPI1_MISO)
  PA7  - W5500 MOSI (SPI1_MOSI)
  PA8  - Wiegand Port 0 D1 (RFID Reader 0 — chính)
  PA9  - Relay 0 (Cửa 1)
  PA10 - Relay 1 (Cửa 2)
  PA11 - Relay 2 (Cửa 3)
  PA12 - Relay 3 (Cửa 4)
  PA13 - SWDIO (Debug — JTAG)
  PA14 - SWCLK (Debug — JTAG)
  PA15 - JTDI  (Debug — JTAG, giữ nguyên)
  
GPIOB:
  PB0  - W5500 RST
  PB1  - W5500 INT (tùy chọn)
  PB3  - JTDO  (Debug — JTAG, giữ nguyên)
  PB4  - NJTRST (Debug — JTAG, giữ nguyên)
  PB6  - LCD I2C SCL (I2C1)
  PB7  - LCD I2C SDA (I2C1)
  PB8  - (trống — DS1302 đã bỏ)
  PB9  - (trống — DS1302 đã bỏ)
  PB10 - (trống — DS1302 đã bỏ)
  PB12 - W25Q128 CS
  PB13 - W25Q128 CLK
  PB14 - W25Q128 MISO
  PB15 - W25Q128 MOSI

GPIOC:
  PC13 - LED onboard (heartbeat)

Chân còn trống: PA2, PB2, PB5, PB8, PB9, PB10, PB11 (7 chân)
Chân JTAG giữ nguyên: PA13, PA14, PA15, PB3, PB4
```

## Sử dụng W25Q128 SPI Flash

### Các hàm W25Q128

| Hàm | Mô tả |
|-----|-------|
| `W25Q_Init()` | Khởi tạo, verify JEDEC ID. Return 1=OK |
| `W25Q_IsConnected()` | Kiểm tra chip có kết nối không |
| `W25Q_ReadJEDEC()` | Đọc JEDEC ID (0xEF4018 = W25Q128) |
| `W25Q_Read(addr, buf, len)` | Đọc data từ flash |
| `W25Q_Write(addr, buf, len)` | Ghi data (auto page-program) |
| `W25Q_WritePage(addr, buf, len)` | Ghi 1 page (max 256 bytes) |
| `W25Q_EraseSector(addr)` | Xóa sector 4KB |
| `W25Q_EraseBlock(addr)` | Xóa block 64KB |
| `W25Q_EraseChip()` | Xóa toàn bộ chip (20-100s) |
| `W25Q_IsBusy()` | Kiểm tra chip đang bận |
| `W25Q_PowerDown()` | Chế độ tiết kiệm điện |
| `W25Q_WakeUp()` | Đánh thức từ power-down |

### Các hàm Data Logger

| Hàm | Mô tả |
|-----|-------|
| `W25Q_LogInit()` | Khởi tạo hệ thống log |
| `W25Q_LogWrite(type, data, len)` | Ghi log entry (max 56 bytes data) |
| `W25Q_LogRead(index, &entry)` | Đọc log theo index |
| `W25Q_LogCount()` | Tổng số log entries |
| `W25Q_GetBootCount()` | Số lần boot |
| `W25Q_LogClear()` | Xóa tất cả log |

### Log Types

| Type | Giá trị | Mô tả |
|------|---------|-------|
| `W25Q_LOG_TCP_RECV` | 0 | Dữ liệu TCP nhận được |
| `W25Q_LOG_TCP_SEND` | 1 | Dữ liệu TCP gửi đi |
| `W25Q_LOG_SYSTEM` | 2 | Sự kiện hệ thống |
| `W25Q_LOG_ERROR` | 3 | Lỗi |
| `W25Q_LOG_BOOT` | 4 | Boot event |

## Sử dụng Wiegand RFID Reader (2 Cổng)

### Các hàm Wiegand (Multi-Port)

| Hàm | Mô tả |
|-----|-------|
| `WG_Init()` | Khởi tạo 2 cổng (JTAG giữ nguyên) |
| `WG_InitPort(port)` | Khởi tạo 1 cổng (0-1) |
| `WG_Process()` | Poll tất cả — return bitmask (bit0-1 = port0-1) |
| `WG_ProcessPort(port)` | Poll 1 cổng — return 1 khi có thẻ |
| `WG_GetCard(port, &card)` | Lấy dữ liệu thẻ từ cổng. Return 1=OK |
| `WG_Available(port)` | Có thẻ chưa đọc trên cổng? |
| `WG_AnyAvailable()` | Cổng nào có thẻ? (0-1 hoặc 0xFF=không) |
| `WG_Reset(port)` | Reset 1 cổng |
| `WG_ResetAll()` | Reset tất cả |
| `WG_GetBitCount(port)` | Số bit đã nhận trên cổng |
| `WG_GetRawBits(port)` | Raw bits trên cổng |
| `WG_Decode(bits, count, &card)` | Decode thủ công từ raw bits |
| `WG_CheckParity26(raw)` | Kiểm tra parity Wiegand 26-bit |
| `WG_CheckParity34(raw)` | Kiểm tra parity Wiegand 34-bit |
| `WG_FormatCard(&card, buf)` | Format "P0 FC:xxx ID:xxxxx" (cần 28 bytes) |
| `WG_FormatCardNumber(&card, buf)` | Format chỉ card number (cần 12 bytes) |

### Wiegand Card Data Structure

| Field | Type | Mô tả |
|-------|------|-------|
| `cardNumber` | `uint32_t` | Số thẻ (Card Number) |
| `facilityCode` | `uint16_t` | Mã cơ sở (Facility Code) |
| `rawData` | `uint32_t` | Dữ liệu thô (không parity) |
| `rawBits` | `uint64_t` | Toàn bộ bits nhận được |
| `bitCount` | `uint8_t` | Số bit (26, 34, 37...) |
| `format` | `uint8_t` | WG_FORMAT_26BIT, 34BIT, 37BIT, UNKNOWN |
| `valid` | `uint8_t` | 1 = parity OK, 0 = lỗi parity |
| `port` | `uint8_t` | Cổng đọc (0-1) |

## Sử dụng Relay (4 Kênh)

### Các hàm Relay

| Hàm | Mô tả |
|-----|-------|
| `RELAY_Init()` | Khởi tạo PA9-PA12 output, tất cả OFF |
| `RELAY_On(ch)` | Bật relay kênh ch (0-3) |
| `RELAY_Off(ch)` | Tắt relay kênh ch (0-3) |
| `RELAY_Toggle(ch)` | Đảo trạng thái relay |
| `RELAY_Set(ch, state)` | Đặt trạng thái (RELAY_ON/OFF) |
| `RELAY_Get(ch)` | Đọc trạng thái 1 kênh |
| `RELAY_AllOn()` | Bật tất cả |
| `RELAY_AllOff()` | Tắt tất cả |
| `RELAY_SetMask(mask)` | Đặt nhiều kênh (bitmask) |
| `RELAY_GetMask()` | Đọc trạng thái bitmask |
| `RELAY_Pulse(ch, delay)` | Bật + auto-off sau delay loop counts |
| `RELAY_Process()` | Xử lý auto-off — gọi trong main loop |
| `RELAY_TestAll(ms)` | Test tuần tự từng relay |
| `RELAY_IsInitialized()` | Đã khởi tạo chưa? |
| `RELAY_FormatStatus(buf)` | Format "R:1010" (cần 7 bytes) |

### Relay Configuration

| Macro | Giá trị mặc định | Mô tả |
|-------|-------------------|-------|
| `RELAY_NUM_CHANNELS` | 4 | Số kênh relay |
| `RELAY_ACTIVE_HIGH` | 0 | 0=Active LOW, 1=Active HIGH |

## Sử dụng LCD I2C

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

### Lưu ý
- **Địa chỉ I2C mặc định**: `0x27` (PCF8574) hoặc `0x3F` (PCF8574A) — đổi trong `Library/lcd1602/lcd_i2c.h`

## Yêu cầu

- [STM32CubeIDE for VS Code Extension](https://marketplace.visualstudio.com/items?itemName=stmicroelectronics.stm32-vscode-extension)
- GNU Arm Embedded Toolchain
- CMake >= 3.20
- Ninja Build System

## Build

- CMake Presets: `cmake --preset Debug` → `cmake --build build/Debug`
- VS Code: Chọn preset "Debug"/"Release" → Build (Ctrl+Shift+B)

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

**Nạp firmware:** Chạy `.\flash.ps1` hoặc dùng STM32CubeProgrammer CLI với `port=SWD`.

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

**Nạp firmware:** Dùng STM32CubeProgrammer CLI với `port=COMx`. Sau khi nạp, chuyển **BOOT0 = 0** (GND) và nhấn **RESET**.

### Phương pháp 3: Sử dụng VS Code

1. Cài đặt extension **STM32 for VS Code**
2. Kết nối ST-Link
3. Nhấn `F5` hoặc chọn **Run > Start Debugging**

## History

- **2026-03-03**: Bỏ DS1302 RTC, giữ nguyên JTAG, giảm Wiegand xuống 2 cổng
  - Bỏ `Library/DS1302/ds1302.c` và `ds1302.h` khỏi build (CMakeLists.txt)
  - Bỏ toàn bộ code DS1302 trong `main.cpp` (init, time display, timestamp log)
  - Giảm Wiegand từ 3 cổng xuống 2 cổng: Port 0 (PA3/PA8), Port 1 (PA0/PA1)
  - Bỏ Port 2 (PA2/PA15) — giữ nguyên cổng JTAG đầy đủ (PA15/PB3/PB4)
  - Không còn disable JTAG trong `WG_Init()`
  - Giải phóng chân: PA2, PB8, PB9, PB10 (tổng 7 chân trống)
  - Cập nhật README: bỏ DS1302 documentation, GPIO summary, Wiegand 2-port
- **2026-03-03**: Mở rộng 3 cổng Wiegand + 4 kênh Relay
  - Cập nhật `Library/Wiegand/wiegand.c` thành multi-port (3 cổng độc lập)
  - Port 0: PA3/PA8, Port 1: PA0/PA1, Port 2: PA2/PA15
  - PA15 tự động disable JTAG (SWD debug vẫn hoạt động)
  - `WG_Process()` trả bitmask, `WG_GetCard(port, &card)` lấy dữ liệu theo cổng
  - Format output: "P0 FC:xxx ID:xxxxx" (có port number)
  - Thêm `Library/Relay/relay.c` và `Library/Relay/relay.h` — 4 kênh relay
  - Relay 0-3 trên PA9, PA10, PA11, PA12
  - Active LOW (optocoupler), có thể đổi Active HIGH qua macro
  - Hỗ trợ: On/Off/Toggle, Bitmask, Auto-off Pulse, Test sequence
  - Tích hợp: quẹt thẻ hợp lệ → bật relay tương ứng port ~3 giây
  - LCD hiển thị trạng thái relay: "R:1010"
  - Build OK: FLASH 30.36%, RAM 21.68%
  - Tất cả chân GPIOA đã dùng hết, còn trống: PB2-PB5, PB11 (5 chân)
- **2026-03-03**: Thêm Wiegand RFID Reader driver
  - Thêm `Library/Wiegand/wiegand.c` và `Library/Wiegand/wiegand.h`
  - Polling-based trên PA3 (D0) và PA8 (D1)
  - Hỗ trợ: Wiegand 26-bit (H10301), 34-bit, 37-bit
  - Tự động decode: Facility Code, Card Number, parity check
  - Tích hợp vào main.cpp: quẹt thẻ → hiển thị LCD + log Flash kèm timestamp
  - Format output: "FC:xxx ID:xxxxx"
- **2026-03-03**: Chuyển DS1302 từ GPIOA sang GPIOB (PB8/PB9/PB10)
  - Cùng port với LCD I2C (PB6/PB7), dễ đi dây
  - Giải phóng PA0-PA2 cho mục đích khác
- **2026-03-03**: Thêm DS1302 RTC driver, tích hợp timestamp vào log
  - Thêm `Library/DS1302/ds1302.c` và `Library/DS1302/ds1302.h`
  - Giao tiếp 3-Wire (bit-bang) trên PA0 (CE), PA1 (CLK), PA2 (DAT)
  - Hỗ trợ: đọc/ghi thời gian, RAM 31 bytes, format chuỗi, timestamp
  - Tích hợp vào main.cpp: khởi tạo RTC, hiển thị thời gian trên LCD
  - Log TCP data kèm timestamp thực từ DS1302 vào W25Q128 Flash
  - Auto-detect: nếu DS1302 không kết nối, bỏ qua không block MCU
  - Cập nhật README: kết nối phần cứng, API reference, GPIO summary
- **2026-03-03**: Thêm W25Q128 SPI Flash driver, tạm tắt P10
  - Thêm `Library/W25Q/w25q128.c` và `Library/W25Q/w25q128.h`
  - Software SPI (bit-bang) trên PB12-PB15 (reuse chân P10)
  - Hỗ trợ: Read, Write, Erase (Sector/Block/Chip), Power Down
  - Data Logger tích hợp: circular buffer, 262K entries, boot counter
  - Tự động log dữ liệu TCP nhận được vào Flash
  - P10 LED Matrix tạm disable trong CMakeLists.txt và main.cpp (file vẫn giữ)
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
