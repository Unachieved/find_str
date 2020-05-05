#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include<bits/stdc++.h> 
#define MAX_ARGS 100000

__global__ int* count;

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
    
#ifdef DEBUG_MODE
    for(int i=0; i<len;i++) {printf("debug: %s\n", (array)[i]);}
#endif
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
	
	*count = 0;
   
    words_array = parse_read(text, length, in_len);
   
}

extern "C" void initMaster(int rank, int nprocs, char * text, int * length, int in_len){
    cudaInitMaster(rank, nprocs, text, length, in_len);
}


__global__ void countSubstring(char** string, char* sub, int length) {

	for (int x = 0; x < length; x++) {
		if (strcmp(string[x], sub) == 0) {
			*count += 1;
		}
	}

}

extern "C" int* getCount(){
	return count;
}

extern "C" void kernelCall(char ** array, int length, ushort threadsCount, int numBlocks, char ** to_find, int find){
	countSubstring<<<numBlocks, threadsCount>>>(array, length, to_find, find);
}

