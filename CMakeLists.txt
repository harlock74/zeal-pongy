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

project(zeal-pongy C ASM)

# -------------------------------
# Executable
# -------------------------------
add_executable(zeal-pongy
    src/pong.c
    src/assets.c
)

# -------------------------------
# SDKs
# -------------------------------
zos_use(zvb)
find_package(ZVB_SDK REQUIRED)
target_link_libraries(zeal-pongy PUBLIC zvb_gfx zvb_sound)

zos_use(zgdk)
find_package(ZGDK REQUIRED)
target_link_libraries(zeal-pongy PUBLIC zgdk)

# -------------------------------
# Assets
# -------------------------------
gif2zeal(zeal-pongy FILES
    "assets/pong.gif"
    COMPRESSED)

tiled2zeal(zeal-pongy FILES
    "assets/tilemap.tmx"
    LAYER 0
    VERBOSE)

# -------------------------------
# Convert to binary
# -------------------------------
zos_add_outputs(zeal-pongy)
