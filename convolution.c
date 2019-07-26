#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "mpi.h"

extern int* imageToMat(char* name, int* dims);
extern void matToImage(char* name, int* mat, int* dims);

void matxmat(int* img, double* kern, int* fin, int* dim, int klen, int st, int end) {

	int kC = klen / 2;

	//how can we include OMP here most effectively
	for (int i = st; i < end; i++) { //row
		for (int j = 0; j < dim[1]; j++) { //column
			for (int k = 0; k < klen; k++) { //k row
				for (int l = 0; l < klen; l++) { //k column
					int re = (k - kC) + i; //edge
					int ce = (l - kC) + j; //detection
					if (re > -1 && re < dim[0] && ce > -1 && ce < dim[1]) {
						fin[i * dim[1] + j] += (int)img[re * dim[1] + ce] * kern[k * klen + l];
						if (kern[k * klen + l] == 1) {//&& i*dim[0]+j%9 == 0){
							//printf("Adjusted Pixel %d Using OG Pixel %d and a kernal Value of %d\n", i*dim[0]+j, re*dim[0] + ce, kern[k*klen +l]);
						}
					}
				}
			}
		}
	}
}



int main(int argc, char** argv) {
	
	int rank, numranks;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double* kernal;
	char* file = (char*)malloc(20 * sizeof(char)); //each rank stores the file it's opening

	//send and recv the kernal



	if (rank == 0) {
		//do the thing needed to get list of image files from directory?
	}

	if (rank == 0) {
		for (int i = 0; i < ##NUMBER_OF_FILE##; i++) {
			//send the file names to the ranks 

			//recv a done call, know when to stop
		}
	}


	if (rank != 0) {
		//loop that checks if we are sent a stop instruction
			//recv image name
		
			int* mat;
			int* dims; 
			int kdim;//set equal to whatever sized kernal we use
			int* answer;
		
			dims = (int*)malloc(sizeof(int) * 2);
			mat = imageToMat(file, dims);
			answer = (int*)malloc(dims[0] * dims[1] * sizeof(int));



			matxmat(mat, kernal, answer, dims, kdim, start, end);

			matToImage(file, answer, dims);
			//send completion notification to master
	}
	MPI_Finalize();

	return 0;
}