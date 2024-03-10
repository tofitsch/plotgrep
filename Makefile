CC = gcc
CFLAGS = -Wall -Wextra -I./include
LIBS = -lmupdf -lmupdf-third -lfreetype -lharfbuzz -ljpeg -lz -lopenjp2 -ljbig2dec -lm
LDFLAGS = -L/usr/lib/x86_64-linux-gnu
SRC = src/main.c src/bitmap.c
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))
EXEC = build/x

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ /usr/lib/libmupdf.a /usr/lib/libmupdf-third.a $(LIBS) $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
