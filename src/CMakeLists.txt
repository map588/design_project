# add source files
add_executable(main
                main.c
                display_manager.c
            )


# add include paths
target_include_directories(main PUBLIC
    ../include
    ../lib/Config
    ../lib/Fonts
    ../lib/GUI
    ../lib/LCD
)

add_subdirectory(keyboard_library)

target_link_libraries(main LINK_PUBLIC
    pico_stdlib
    pico_stdio
    pico_printf
    pico_multicore
    pico_rand
    hardware_spi
    hardware_pio
    hardware_pwm
    keyboard_lib
    Config
    LCD
    GUI
    Fonts
    )

    target_compile_definitions(main PUBLIC
        PICO_CORE1_STACK_SIZE=4096
        )

# disable usb output, enable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)



    # create disassembly with source
function(pico_add_dis_output2 TARGET)
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -S $<TARGET_FILE:${TARGET}> >$<IF:$<BOOL:$<TARGET_PROPERTY:${TARGET},OUTPUT_NAME>>,$<TARGET_PROPERTY:${TARGET},OUTPUT_NAME>,$<TARGET_PROPERTY:${TARGET},NAME>>.dis2)
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND arm-none-eabi-size ${CMAKE_CURRENT_LIST_DIR}/../build/src/$<IF:$<BOOL:$<TARGET_PROPERTY:${TARGET},OUTPUT_NAME>>,$<TARGET_PROPERTY:${TARGET},OUTPUT_NAME>,$<TARGET_PROPERTY:${TARGET},NAME>>.elf
    VERBATIM
)
endfunction()



# create map/bin/hex file etc.
pico_add_extra_outputs(main)
pico_add_dis_output2(main)
  

# linker options
target_link_options(main 
    PRIVATE "LINKER:--print-memory-usage"
)