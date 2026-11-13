CC = gcc
NVCC = nvcc
CFLAGS = -std=c11 -Wall -Wextra -O2
CPPFLAGS = -I. -Iheaders
LDLIBS = -lm
OMPFLAGS = -fopenmp

# Build with:  make USE_CUDA=1      (Linux)
# On Windows the CUDA version is built with build_cuda.bat (MSVC + nvcc).
USE_CUDA ?= 0

ifeq ($(OS),Windows_NT)
TARGET = main.exe
else
TARGET = main
endif

OBJS = main.o src/image.o src/sequential_blur.o src/blur.o src/tui.o src/openmp_blur.o src/metrics.o

ifeq ($(USE_CUDA),1)
ifeq ($(OS),Windows_NT)
$(error On Windows, build the CUDA version with build_cuda.bat; MinGW cannot link nvcc/MSVC objects)
endif
ifeq ($(strip $(CUDA_PATH)),)
$(error CUDA_PATH is not set. Install the CUDA toolkit or pass CUDA_PATH=...)
endif
CPPFLAGS += -DUSE_CUDA
OBJS += src/cuda_blur.o
LDLIBS += -L"$(CUDA_PATH)/lib64" -lcudart
NVCCFLAGS = -O2 -arch=native
# If your toolkit is older than the GPU, use NVCCFLAGS="-O2 -arch=sm_89"
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(OMPFLAGS) -o $@ $(LDLIBS)

main.o: main.c headers/blur.h headers/cuda_blur.h headers/image.h headers/metrics.h headers/openmp_blur.h headers/sequential_blur.h headers/tui.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c main.c -o $@

src/image.o: src/image.c headers/image.h stb_image.h stb_image_write.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/image.c -o $@

src/sequential_blur.o: src/sequential_blur.c headers/sequential_blur.h headers/blur.h headers/image.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/sequential_blur.c -o $@

src/blur.o: src/blur.c headers/blur.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/blur.c -o $@

src/tui.o: src/tui.c headers/tui.h headers/blur.h headers/image.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/tui.c -o $@

src/openmp_blur.o: src/openmp_blur.c headers/openmp_blur.h headers/blur.h headers/image.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OMPFLAGS) -c src/openmp_blur.c -o $@

src/metrics.o: src/metrics.c headers/metrics.h headers/image.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/metrics.c -o $@

# nvcc compiles the kernel; main.o needs -DUSE_CUDA (set above) to enable case 4
src/cuda_blur.o: src/cuda_blur.cu headers/cuda_blur.h headers/blur.h headers/image.h
	$(NVCC) $(CPPFLAGS) $(NVCCFLAGS) -c src/cuda_blur.cu -o $@

clean:
	-rm -f $(TARGET) main main.exe main.o src/*.o src/cuda_blur.obj
	-cmd /c "del /Q /F $(TARGET) *.o src\*.o 2>NUL & if exist obj rmdir /s /q obj"

.PHONY: all clean
