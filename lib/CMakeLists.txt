
aux_source_directory(. DIR_LIB_SRCS)

add_library(display INTERFACE)

target_sources(display INTERFACE ${DIR_LIB_SRCS})

target_link_libraries(display INTERFACE Config Fonts GUI LCD pico_stdlib)