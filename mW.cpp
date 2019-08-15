#include <iostream>
#include <cstdlib>
#include <string>
#include <cmath>
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

int main(int argc, char** argv) {

	// Start up MPI
	int numranks, myrank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Status status;

	// Create our kernel for all ranks
    // Change this value to set the size of our desired kernel
	int kernel_dimensions = 5;
	double** kernel = (double**) malloc(kernel_dimensions * sizeof(double*));
	for (int i = 0; i < kernel_dimensions; i++) {
		kernel[i] = (double*) malloc(kernel_dimensions * sizeof(double));
	}
	double value = 1.0/9.0;
    for(int i = 0; i < kernel_dimensions; i++) {
        for (int j = 0; j < kernel_dimensions; j++) {
            kernel[i][j] = value;
        }
    }

	int* mat;
	int* dims = (int*) malloc(2*sizeof(int));
	dims[0] = 0;
	dims[1] = 0;
	int* answer;

	if (myrank == 0) {
		int num_frames = 0;
		int current_frame = 0;

		int rowsSent = 0;
		int rowsReceived  = 0;

		// OpenCV calls to open the video, get number of frames, get dimensions
		VideoCapture videoIn("/home/bebbr/test.mp4");

		// Check if file can open
		if (!videoIn.isOpened()) {
			cout <<  "Error opening file" << std::endl;
		}

		num_frames = videoIn.get(CV_CAP_PROP_FRAME_COUNT);

		Mat temp;
		videoIn >> temp;
		dims[0] = temp.size().height;
		dims[1] = temp.size().width;

        // Make sure everyone knows the resolution
		MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);

		for (int i = 1; i < numranks; i++) {
            // Get current frame
			videoIn.set(CV_CAP_PROP_POS_FRAMES, current_frame);

			Mat color;
			videoIn >> color;

            // Convert to grayscale
			Mat grayImage;
			cvtColor(color, grayImage, CV_BGR2GRAY);

            // Get height and width of video
			int width = grayImage.size().width;
			int height = grayImage.size().height;
			dims[0] = height;
			dims[1] = width;
		
			// Allocate 2D array
			mat = (int*) malloc(height*width*sizeof(*mat));
			for (int j = 0; j < height; j++) {
				for (int k = 0; k < width; k++) {
					int intensity = grayImage.at<uchar>(j,k);
					if (intensity > 255) {
						intensity = 255;
					}
					if (intensity < 0) {
						intensity = 0;
					}
					mat[j*width+k] = intensity;
				}
			}

            // Send matrix to worker to eventually convolve the desired frame
			MPI_Send(mat, dims[0] * dims[1], MPI_INT, i, 0, MPI_COMM_WORLD);
			current_frame++;
			rowsSent++;
			free(mat);
		}

        // Set video output file name, encoding, FPS, and resolution
		VideoWriter videoOut("output.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(dims[1], dims[0]));
		
		int ranksWorking = numranks - 1;

		while (1) {
			for (int i = 1; i <= ranksWorking; i++) {
				answer = (int*) malloc(dims[0] * dims[1] * sizeof(int));
                // Receive the convolved frame from the worker
				MPI_Recv(answer, dims[0] * dims[1], MPI_INT, i, 0, MPI_COMM_WORLD, &status);
				rowsReceived++;

                // Convert array back to Mat object
				Mat frame(dims[0], dims[1], CV_8UC1, Scalar(0, 0, 0));
				for (int j = 0; j < dims[0]; j++) {
					for (int k = 0; k < dims[1]; k++) {
						frame.at<uchar>(j, k) = (int) answer[j*dims[1]+k];
					}
				}

                // Save video as image and save image to frame of video
				imwrite("test.jpg", frame);
				Mat img = imread("test.jpg");
				videoOut.write(img);
				
				free(answer);
			}


			if (rowsReceived >= num_frames-1) {
				mat = (int*) malloc(dims[0]*dims[1]*sizeof(*mat));
				mat[0] = -1;
				for (int i = 1; i < numranks; i++) {
                    // If there are no more frames to convolve, let the other ranks know
					MPI_Send(mat, dims[1] * dims[0], MPI_INT, i, 0, MPI_COMM_WORLD);
				}
				free(mat);
				break;
			}

			ranksWorking = 0;

			for(int i = 1; i < numranks; i++) {
				if (rowsSent <= num_frames-1) {
                    // Get current frame
					videoIn.set(CV_CAP_PROP_POS_FRAMES, current_frame);

					Mat color;
					videoIn >> color;

                    // Convert to grayscale
					Mat grayImage;
					cvtColor(color, grayImage, CV_BGR2GRAY);

                    // Get height and width of frame
					int width = grayImage.size().width;
					int height = grayImage.size().height;
					dims[0] = height;
					dims[1] = width;
				
					// Allocate 2D array
					mat = (int*) malloc(height*width*sizeof(*mat));
					for (int j = 0; j < height; j++) {
						for (int k = 0; k < width; k++) {
							int intensity = grayImage.at<uchar>(j,k);
							if (intensity > 255) {
								intensity = 255;
							}
							if (intensity < 0) {
								intensity = 0;
							}
							mat[j*width+k] = intensity;
						}
					}

                    // Send matrix to worker to eventually convolve the desired frame
					MPI_Send(mat, dims[1] * dims[0], MPI_INT, i, 0, MPI_COMM_WORLD);
					current_frame++;
					rowsSent++;
					ranksWorking++;
					free(mat);
				}
			}
		}

		// Close VideoCapture and close all frames
    	videoIn.release();
		videoOut.release();
    	destroyAllWindows();

	}

	if (myrank != 0) {
        // Make sure everyone knows the resolution
		MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
        // Array to store original frame
		mat = (int*) malloc(dims[0] * dims[1] * sizeof(int));
        // Array to store convolved frame
		answer = (int*) malloc(dims[0] * dims[1] * sizeof(int));
		while (1) {
            // Receive matrix to convolve from the master rank
			MPI_Recv(mat, dims[0] * dims[1], MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
			if (mat[0] == -1) {
				break;
			}

            // Method call to perform convolution with mat and kernel
			answer = convolute_image(mat, dims, kernel, kernel_dimensions);
            // Send convolved frame back to the master rank
			MPI_Send(answer, dims[0] * dims[1], MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}


	MPI_Finalize();
	return 0;
}
