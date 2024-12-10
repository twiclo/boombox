cmake_minimum_required(VERSION 3.13)
include(pico-sdk/pico_sdk_init.cmake)
project(boombox CXX C ASM)

pico_sdk_init()

add_executable(boombox src/main.c src/hw_config.c)
add_subdirectory(lib/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build_fatfs)
add_subdirectory(lib/pico-extras/src/rp2_common/pico_audio_i2s build_audio_i2s)
add_subdirectory(lib/pico-extras/src/common/pico_audio build_audio)
target_link_libraries(boombox pico_stdlib pico_cyw43_arch_none FatFs_SPI pico_audio_i2s)

# add_executable(boombox src/main.c src/hw_config.c)
# add_subdirectory(lib/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)
# target_link_libraries(boombox pico_stdlib pico_cyw43_arch_none FatFs_SPI)

pico_enable_stdio_usb(boombox 1)
pico_enable_stdio_uart(boombox 0)
pico_add_extra_outputs(boombox)

add_definitions(-DPICO_BPARD=pico_w)
