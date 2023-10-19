cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH

include(pico_sdk_import.cmake)

project(BOMB_IT)

pico_sdk_init()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src)

target_link_libraries(bomb_it pico_stdlib)

#pico_add_extra_outputs(bomb_it)
