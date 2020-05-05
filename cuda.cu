#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include<bits/stdc++.h> 

int count;

static inline void genInitMaster(){

    int cE, cudaDeviceCount;
	if((cE = cudaGetDeviceCount( &cudaDeviceCount)) != cudaSuccess ){
		printf(" Unable to determine cuda device count, error is %d, count is %d\n",
			cE, cudaDeviceCount );
		exit(-1);
	}
	if((cE = cudaSetDevice( myrank % cudaDeviceCount )) != cudaSuccess ){
		printf(" Unable to have rank %d set to cuda device %d, error is %d \n",
			myrank, (myrank % cudaDeviceCount), cE);
		exit(-1);
    }
	
	count = 0;
   
}

__device__ checkSubstring(char* string, char* sub, int pos){
	
	for (int y = 0; y < strlen(sub); y++) {
		if (string[pos + y] == sub[y]) {
			continue;
		}
		else{
			return 0;
		}
	}
	return 1;

}

__global__ void countSubstring(char* string, char* sub) {

	for (int x = 0; x < (strlen(string) - strlen(sub)); x++) {
		count += checkSubstring(string, sub, x);
	}

}

extern "C" int getCount(){
	return count;
}