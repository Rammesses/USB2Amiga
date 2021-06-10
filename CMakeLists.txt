cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(usb2amiga C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(usb2amiga
  src/usb2amiga.c
  src/ami_joystick.c
  src/ami_kbd.c
  src/ami_mouse.c
  src/usb_hid_gamepad.c
  src/usb_hid_kbd.c
  src/usb_hid_mouse.c
)

pico_enable_stdio_uart(usb2amiga 1)

target_include_directories(usb2amiga PRIVATE src)
target_link_libraries(usb2amiga PRIVATE pico_stdlib pico_multicore tinyusb_host tinyusb_board)

pico_add_extra_outputs(usb2amiga)
