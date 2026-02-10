# STM32F103C8T6 Flash Script
# Requires STM32CubeProgrammer installed

$PROGRAMMER = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
$FIRMWARE_HEX = "build\Debug\Test-stm32f103c6t8.hex"
$FIRMWARE_ELF = "build\Debug\Test-stm32f103c6t8.elf"

# Check if programmer exists
if (-not (Test-Path $PROGRAMMER)) {
    Write-Host "ERROR: STM32CubeProgrammer not found!" -ForegroundColor Red
    Write-Host "Please install from: https://www.st.com/en/development-tools/stm32cubeprog.html"
    exit 1
}

# Check firmware file
$firmware = $null
if (Test-Path $FIRMWARE_HEX) {
    $firmware = $FIRMWARE_HEX
} elseif (Test-Path $FIRMWARE_ELF) {
    $firmware = $FIRMWARE_ELF
} else {
    Write-Host "ERROR: Firmware file not found! Please build the project first." -ForegroundColor Red
    Write-Host "Run: cmake --build build/Debug"
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " STM32F103C8T6 Firmware Flash Tool" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Firmware: $firmware" -ForegroundColor Green
Write-Host ""

# Select connection method
Write-Host "Select connection method:"
Write-Host "  1. ST-Link (SWD)"
Write-Host "  2. Serial (UART via USB-TTL)"
Write-Host ""
$choice = Read-Host "Enter choice (1 or 2)"

switch ($choice) {
    "1" {
        Write-Host ""
        Write-Host "Flashing via ST-Link (SWD)..." -ForegroundColor Yellow
        Write-Host "Make sure ST-Link is connected!"
        Write-Host ""
        
        # Flash using ST-Link
        & $PROGRAMMER -c port=SWD -w $firmware -v -rst
    }
    "2" {
        Write-Host ""
        Write-Host "Flashing via Serial (UART)..." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Connection:"
        Write-Host "  USB-TTL TX  -> STM32 PA10 (RX)"
        Write-Host "  USB-TTL RX  -> STM32 PA9  (TX)"
        Write-Host "  USB-TTL GND -> STM32 GND"
        Write-Host ""
        Write-Host "BOOT0 = 1, BOOT1 = 0, then press RESET"
        Write-Host ""
        
        $comPort = Read-Host "Enter COM port (e.g., COM3)"
        
        # Flash using Serial
        & $PROGRAMMER -c port=$comPort -w $firmware -v -rst
    }
    default {
        Write-Host "Invalid choice!" -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
