#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include<bits/stdc++.h> 
#define MAX_ARGS 100000

extern "C" int count;
extern "C" char ** words_array;

static inline char ** parse_read(char * buffer, int * Length, int in_len){

    char ** array;
    //array = (char **)calloc(MAX_ARGS, sizeof(char *));
    cudaMallocManaged(&array, MAX_ARGS*sizeof(int));
    int len=in_len;
    
    // delimit commands by space and newline
	char * pch;
    
    pch = strtok (buffer,",\n \r\n");
    while (pch != NULL){
        *(array+len)=(char *)calloc(50, sizeof(char));
        strcpy(*(array+len), pch);
        //printf("psh: %s\n", pch);
        len++;
        pch = strtok (NULL, ",\n \r\n");
    }
    
    array[len]=NULL; //NULL terminate array for use in execv
    *Length=len;
    return array;
}


static inline void cudaInitMaster(int rank, int nprocs, char * text, int * length, int in_len){

    int cE, cudaDeviceCount;
	if((cE = cudaGetDeviceCount( &cudaDeviceCount)) != cudaSuccess ){
		printf(" Unable to determine cuda device count, error is %d, count is %d\n",
			cE, cudaDeviceCount );
		exit(-1);
	}
	if((cE = cudaSetDevice( rank % cudaDeviceCount )) != cudaSuccess ){
		printf(" Unable to have rank %d set to cuda device %d, error is %d \n",
			rank, (rank % cudaDeviceCount), cE);
		exit(-1);
    }
    words_array = parse_read(text, length, in_len);
   
}

extern "C" void initMaster(int rank, int nprocs, char * text, int * length, int in_len){
    cudaInitMaster(rank, nprocs, text, length, in_len);
}


__global__ void countSubstring(char** string, int length, char** to_find, int find, int * counter) {
    unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int stride = blockDim.x * gridDim.x;

    for(unsigned int i = index; i<length; i+=stride){
        
        int found = 1;

        if(i+(2*stride)<length){
            
            for(int j =0; j<find;j++){
                if(string[i+(j*stride)] == to_find[j])
                    found = 0;
            }
        }

        if(found)*counter++;
    }
}

extern "C" void kernelCall(char ** array, int length, ushort threadsCount, int numBlocks, char ** to_find, int find, int * counter){
	countSubstring<<<numBlocks, threadsCount>>>(array, length, to_find, find, counter);
}

