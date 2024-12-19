CFLAGS = -Wall -Wextra -lvulkan -lglfw
OUT_DIR = bin
OUT_FILE = $(OUT_DIR)/game.x


build:
	mkdir -p $(OUT_DIR)
	gcc ./src/*.c $(CFLAGS) -o $(OUT_FILE)

run: build build-shader
	$(OUT_FILE)

build-shader:
	glslc ./src/shaders/first_frag.frag -o ./src/shaders/first_frag.frag.spv
	glslc ./src/shaders/first_vert.vert -o ./src/shaders/first_vert.vert.spv

