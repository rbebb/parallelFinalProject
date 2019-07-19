#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

double func(double x);

int main(int argc, char**argv) {
    int rank, numRanks;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status stat;

    int n = 0;
    double a = 0;
    double b = 0;

    // Assume only rank 0 has these values to start with
    if (rank == 0) {
        n = 100000;
        a = 1;
        b = 2;
    }

    // Send the 3 initial values to all threads
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&a, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&b, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double width = (b - a) / n;
    double result = 0;
    double myResult = 0;

    int numele = n / (numRanks - 1);
    int myStart = 0;
    int myEnd = 0;

    // Master section (mostly sits around and does nothing)
    if (rank == 0) {
        // Initial sends (use numRanks-1 because rank 0 is the master rank)
        for (int i = 1; i < numRanks; i++) {
            myStart = i * numele + 1;
            myEnd  = myStart + numele - 1;

            if (i == numRanks-1) {
                myEnd = n;
            }
            
            printf("Rank: %d, myStart: %d, myEnd %d\n", rank, myStart, myEnd);

            MPI_Send(&myStart, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&myEnd, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        // Recv area from workers (start at 1 because rank 0 is the master rank)
        for (int i = 1; i < numRanks; i++) {
            MPI_Recv(&myResult, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);
            result += myResult;
        }

        printf("\nIntegral of x^2 from 1 to 2: %f\n\n", result);
    }

    // Worker section
    if (rank != 0) {
        MPI_Recv(&myStart, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);
        MPI_Recv(&myEnd, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);
        
        // Sum of height
        for (int i = myStart; i < myEnd; i++) {
            myResult += func(a+(i-1)*width);
        }

        // Multiply by widths
        myResult = myResult*width;

        MPI_Send(&myResult, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}

double func(double x) {
    return x * x;
}
