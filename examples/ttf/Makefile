CFLAGS ?= -ffast-math -mavx2 -lm -DUSE_FONTS -O3 -flto -Wall -Wextra -std=c99

ifeq ($(OS),Windows_NT)
	CC = gcc
	LDFLAGS = -lgdi32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS = -framework Cocoa
	else
		LDFLAGS = -lX11
	endif
endif

main: main.c ../../src/fenster/fenster.h
	$(CC) main.c -I../../src/fenster/ -o $@ $(CFLAGS) $(LDFLAGS)
