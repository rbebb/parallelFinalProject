#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "mpi.h"

extern int* imageToMat(char* name, int* dims);
extern void matToImage(char* name, int* mat, int* dims);

int main(int argc, char** argv) {

	int rank, numranks;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Status stat;
	MPI_Request req;
	int* kernel;
	int kdim;//set equal to whatever sized kernal we use
	//malloc space for kernel
	//assign values for kernel

	int* mat;
	int* dims;
	int* answer

	int done = 0;
	int counter = 0;
	char* file = (char*)malloc(20 * sizeof(char));
	int x = rank;

	if (myranks == 0) {


		//openCV calls that get us all of our images for convolution
		done = 0; //change from zero to number of images that need convolution
		for (int i = 1; i < ranks; i++) {
			file = 'the counter index of our images'; //assume we have at least enough images for init
			counter++;
			MPI_Isend(file, 20, MPI_CHAR, i, 0, MPI_COMM_WORLD, req); //non-blocking so we can get on with sending the other file names for opening
		}

		while (1) {
			MPI_Recv(&x, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);//blocking to make sure we recv and send to the same worker 
			if (counter < done) {
				file = 'the counter index of our images';
				counter++;
				MPI_Isend(file, 20, MPI_CHAR, x, 0, MPI_COMM_WORLD, req);//non-blocking so that we can get on with recv and sending
			}
			else {
				int finished = 0;

				//some kind of end signal to the slaves
				break;
			}

		}


	}

	if (myrank != 0) {
		dims = (int*)malloc(sizeof(int) * 2);
		while (1) {
			MPI_Recv(file, 20, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &stat);//blocking because we cant do anything till we know what file to open
			mat = imageToMat(file, dims);
			answer = (int*)malloc(dims[0] * dims[1] * sizeof(int));
			//call convolution method
			matToImage(file, answer, dims);
			MPI_Send(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);//blocking because we want to wait till we finish this image before going to the next
		}
	}




	MPI_Finalize();

	return 0;
}