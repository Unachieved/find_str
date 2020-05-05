#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include<bits/stdc++.h> 


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

    int i = 0;
    char *p = strtok (buf, " ");
    char ** array = calloc(100, sizeof(char *));

    while (p != NULL)
    {
        (*array) = calloc(strlen(p), sizeof(char));
        array[i++] = p;
        p = strtok (NULL, " ");
    }

    
    //cudaMallocManaged(&dist_info.dp_array, num_files * num_files * tot_length*tot_length*sizeof(int));
	//cudaMallocManaged(&dist_info.dists, num_files*num_files*sizeof(int));
	

    
}