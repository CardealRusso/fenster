CFLAGS ?= -Wall -Wextra -std=c99

ifeq ($(OS),Windows_NT)
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
	$(CC) main.c -I../../src/fenster -o $@ $(CFLAGS) $(LDFLAGS)
