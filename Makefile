CC = gcc
CFLAGS = -g -Wall -Wextra -Wshadow
LDFLAGS = -lvulkan -lglfw

SRC_DIR = src
OUT_DIR = bin
BUILD_DIR = build
OUT_FILE = $(OUT_DIR)/game.x


build: build-shader
	mkdir -p $(OUT_DIR)

	$(CC) $(SRC_DIR)/*.c $(CFLAGS) $(LDFLAGS) -o $(OUT_FILE)

run: build
	$(OUT_FILE)

build-shader:
	glslc ./src/shaders/frag.frag -o ./src/shaders/frag.frag.spv
	glslc ./src/shaders/vert.vert -o ./src/shaders/vert.vert.spv

