test: build run

build:
	g++ -o sdl_app main.cpp -lSDL2 -lSDL2_ttf

run:
	./sdl_app