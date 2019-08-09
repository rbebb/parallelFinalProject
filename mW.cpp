#include <iostream>
#include <cstdlib>
#include <omp.h>
#include "mpi.h"
#include "convolution.hpp"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/video/video.hpp"

using namespace cv;
using namespace std;

int* getFrame(VideoCapture videoIn, int* dims, int frame);
void matToVideo(string filename, int* mat, int* dims, int frameCount);
int frameCount(VideoCapture video);

int main(int argc, char** argv) {

	// Start up MPI
	int numranks, myrank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Status status;
	// MPI_Request request;

	// Create our kernel for all ranks
	int kernel_dimensions = 20;
	double** kernel = (double**) malloc(kernel_dimensions * kernel_dimensions * sizeof(double*));
	for (int i = 0; i < kernel_dimensions; i++) {
		kernel[i] = (double*) malloc(2 * sizeof(double)); //change this inner bit to match our desired kernel
	}
	double value = 1.0/9.0;
    for(int i = 0; i < kernel_dimensions; i++) {
        for (int j = 0; j < kernel_dimensions; j++) {
            kernel[i][j] = value;
        }
    }

	//
	int* mat;
	int* dims = (int*) malloc(2*sizeof(int));
	int* answer;

	if (myrank == 0) {
		int num_frames, current_frame;

		//OPENCV CALLS to open the video, get number of frames, get dimensions
		VideoCapture videoIn("../../test.mp4");

		// Check if file can open
		if (!videoIn.isOpened()) {
			cout <<  "Error opening file" << std::endl;
		}

		num_frames = frameCount(videoIn);

		int *mat;
		answer = (int*) malloc(dims[0] * dims[1] * sizeof(int));

		MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);//make sure everyone knows the resolution

		for (int i = 1; i < numranks; i++) {
			// Getter method for a frame's matrix
			mat = getFrame(videoIn, dims, current_frame);
			MPI_Send(mat, dims[0] * dims[1], MPI_INT, i, 0, MPI_COMM_WORLD);
			current_frame++;
		}

		VideoWriter videoOut("output.mp4", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(dims[1], dims[0]));

		while (1) {
			for (int i = 1; i < numranks; i++) {
				MPI_Recv(answer, dims[0] * dims[1], MPI_INT, i, 0, MPI_COMM_WORLD, &status);

				Mat frame;
				int *matrix = (int*) malloc(dims[0]*dims[1]*sizeof(int*));
				for (int i = 0; i < dims[0]; i++) {
					for (int j = 0; j < dims[1]; j++) {
						frame.at<uchar>(i, j) = (int) matrix[i*dims[1]*j];
					}
				}
				videoOut.write(frame);

				// Set the new image as the appropriate frame somewhere new?
				if (current_frame < num_frames) {
					// Method call to get next mat from video file
					mat = getFrame(videoIn, dims, current_frame);
					MPI_Send(mat, dims[1] * dims[0], MPI_INT, i, 0, MPI_COMM_WORLD);
					// Perhaps an array where a rank's index holds the number of the frame they're convolving?
					current_frame++;
				} else {
					// Something to do with saving the final product?
					break;
				}
			}
		}

		// Close VideoCapture and close all frames
    	videoIn.release();
		videoOut.release();
    	destroyAllWindows();

	}

	if (numranks != 0) {
		MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);//make sure everyone knows the resolution
		mat = (int*) malloc(dims[0] * dims[1] * sizeof(int));
		answer = (int*) malloc(dims[0] * dims[1] * sizeof(int));
		while (1) {
			MPI_Recv(mat, dims[0] * dims[1], MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
			answer = convolute_image(mat, dims, kernel, kernel_dimensions);
			// Method call to perform convolution with mat and kernel
			// answer = convolve(mat, kernel);
			MPI_Send(answer, dims[0] * dims[1], MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}




	MPI_Finalize();
	return 0;
}