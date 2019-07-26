#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "convolution.h"

/**
 * Function that applies a supplied kernel to a supplied image.
 * 
 * @param *img buffer containing image data
 * @param *dims array of length 2 holding height and width of the image
 * @param **kernel odd-sized square matrix holding transformation kernel
 * @param k width of the kernel
 * @return *filtered filtered image
 */
int *convolute_image(int *img, int *dims, double **kernel, int k) {
    int a;
    int *filtered;

    filtered = (int *) malloc(dims[0] * dims[1] * sizeof(int));

    #pragma omp parallel for shared(filtered)
    for(int i = 0; i < dims[0]; i++) {
        for(int j = 0; j < dims[1]; j++) {
            a = 0.0;

            for(int ki = -k; ki <= k; ki++) {
                for(int kj = -k; kj <= k; kj++) {
                    if((i + ki < 0) || (j + kj < 0) || (i + ki > dims[0] - 1) || (j + kj > dims[1] - 1)) {
                        continue;
                    } else {
                        a += kernel[kj + 1][ki + 1] * img[(i + ki) * dims[1] + (j + kj)];
                    }
                }
            }

            if(a > 255) a = 255;
            if(a < 0) a = 0;
            filtered[(i * dims[1]) + j] = (int) a;
        }
    }

    return filtered;
}