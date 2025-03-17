CC=gcc
CFLAGS=-g $(shell sdl2-config --cflags)
LDFLAGS=$(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf
TARGET=forkplate

all: $(TARGET)

$(TARGET): forkplate.c
	$(CC) $(CFLAGS) $< -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
