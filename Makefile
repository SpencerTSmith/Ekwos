CFLAGS = -lvulkan -lglfw
OUTFILE = game.x

build-shader:
	glslc ./src/shaders/first_frag.frag -o ./src/shaders/first_frag.frag.spv
	glslc ./src/shaders/first_vert.vert -o ./src/shaders/first_vert.vert.spv

build:
	mkdir ./bin
	gcc ./src/*.c $(CFLAGS) -o ./bin/$(OUTFILE)

run: build
	./bin/$(OUTFILE)


