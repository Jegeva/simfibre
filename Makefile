CC = g++
CFLAGS=$(shell pkg-config --cflags --libs gtk+-3.0)

all: simfibre

effects.o: effects.c simfibre_config.h
	$(CC) -c -o effects.o effects.c

fast_hsv2rgb.o: fast_hsv2rgb.c fast_hsv2rgb.h
	$(CC) -c -o fast_hsv2rgb.o  fast_hsv2rgb.c

simfibre: simfibre.c fast_hsv2rgb.o effects.o
	$(CC) -o simfibre fast_hsv2rgb.o effects.o simfibre.c -g $(CFLAGS) 

clean:
	rm simfibre *.o
