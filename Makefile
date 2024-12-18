CFLAGS = -lvulkan -lglfw
OUTFILE = game.x

build:
	gcc ./src/*.cpp $(CFLAGS) -o $(OUTFILE)

run: build
	./$(OUTFILE)

