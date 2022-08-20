cmake_minimum_required(VERSION 3.13)

include(../pico-sdk/pico_sdk_init.cmake)

project(led_clock)

add_executable(led_clock
  led_clock.c
  )

pico_sdk_init()

# pull in common dependencies
target_link_libraries(led_clock pico_stdlib)

# # enable usb output, disable uart output
pico_enable_stdio_usb(led_clock 1)
pico_enable_stdio_uart(led_clock 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(led_clock)
