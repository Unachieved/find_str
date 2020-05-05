#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <mpi.h>
#include <string.h>
#include <math.h>

extern void initMaster(int rank, int nprocs, char * text, int * length);
char ** words_array = NULL;
extern void kernelCall(char ** array, int length, ushort threadsCount, int numBlocks);
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


void kernellLaunch(int rank, int nprocs, char ** array, int length, ushort threadsCount){

    //hello
	int numBlocks = (length + threadsCount -1 ) / threadsCount;
	kernellCall(array, length, threadsCount, numBlocks);
	
}

int main(int argc, char** argv) {

	setvbuf(stdout, NULL, _IONBF, 0 );

	if(argc<4){
		fprintf(stderr, "Process 0: Invalid number of arguments\nPlease input as such:\n\t./a.out <file_name> <num_threads>\n");
	}
	
	int rank, nprocs;
    char * file_name = argv[1];
	int threadsCount = atoi(argv[2]);
	char * text;
    int length;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	double start = MPI_Wtime();
	read_file(rank, nprocs, &text, file_name);
	double end = MPI_Wtime();

	printf("Parallel I/O reading time: %f\n", (end - start));

	initMaster(rank, nprocs, text, &length);
	kernellLaunch(rank, nprocs, words_array, length, threadsCount);


	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Finalize();
}