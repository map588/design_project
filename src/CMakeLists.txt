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
add_subdirectory(tone_generation)

target_link_libraries(main LINK_PUBLIC
    keyboard_lib
    pwm_tone
    pico_stdlib
    pico_stdio
    pico_printf
    pico_multicore
    pico_rand
    hardware_spi
    hardware_pio
    hardware_pwm
    Config
    LCD
    GUI
    Fonts
    )

target_compile_definitions(main PUBLIC
    PICO_CORE1_STACK_SIZE=4096
    PICO_RAND_ENTROPY_SRC_BUS_PERF_COUNTER=1
    PICO_RAND_SEED_ENTROPY_SRC_TIME=1
    PICO_RAND_SEED_ENTROPY_SRC_ROSC=1
    PICO_RAND_SEED_ENTROPY_SRC_RAM_HASH=1
    )
    
add_compile_options(CMAKE_EXPORT_COMPILE_COMMANDS=1 -M -Wall)

# disable usb output, enable uart output
pico_enable_stdio_usb(main 0)
pico_enable_stdio_uart(main 1)



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