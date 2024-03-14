CC = gcc
CFLAGS = -Wall -Wextra -ggdb3 -I./include
LIBS = -lmupdf -lmupdf-third -lfreetype -lharfbuzz -ljpeg -lpng -lz -lopenjp2 -ljbig2dec -lm
LDFLAGS = -L/usr/lib/x86_64-linux-gnu
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))
EXEC = plotgrep

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ /usr/lib/libmupdf.a /usr/lib/libmupdf-third.a $(LIBS) $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
	rm $(EXEC)
