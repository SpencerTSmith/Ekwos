cmake_minimum_required(VERSION 3.10)
project(ekwos LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create the executable

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(ekwos ${SOURCES})

file(GLOB_RECURSE SHADER_SOURCES "src/shaders/*.vert" "src/shaders/*.frag")
make_directory(shaders)

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(OUT_FILE "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT ${OUT_FILE}
        COMMAND glslc ${SHADER} -o ${OUT_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${SHADER}"
    )
    list(APPEND COMPILED_SHADERS ${OUT_FILE})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${COMPILED_SHADERS})

add_dependencies(ekwos compile_shaders)

# Set include directories (if any)
target_include_directories(ekwos PRIVATE ./src)

target_link_libraries(ekwos PRIVATE 
	glfw
	vulkan
)

target_compile_options(ekwos PRIVATE
	-g
	-Wall
	-Wextra
	-Wshadow
	-Wpedantic
	-DDEBUG=1
)

set_target_properties(ekwos PROPERTIES
	C_STANDARD 17
)
