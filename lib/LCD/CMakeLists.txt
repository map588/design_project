
aux_source_directory(. DIR_LCD_SRCS)

include_directories(../Config)

add_library(LCD INTERFACE)

target_sources(LCD INTERFACE ${DIR_LCD_SRCS})

target_link_libraries(LCD INTERFACE Config GUI pico_stdlib)