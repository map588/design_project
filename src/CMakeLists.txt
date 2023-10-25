
aux_source_directory(. DIR_SRCS)


add_executable(main
        ${DIR_SRCs}
        )

include_directories(../lib/Config)
include_directories(../lib/GUI)
include_directories(../lib/LCD)




pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(main)

target_link_libraries(main PUBLIC display pico_stdlib pico_multicore pico_rand )