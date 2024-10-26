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

all: shapes lines

shapes: shapes.c ../../src/fenster/fenster.h
	$(CC) shapes.c -I../../src/fenster/ -o $@ $(CFLAGS) $(LDFLAGS)

lines: lines.c ../../src/fenster/fenster.h
	$(CC) lines.c -I../../src/fenster/ -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f shapes lines
