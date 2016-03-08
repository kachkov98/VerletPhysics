all: physics

physics: main.o window.o physics.o log.o loader.o
	g++ main.o window.o physics.o log.o loader.o -lGL -lGLU -lglut -o physics

main.o: main.cpp
	g++ -O2 -c main.cpp -mfpmath=sse

window.o: window.cpp
	g++ -O2 -c window.cpp -mfpmath=sse

physics.o: physics.cpp
	g++ -O2 -c physics.cpp -mfpmath=sse

log.o: log.cpp
	g++ -O2 -c log.cpp -mfpmath=sse

loader.o: loader.cpp
	g++ -O2 -c loader.cpp -mfpmath=sse