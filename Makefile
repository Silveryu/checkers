CC = g++
CFLAGS = -g -Wall -Wextra

SRCS = src/main.cpp src/game.cpp
PROG = bin/main

OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)

.PHONY: footage

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

footage:
	mkdir -p bin
	youtube-dl https://www.youtube.com/watch?v=9Maesq2iISM -o "bin/footage.%(ext)s"

