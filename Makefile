ARCH=$(shell arch)

maddotz: bin/${ARCH}/maddotz
clean:
	rm -Rf bin/${ARCH}

bin/x86_64/maddotz: src/MadDotz.cpp src/MadDotz.hpp
	mkdir -p bin/x86_64
	g++ -m64 src/MadDotz.cpp `sdl2-config --cflags --libs` -lSDL2_image -o bin/x86_64/maddotz  
	strip bin/x86_64/maddotz

bin/i686/maddotz: src/MadDotz.cpp src/MadDotz.hpp
	mkdir -p bin/i686
	g++ -m32 src/MadDotz.cpp `sdl2-config --cflags --libs` -lSDL2_image -o bin/i686/maddotz  
	strip bin/i686/maddotz

web/maddotz.js: src/MadDotz.cpp src/MadDotz.hpp
	mkdir -p web
	em++ -s USE_SDL=2 -s USE_SDL_IMAGE=2 src/MadDotz.cpp --preload-files res/dotz24.png -o web/maddotz.js
