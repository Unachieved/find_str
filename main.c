#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <mpi.h>
#include <string.h>
#include <math.h>

int count =0;
extern void initMaster(int rank, int nprocs, char * text, int * length);
char ** words_array = NULL;
extern void kernelCall(char ** array, int length, ushort threadsCount, int numBlocks, char ** to_find, int find);
void ErrorMessage(int error, int rank, char* string){
          fprintf(stderr, "Process %d: Error %d in %s\n", rank, error, string);
          MPI_Finalize();
          exit(-1);
}

/*void write_file(char * file_name){
	MPI_File file;
	MPI_Status status;
	int my_size1, error;
	MPI_Offset filesize, end, start;


	error = MPI_File_open(MPI_COMM_WORLD, file_name,
                  MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file);
    if(error != MPI_SUCCESS) ErrorMessage(error, rank, "MPI_File_open");

    error = MPI_File_write_at(file, start, buf, end-start, MPI_BYTE, &status);
    if(error != MPI_SUCCESS) ErrorMessage(error, rank, "MPI_File_write");*/

    /* close the file */
   /* MPI_File_close(&file);
}*/
ssize_t read_file(int rank, int nprocs, char ** buf, char * file_name){
	MPI_File file;
	MPI_Status status;
	int my_size, error;
	MPI_Offset filesize, end, start;

	error = MPI_File_open(MPI_COMM_WORLD, file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
	if(error != MPI_SUCCESS){
		printf( "Unable to open file \"temp\"\n" );fflush(stdout);
	}

	error = MPI_File_get_size(file, &filesize);
	if(error != MPI_SUCCESS) ErrorMessage(error, rank, "MPI_File_get_size");

	my_size = filesize/nprocs;
	start = rank * my_size;

	if (rank == nprocs-1)
        end = filesize;
    else
    	end = start + my_size;

	fprintf(stdout, "Proc %d: range = [%lld, %lld)\n", rank, start, end);

	(*buf) = (char *)calloc((end-start), sizeof(char));
	if ((*buf) == NULL) ErrorMessage(-1, rank, "malloc");

	MPI_File_seek(file, start, MPI_SEEK_SET);
	error = MPI_File_read(file, (*buf), end-start, MPI_BYTE, &status);
	if(error != MPI_SUCCESS) ErrorMessage(error,rank, "MPI_File_read");

	MPI_File_close(&file);
	printf("rank %d, file: %s\n", rank, (*buf));

	return (ssize_t)end-start;
}


void kernellLaunch(int rank, int nprocs, char ** array, int length, ushort threadsCount, int find, char ** to_find){

    //hello
	int ierr;
	MPI_Request request_send = MPI_REQUEST_NULL;
	MPI_Request request_recv = MPI_REQUEST_NULL;
	
		 
	if(nprocs>1 && find>1){
		char ** data_last = (char **)calloc(find, sizeof(char *));
		char ** data_before = (char **)calloc(find, sizeof(char *));

		int found = 0;
		while(found<find){
			char * tmp = calloc(strlen(array[length+found-find]), sizeof(char));
			strcpy(tmp, array[length+found-find]));
			data_last[found] = tmp;
		}
			 
		if(rank<nprocs-1)
			MPI_Isend(data_last, find, MPI_CHAR, (my_rank+1), 0, MPI_COMM_WORLD, &request_send);
			 
		//recieved the last row of the previous rank and the first row of the next rank --- to populate this ranks ghost cells
		if(rank >0)
			MPI_Irecv(data_before, find, MPI_CHAR, (my_rank-1), MPI_ANY_TAG, MPI_COMM_WORLD, &request_recv);
			 
		//The following are waits to make sure that the data was sent and recieved as expected
		ierr=MPI_Wait(&request_recv, &status);
		Ierr=MPI_Wait(&request_send, &status);
			 
			 //place the recieved ghost rows in their proper positions in the local rank
		for(j=0; j < find; ++j){
			array[j] = data_before[j]; 
		}
			
		free(data_last);
		free(ghost_before);

	}
	
	int numBlocks = length + threadsCount -1 ) / threadsCount;
	kernellCall(array, length, threadsCount, numBlocks, to_find, find);
	cudaDeviceSynchronize();
	
}

int main(int argc, char** argv) {

	setvbuf(stdout, NULL, _IONBF, 0 );

	if(argc<4){
		fprintf(stderr, "Process 0: Invalid number of arguments\nPlease input as such:\n\t./a.out <file_name> <to_find> <num_threads>\n");
	}
	
	int rank, nprocs;
    char * file_name = argv[1];
	char * input = argv[2];
	int threadsCount = atoi(argv[3]);
	char * text;
    int length, input_len;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	char ** to_find = parse_read(input, &input_len, 0);


	double start = MPI_Wtime();
	read_file(rank, nprocs, &text, file_name);
	double end = MPI_Wtime();

	printf("Parallel I/O reading time: %f\n", (end - start));

	initMaster(rank, nprocs, text, &length, input_len);
	kernellLaunch(rank, nprocs, words_array, length, threadsCount, to_find, input_len);


	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Finalize();
}