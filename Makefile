CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
CPPFLAGS = -I. -Iheaders
LDLIBS = -lm

TARGET = main
OBJS = main.o src/image.o src/sequential_blur.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

main.o: main.c headers/image.h headers/sequential_blur.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c main.c -o $@

src/image.o: src/image.c headers/image.h stb_image.h stb_image_write.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/image.c -o $@

src/sequential_blur.o: src/sequential_blur.c headers/sequential_blur.h headers/image.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/sequential_blur.c -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
