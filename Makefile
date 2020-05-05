all: main.c cuda.cu
		mpicc -g main.c -c -o main.o
		nvcc -g -G -arch=sm_70 cuda.cu -c -o cuda.o
		mpicc -g main.o cuda.o -o main-mpi-exe \
			-L/usr/local/cuda-10.1/lib64/ -lcudadevrt -lcudart -lstdc++