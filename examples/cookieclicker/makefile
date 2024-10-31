CFLAGS ?= -mavx2 -lm -DUSE_FONTS -O3 -flto -Wall -Wextra  -std=c99
DESTDIR ?= .

ifeq ($(OS),Windows_NT)
	CC = gcc
	LDFLAGS = -mwindows -lgdi32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS = -framework Cocoa
	else
		LDFLAGS = -lX11
	endif
endif

all: main

main: main.c ../../src/fenster/fenster.h
	$(CC) main.c -I../../src/fenster/ -o $(DESTDIR)/$@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f main
