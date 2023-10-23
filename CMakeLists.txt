cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(bomb_it C CXX ASM)
pico_sdk_init()

add_compile_definitions(PICO_STDIO_STACK_BUFFER_SIZE = 256)

add_subdirectory(display_lib/Config)
add_subdirectory(display_lib/LCD)
add_subdirectory(display_lib/Fonts)
add_subdirectory(display_lib/GUI)


include_directories(./display_lib/Config)
include_directories(./display_lib/GUI)
include_directories(./display_lib/LCD)
include_directories(./display_lib/Fonts)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src)

target_link_libraries(main pico_stdlib)


target_include_directories(main PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(main 
        hardware_spi
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_clocks
        )
        
#pico_add_extra_outputs(main)
