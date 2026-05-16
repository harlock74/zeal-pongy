cmake_minimum_required(VERSION 3.16)

# -------------------------------
# Toolchain
# -------------------------------
set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
# Cross-compiling for Z80: avoid link checks in try-compile tests.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# -------------------------------
# Project
# -------------------------------

project(zpong C ASM)

# -------------------------------
# Executable
# -------------------------------
add_executable(zpong
    src/pong.c
    src/assets.c
)

# -------------------------------
# SDKs
# -------------------------------
zos_use(zvb)
find_package(ZVB_SDK REQUIRED)
target_link_libraries(zpong PUBLIC zvb_gfx zvb_sound)

zos_use(zgdk)
find_package(ZGDK REQUIRED)
target_link_libraries(zpong PUBLIC zgdk)

# -------------------------------
# Assets
# -------------------------------
gif2zeal(zpong FILES
    "assets/pong.gif"
    COMPRESSED)

tiled2zeal(zpong FILES
    "assets/tilemap.tmx"
    LAYER 0
    VERBOSE)

# -------------------------------
# Convert to binary
# -------------------------------
zos_add_outputs(zpong)
