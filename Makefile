CC = g++
CFLAGS=$(shell pkg-config --cflags --libs gtk+-3.0) -std=c++23 -g

all: simfibre

constexpr.o: constexpr.cpp constexpr.hpp
	$(CC) -c -o constexpr.o constexpr.cpp $(CFLAGS)

effects.o: effects.c simfibre_config.h
	$(CC) -c -o effects.o effects.c $(CFLAGS)

fast_hsv2rgb.o: fast_hsv2rgb.c fast_hsv2rgb.h
	$(CC) -c -o fast_hsv2rgb.o  fast_hsv2rgb.c $(CFLAGS)

simfibre: simfibre.c fast_hsv2rgb.o effects.o constexpr.o
	$(CC) -o simfibre fast_hsv2rgb.o effects.o constexpr.o simfibre.c -g $(CFLAGS) 

clean:
	rm simfibre *.o
