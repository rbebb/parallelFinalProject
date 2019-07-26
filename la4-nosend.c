#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

extern int *imageToMat(char *name, int *dims);
extern void matToImage(char *name, int *mat, int *dims);

int convolute(double kernel[3][3], int x, int y, int **img);

int KERNEL_SIZE = 3;

int main(int argc, char **argv) {
    int rank, num_ranks;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int *mat;
    // char *name = "image.jpg";
    char *name = "image.png";
    int *dims;
    dims = (int *)malloc(sizeof(*dims) * 2);

    mat = imageToMat(name, dims);
    // printf("Dims: %d %d\n", dims[0], dims[1]);

    int **realmat;
    realmat = malloc(dims[0] * sizeof(int*));
    for(int i = 0; i < dims[0]; i++) {
        realmat[i] = malloc(dims[1] * sizeof(int));
    }

    for(int i = 0; i < dims[0]; i++) {
        for(int j = 0; j < dims[1]; j++) {
            realmat[i][j] = mat[(i * dims[1] + j)];
        }
    }

    int howmanyrows[num_ranks];
    int rank_starts[num_ranks];
    int rank_ends[num_ranks];
    int gdispls[num_ranks];
    int howmanypixels[num_ranks];
    int runningpixels = dims[1];
    int num_rows = dims[0] / num_ranks;
    int remainder = dims[0] % num_ranks;
    // if(rank == 0) printf("height: %d; width: %d\n", dims[0], dims[1]);
    // if(rank == 0) printf("num ranks: %d; rows per rank: %d; remaining: %d\n", num_ranks, num_rows, remainder);

    for(int i = 0; i < num_ranks; i++) {
        howmanyrows[i] = num_rows;
    }
    howmanyrows[0]--;
    howmanyrows[num_ranks - 1]-=2;
    for(int i = remainder; i > 0; i--) {
        howmanyrows[i]++;
    }
    for(int i = 0; i < num_ranks; i++) {
        if(i == 0) {
            rank_starts[i] = 1;
            rank_ends[i] = rank_starts[i] + howmanyrows[i];
        } else {
            rank_starts[i] = rank_ends[i - 1];
            rank_ends[i] = rank_starts[i] + howmanyrows[i];
        }
        howmanypixels[i] = howmanyrows[i] * dims[1];
        gdispls[i] = runningpixels;
        runningpixels += howmanypixels[i];
    }

    // double kernel[3][3] = {{-1.0,-1.0,-1.0},{-1.0,8.0,-1.0},{-1.0,-1.0,-1.0}}; // laplacian / edge detection
    // double kernel[3][3] = {{0.0,-1.0,0.0},{-1.0,5.0,-1.0},{0.0,-1.0,0.0}}; // sharpen
    double kernel[3][3] = {{1.0/9.0,1.0/9.0,1.0/9.0},{1.0/9.0,1.0/9.0,1.0/9.0},{1.0/9.0,1.0/9.0,1.0/9.0}}; // blur
    // double kernel[3][3] = {{1.0/16.0,1.0/8.0,1.0/16.0},{1.0/8.0,1.0/4.0,1.0/8.0},{1.0/16.0,1.0/8.0,1.0/16.0}}; // gaussian blur

    //do stuff with the matrix
    double start_calc = MPI_Wtime();
    int a; // accumulator
    // printf("rank: %d; how many rows: %d; start at: %d; end at: %d\n", rank, howmanyrows[rank], rank_starts[rank], rank_ends[rank]);
    for (int i = rank_starts[rank]; i <= rank_ends[rank]; i++) { // image row (height)
        for(int j = 1; j < dims[1] - 1; j++) { // image column (width)
            a = 0.0;

            for(int k = -1; k <= 1; k++) { // kernel row
                for(int l = -1; l <= 1; l++) { // kernel column
                    a += kernel[l + 1][k + 1] * realmat[i - l][j - k];
                }
            }

            if(a > 255) a = 255;
            if(a < 0) a = 0;
            mat[(i * dims[1] + j)] = (int) a;
        }
    }
    double end_calc = MPI_Wtime();

    double start_gather = MPI_Wtime();
    int *rankstartptr = &mat[gdispls[rank]];
    MPI_Gatherv(rankstartptr, howmanypixels[rank], MPI_INT, mat, howmanypixels, gdispls, MPI_INT, 0, MPI_COMM_WORLD);
    double end_gather = MPI_Wtime();

    printf("rank: %d; calc time: %f; gather time: %f\n", rank, end_calc - start_calc, end_gather - start_gather);

    if(rank == 0) matToImage("processedImage.jpg", mat, dims);

    MPI_Finalize();
    return 0;
}
