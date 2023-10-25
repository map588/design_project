
aux_source_directory(. DIR_GUI_SRCS)

add_library(GUI INTERFACE)

include_directories(../Config)
include_directories(../Fonts)

target_sources(GUI INTERFACE ${DIR_GUI_SRCS})

target_link_libraries(GUI Config Fonts)