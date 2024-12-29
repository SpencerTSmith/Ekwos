cmake_minimum_required(VERSION 3.10)
project(game LANGUAGES C)

file(GLOB SOURCES "src/*.c")

# Create the executable
add_executable(game ${SOURCES})

# Set include directories (if any)
target_include_directories(game PRIVATE src)

target_link_libraries(game PRIVATE glfw vulkan)

target_compile_options(game PRIVATE
	-g
	-Wall
	-Wextra
	-Wshadow
	-Wpedantic
)

set_target_properties(game PROPERTIES
	C_STANDARD 17
)
