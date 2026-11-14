CC = gcc
NVCC = nvcc
CFLAGS = -std=c11 -Wall -Wextra -O2
CPPFLAGS = -I. -Iheaders
LDLIBS = -lm
OMPFLAGS = -fopenmp

# Build with:  make USE_CUDA=1      (Linux)
# On Windows the CUDA version is built with build_cuda.bat (MSVC + nvcc).
USE_CUDA ?= 0

# Build with:  make clean, then make USE_MPI=1
# Needs MPI installed: MS-MPI + the MSYS2 msmpi package on Windows, or
# OpenMPI/MPICH on Linux. Run with:  mpiexec -n 4 ./main.exe
# Run "make clean" when switching between a normal and an MPI build,
# because main.o has to be recompiled with or without -DUSE_MPI.
USE_MPI ?= 0

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

ifeq ($(USE_MPI),1)
# mpicc is gcc plus the MPI include path and library
CC = mpicc
CPPFLAGS += -DUSE_MPI
OBJS += src/mpi_blur.o
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(OMPFLAGS) -o $@ $(LDLIBS)

main.o: main.c headers/blur.h headers/cuda_blur.h headers/image.h headers/metrics.h headers/mpi_blur.h headers/openmp_blur.h headers/sequential_blur.h headers/tui.h
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

src/mpi_blur.o: src/mpi_blur.c headers/mpi_blur.h headers/blur.h headers/image.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/mpi_blur.c -o $@

# nvcc compiles the kernel; main.o needs -DUSE_CUDA (set above) to enable case 4
src/cuda_blur.o: src/cuda_blur.cu headers/cuda_blur.h headers/blur.h headers/image.h
	$(NVCC) $(CPPFLAGS) $(NVCCFLAGS) -x cu src/cuda_blur.cu -o $@

# A plain Windows prompt has no rm; MSYS2 and Linux do. In MSYS2 the "cmd /c"
# line must not run: "/c" is rewritten to "C:\" and cmd opens interactively.
ifeq ($(OS)$(MSYSTEM),Windows_NT)
clean:
	-cmd /c "del /Q /F $(TARGET) *.o src\*.o 2>NUL & if exist obj rmdir /s /q obj"
else
clean:
	-rm -f $(TARGET) main main.exe main.o src/*.o src/cuda_blur.obj
endif

.PHONY: all clean
