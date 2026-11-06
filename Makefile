CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
CPPFLAGS = -I. -Iheaders
LDLIBS = -lm

TARGET = main
OBJS = main.o src/image.o src/sequential_blur.o src/blur.o src/tui.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

main.o: main.c headers/blur.h headers/image.h headers/sequential_blur.h headers/tui.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c main.c -o $@

src/image.o: src/image.c headers/image.h stb_image.h stb_image_write.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/image.c -o $@

src/sequential_blur.o: src/sequential_blur.c headers/sequential_blur.h headers/blur.h headers/image.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/sequential_blur.c -o $@

src/blur.o: src/blur.c headers/blur.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/blur.c -o $@

src/tui.o: src/tui.c headers/tui.h headers/blur.h headers/image.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/tui.c -o $@

clean:
	rm -f main main.o src/image.o src/sequential_blur.o src/blur.o src/tui.o

.PHONY: all clean
