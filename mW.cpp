#include <iostream>
#include <cstdlib>
#include <omp.h>
#include "mpi.h"

int main(int argc, char** argv) {

	// Start up MPI
	int numranks, myrank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Status status;
	MPI_Request request;

	// Create our kernel for all ranks
	int kernel_dimensions = 20;
	int* kernel = (int*) malloc(kernel_dimensions * kernel_dimensions * sizeof(int));
	for (int i = 0; i < kernel_dimensions * kernel_dimensions; i++) {
		kernel[i] = 0; //change this inner bit to match our desired kernel
	}

	//
	int* mat;
	int* dims = (int*) malloc(2*sizeof(int));
	int* answer;

	if (myrank == 0) {
		int num_frames, current_frame;
		//OPENCV CALLS to open the video, get number of frames, get dimensions
		mat = (int*) malloc(dims[0] * dims[1] * sizeof(int));
		answer = (int*) malloc(dims[0] * dims[1] * sizeof(int));

		MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);//make sure everyone knows the resolution

		for (int i = 1; i < numranks; i++) {
			mat = 0; //getter method for a frame's matrix
			MPI_Isend(mat, dims[0] * dims[1], MPI_INT, i, 0, MPI_COMM_WORLD, &request);
			current_frame++;
		}


		while (1) {
			MPI_Recv(answer, dims[0] * dims[1], MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			// Set the new image as the appropriate frame somewhere new?
			if (current_frame < num_frames) {
				mat = 0; //method call to get next mat from video file
				MPI_Isend(mat, dims[1] * dims[0], MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, & request);
				//mayhaps an array where a ranks index holds the number of the frame they're convolving?
				current_frame++;
			}
			else {
				//something to do with saving the final product?
				break;
			}
		}

	}

	if (numranks != 0) {
		MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);//make sure everyone knows the resolution
		mat = (int*)malloc(dims[0] * dims[1] * sizeof(int));
		answer = (int*)malloc(dims[0] * dims[1] * sizeof(int));
		while (1) {
			MPI_Recv(mat, dims[0] * dims[1], MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
			// Method call to perform convolution with mat and kernel
			// answer = convolve(mat, kernel);
			MPI_Send(answer, dims[0] * dims[1], MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}




	MPI_Finalize();
	return 0;
}