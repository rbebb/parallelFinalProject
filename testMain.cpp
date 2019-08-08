#include <iostream>
#include <cstdlib>
#include <string>
#include "convolution.hpp"

using namespace std;

int* imageToMat(string filename, int* dims);
void matToImage(string filename, int* mat, int* dims);

//TODO: Document
int main(int argc, char **argv) {
	
    int *matrix;
    string name = "test.jpg";
    int *dims = (int*) malloc(2*sizeof(int));
    int total = 0;
    double **kernel = (double**) malloc(3*sizeof(double*));
    // double kernel[3][3] = {{1.0/9.0,1.0/9.0,1.0/9.0},{1.0/9.0,1.0/9.0,1.0/9.0},{1.0/9.0,1.0/9.0,1.0/9.0}}; // blur
    for(int i = 0; i < 3; i++) {
        kernel[i] = (double*) malloc(3 * sizeof(double));
    }
    double value = 1.0/9.0;
    for(int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = value;
        }
    }


    matrix = imageToMat(name, dims);
    int *resultFull = (int*) malloc(dims[0]*dims[1]*sizeof(*matrix));
    resultFull = convolute_image(matrix, dims, kernel, 3);

    matToImage("processedImage.jpg", resultFull, dims);
}