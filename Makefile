CC = gcc
CFLAGS=$(shell pkg-config --cflags --libs gtk+-3.0)

all: simfibre

fast_hsv2rgb.o: fast_hsv2rgb.c fast_hsv2rgb.h
	$(CC) -c -o fast_hsv2rgb.o  fast_hsv2rgb.c
simfibre: simfibre.c fast_hsv2rgb.o
	$(CC) -o simfibre fast_hsv2rgb.o simfibre.c -g $(CFLAGS) 

clean:
	rm simfibre fast_hsv2rgb.o
