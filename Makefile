CC = gcc
CFLAGS = -g -Wall -Wextra -Wshadow
LDFLAGS = -lvulkan -lglfw

SRC_DIR = src
OUT_DIR = bin
BUILD_DIR = build
OUT_FILE = $(OUT_DIR)/game.x


build:
	mkdir -p $(OUT_DIR)

	$(CC) $(SRC_DIR)/*.c $(CFLAGS) $(LDFLAGS) -o $(OUT_FILE)

run: build build-shader
	$(OUT_FILE)

build-shader:
	glslc ./src/shaders/first_frag.frag -o ./src/shaders/first_frag.frag.spv
	glslc ./src/shaders/first_vert.vert -o ./src/shaders/first_vert.vert.spv

