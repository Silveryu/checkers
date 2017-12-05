CC = g++
CFLAGS = -g -Wall

SRCS = src/main.cpp
PROG = bin/main

OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

