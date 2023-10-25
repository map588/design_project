# add source files
add_executable(main
  main.c
  display_manager.c
)

# add include paths
target_include_directories(main PRIVATE
    ../include
    ../lib/Config
    ../lib/Fonts
    ../lib/GUI
    ../lib/LCD
)


# preprocessor macros
target_compile_definitions(main PRIVATE 
    PICO_HEAP_SIZE=2048
    PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
)


# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)
pico_add_dis_output2(main)
  
target_link_libraries(main PUBLIC
    pico_multicore
    pico_stdlib
    pico_rand
    hardware_spi
    hardware_timer
    Config
    LCD
    GUI
    Fonts
    )

# linker options
target_link_options(main 
    PRIVATE "LINKER:--print-memory-usage"
)