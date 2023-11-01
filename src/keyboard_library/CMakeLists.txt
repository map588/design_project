
add_library(keyboard_lib INTERFACE)

target_sources(keyboard_lib INTERFACE ${CMAKE_CURRENT_LIST_DIR}/keyboard.c)

pico_generate_pio_header (keyboard_lib ${CMAKE_CURRENT_LIST_DIR}/keyboard.pio)

target_link_libraries(keyboard_lib INTERFACE
            pico_stdlib 
            pico_stdio
            hardware_pio
            hardware_irq
            )

target_include_directories (keyboard_lib INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
            )