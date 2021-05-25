cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(usb2amiga C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(usb2amiga
  src/usb2amiga.c
  src/ami_kbd.c
  src/ami_mouse.c
  src/usb_hid_kbd.c
  src/usb_hid_mouse.c
)

pico_enable_stdio_uart(usb2amiga 1)
pico_add_extra_outputs(usb2amiga)
target_link_libraries(usb2amiga pico_stdlib)
