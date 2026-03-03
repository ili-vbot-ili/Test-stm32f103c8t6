# -----------------------------------------------------------------------------
# @author    STM32Cube bundle system
# -----------------------------------------------------------------------------
# @attention
#
# Copyright (c) 2025 STMicroelectronics.
# All rights reserved.
#
# This software is licensed under terms that can be found in the LICENSE file
# in the root directory of this software component.
# If no LICENSE file comes with this software, it is provided AS-IS.
#
# -----------------------------------------------------------------------------

# This file describes GNU Tools for STM32 Toolchain.
#
#   - Applies to toolchain: GNU Tools for STM32 Toolchain 13.3.1 and greater

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Some target device related default GCC settings
if(CMSIS_Dcore STREQUAL "Cortex-M0")
  set(CPU_FLAGS -mcpu=cortex-m0)
elseif(CMSIS_Dcore STREQUAL "Cortex-M0+")
  set(CPU_FLAGS -mcpu=cortex-m0plus)
elseif(CMSIS_Dcore STREQUAL "Cortex-M1")
  set(CPU_FLAGS -mcpu=cortex-m1)
elseif(CMSIS_Dcore STREQUAL "Cortex-M3")
  set(CPU_FLAGS -mcpu=cortex-m3)
elseif(CMSIS_Dcore STREQUAL "Cortex-M4")
  if(CMSIS_Dfpu STREQUAL "SP_FPU")
    set(CPU_FLAGS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=cortex-m4)
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M7")
  if(CMSIS_Dfpu STREQUAL "DP_FPU")
    set(CPU_FLAGS -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
  elseif(CMSIS_Dfpu STREQUAL "SP_FPU")
    set(CPU_FLAGS -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=cortex-m7)
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M23")
  set(CPU_FLAGS -mcpu=cortex-m23)
elseif(CMSIS_Dcore STREQUAL "Cortex-M33")
  if(CMSIS_Dfpu STREQUAL "SP_FPU")
    if(CMSIS_Ddsp STREQUAL "DSP")
      set(CPU_FLAGS -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    else()
      set(CPU_FLAGS -mcpu=cortex-m33+nodsp -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    endif()
  else()
    if(CMSIS_Ddsp STREQUAL "DSP")
      set(CPU_FLAGS -mcpu=cortex-m33)
    else()
      set(CPU_FLAGS -mcpu=cortex-m33+nodsp)
    endif()
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M35P")
  if(CMSIS_Dfpu STREQUAL "SP_FPU")
    if(CMSIS_Ddsp STREQUAL "DSP")
      set(CPU_FLAGS -mcpu=cortex-m35p -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    else()
      set(CPU_FLAGS -mcpu=cortex-m35p+nodsp -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
    endif()
  else()
    if(CMSIS_Ddsp STREQUAL "DSP")
      set(CPU_FLAGS -mcpu=cortex-m35p)
    else()
      set(CPU_FLAGS -mcpu=cortex-m35p+nodsp)
    endif()
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M55")
  if(CMSIS_Dfpu STREQUAL "NO_FPU")
    if(CMSIS_Dmve STREQUAL "NO_MVE")
      set(CPU_FLAGS -mcpu=cortex-m55+nofp+nomve)
    else()
      set(CPU_FLAGS -mcpu=cortex-m55+nofp)
    endif()
  else()
    if(CMSIS_Dmve STREQUAL "NO_MVE")
      set(CPU_FLAGS -mcpu=cortex-m55+nomve -mfloat-abi=hard)
    elseif(CMSIS_Dmve STREQUAL "MVE")
      set(CPU_FLAGS -mcpu=cortex-m55+nomve.fp -mfloat-abi=hard)
    else()
      set(CPU_FLAGS -mcpu=cortex-m55 -mfloat-abi=hard)
    endif()
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-M85")
  if(CMSIS_Dfpu STREQUAL "NO_FPU")
    if(CMSIS_Dmve STREQUAL "NO_MVE")
      set(CPU_FLAGS -mcpu=cortex-m85+nofp+nomve)
    else()
      set(CPU_FLAGS -mcpu=cortex-m85+nofp)
    endif()
  else()
    if(CMSIS_Dmve STREQUAL "NO_MVE")
      set(CPU_FLAGS -mcpu=cortex-m85+nomve -mfloat-abi=hard)
    elseif(CMSIS_Dmve STREQUAL "MVE")
      set(CPU_FLAGS -mcpu=cortex-m85+nomve.fp -mfloat-abi=hard)
    else()
      set(CPU_FLAGS -mcpu=cortex-m85 -mfloat-abi=hard)
    endif()
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-A5")
  if(CMSIS_Dfpu STREQUAL "DP_FPU")
    set(CPU_FLAGS -mcpu=cortex-a5+nosimd -mfpu=auto -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=cortex-a5+nosimd+nofp)
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-A7")
  if(CMSIS_Dfpu STREQUAL "DP_FPU")
    set(CPU_FLAGS -mcpu=cortex-a7+nosimd -mfpu=auto -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=Cortex-a7+nosimd+nofp)
  endif()
elseif(CMSIS_Dcore STREQUAL "Cortex-A9")
  if(CMSIS_Dfpu STREQUAL "DP_FPU")
    set(CPU_FLAGS -mcpu=cortex-a9+nosimd -mfpu=auto -mfloat-abi=hard)
  else()
    set(CPU_FLAGS -mcpu=cortex-a9+nosimd+nofp)
  endif()
endif()

if(CMSIS_Dendian STREQUAL "Little-endian")
  set(CPU_FLAGS ${CPU_FLAGS} -mlittle-endian)
elseif(CMSIS_Dendian STREQUAL "Big-endian")
  set(CPU_FLAGS ${CPU_FLAGS} -mbig-endian)
endif()

set(CPU_FLAGS ${CPU_FLAGS} -mthumb)

# Some C Pre-Processor related default GCC settings 
if(CMSIS_Dsecure STREQUAL "Secure" OR CMSIS_Dsecure STREQUAL "Secure-only")
  set(SECURE_FLAGS "-mcmse")
endif()

# Some default GCC settings
# Auto-detect arm-none-eabi toolchain
#
# Search order:
#   1. User-specified ARM_TOOLCHAIN_PATH (cmake -DARM_TOOLCHAIN_PATH=...)
#   2. System PATH (arm-none-eabi-gcc already in PATH)
#   3. STM32CubeIDE VS Code Extension bundle (Windows)
#   4. Common install locations (Windows / Linux / macOS)

set(TOOLCHAIN_PREFIX "arm-none-eabi-")

# Helper: try to find arm-none-eabi-gcc
if(ARM_TOOLCHAIN_PATH)
  # User explicitly specified toolchain path
  set(TOOLCHAIN_PREFIX "${ARM_TOOLCHAIN_PATH}/arm-none-eabi-")
  message(STATUS "[Toolchain] Using user-specified path: ${ARM_TOOLCHAIN_PATH}")
else()
  # Try system PATH first
  find_program(_ARM_GCC_IN_PATH "arm-none-eabi-gcc")
  if(_ARM_GCC_IN_PATH)
    message(STATUS "[Toolchain] Found in PATH: ${_ARM_GCC_IN_PATH}")
    set(TOOLCHAIN_PREFIX "arm-none-eabi-")
  else()
    # Auto-detect from common locations
    set(_TOOLCHAIN_SEARCH_PATHS "")

    # Windows: STM32CubeIDE VS Code Extension bundle
    if(WIN32 OR CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
      file(GLOB _STM32_BUNDLE_PATHS "$ENV{LOCALAPPDATA}/stm32cube/bundles/gnu-tools-for-stm32/*/bin")
      list(APPEND _TOOLCHAIN_SEARCH_PATHS ${_STM32_BUNDLE_PATHS})

      # STM32CubeIDE standalone
      file(GLOB _CUBEIDE_PATHS "C:/ST/STM32CubeIDE_*/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.*/tools/bin")
      list(APPEND _TOOLCHAIN_SEARCH_PATHS ${_CUBEIDE_PATHS})

      # Common Windows install paths
      file(GLOB _WIN_PATHS
        "C:/Program Files/GNU Arm Embedded Toolchain/*/bin"
        "C:/Program Files (x86)/GNU Arm Embedded Toolchain/*/bin"
        "C:/gcc-arm-none-eabi*/bin"
      )
      list(APPEND _TOOLCHAIN_SEARCH_PATHS ${_WIN_PATHS})
    endif()

    # Linux/macOS common paths
    if(UNIX)
      file(GLOB _UNIX_PATHS
        "/usr/bin"
        "/usr/local/bin"
        "/opt/gcc-arm-none-eabi*/bin"
        "/opt/arm-gnu-toolchain*/bin"
        "$ENV{HOME}/.local/bin"
        "$ENV{HOME}/gcc-arm-none-eabi*/bin"
      )
      list(APPEND _TOOLCHAIN_SEARCH_PATHS ${_UNIX_PATHS})

      # Homebrew (macOS)
      file(GLOB _BREW_PATHS "/opt/homebrew/bin" "/usr/local/opt/arm-none-eabi-gcc/bin")
      list(APPEND _TOOLCHAIN_SEARCH_PATHS ${_BREW_PATHS})
    endif()

    # Sort descending to prefer newest version
    if(_TOOLCHAIN_SEARCH_PATHS)
      list(SORT _TOOLCHAIN_SEARCH_PATHS ORDER DESCENDING)
    endif()

    # Search each path
    set(_FOUND_TOOLCHAIN FALSE)
    foreach(_PATH ${_TOOLCHAIN_SEARCH_PATHS})
      if(EXISTS "${_PATH}/arm-none-eabi-gcc${CMAKE_EXECUTABLE_SUFFIX}" OR EXISTS "${_PATH}/arm-none-eabi-gcc.exe" OR EXISTS "${_PATH}/arm-none-eabi-gcc")
        set(TOOLCHAIN_PREFIX "${_PATH}/arm-none-eabi-")
        set(_FOUND_TOOLCHAIN TRUE)
        message(STATUS "[Toolchain] Auto-detected: ${_PATH}")
        break()
      endif()
    endforeach()

    if(NOT _FOUND_TOOLCHAIN)
      message(WARNING
        "[Toolchain] arm-none-eabi-gcc NOT FOUND!\n"
        "  Solutions:\n"
        "  1. Install 'STM32CubeIDE for VS Code' extension (auto-downloads toolchain)\n"
        "  2. Install GNU Arm Embedded Toolchain and add to PATH\n"
        "  3. Specify path: cmake -DARM_TOOLCHAIN_PATH=/path/to/bin ...\n"
        "  Searched paths: ${_TOOLCHAIN_SEARCH_PATHS}"
      )
      # Fallback: hope it's in PATH at build time
      set(TOOLCHAIN_PREFIX "arm-none-eabi-")
    endif()
  endif()
endif()

set(FLAGS                           "-fdata-sections -ffunction-sections -Wl,--gc-sections -Wno-comment")
set(CPP_FLAGS                       "${FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_C_FLAGS                   ${FLAGS})
set(CMAKE_CXX_FLAGS                 ${CPP_FLAGS})

set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)