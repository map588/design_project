# Collect all the source files in the /src directory
file(GLOB SOURCES "*.c")

# Specify the executable and its source files
add_executable(bomb_it ${SOURCES})