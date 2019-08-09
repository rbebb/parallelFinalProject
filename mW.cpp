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

// int* getFrame(VideoCapture videoIn, int* dims, int frame);
// void matToVideo(string filename, int* mat, int* dims, int frameCount);
// int frameCount(VideoCapture video);

int main(int argc, char** argv) {

	// Start up MPI
	int numranks, myrank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Status status;
	// MPI_Request request;

	// Create our kernel for all ranks
	int kernel_dimensions = 5;
	double** kernel = (double**) malloc(kernel_dimensions * sizeof(double*));
	for (int i = 0; i < kernel_dimensions; i++) {
		kernel[i] = (double*) malloc(kernel_dimensions * sizeof(double)); //change this inner bit to match our desired kernel
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
	dims[0] = 0;
	dims[1] = 0;
	int* answer;

	if (myrank == 0) {
		Vector<Mat> frames;
		int done = 0;
		int num_frames = 0;
		int current_frame = 0;

		int rowsSent = 0;
		int rowsReceived  = 0;

		//OPENCV CALLS to open the video, get number of frames, get dimensions
		VideoCapture videoIn("/home/bebbr/test.mp4");

		// Check if file can open
		if (!videoIn.isOpened()) {
			cout <<  "Error opening file" << std::endl;
		}

		num_frames = videoIn.get(CV_CAP_PROP_FRAME_COUNT);
		cout << num_frames << std::endl;

		Mat temp;
		videoIn >> temp;
		dims[0] = temp.size().height;
		dims[1] = temp.size().width;

		MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);//make sure everyone knows the resolution

		for (int i = 1; i < numranks; i++) {
			// Getter method for a frame's matrix
			// mat = getFrame(videoIn, dims, current_frame);


			videoIn.set(CV_CAP_PROP_POS_FRAMES, current_frame);

			Mat color;
			videoIn >> color;

			Mat grayImage;
			cvtColor(color, grayImage, CV_BGR2GRAY);

			int width = grayImage.size().width;
			int height = grayImage.size().height;
			dims[0] = height;
			dims[1] = width;
		
			// Allocate 2D array
			// int *mat = (int*) malloc(height*width*sizeof(*matrix));
			mat = (int*) malloc(height*width*sizeof(*mat));
			for (int j = 0; j < height; j++) {
				for (int k = 0; k < width; k++) {
					int intensity = grayImage.at<uchar>(j,k);
					if (intensity > 254) {
						intensity = 254;
					}
					if (intensity < 0) {
						intensity = 0;
					}
					mat[j*width+k] = intensity;
				}
			}

			cout << current_frame << std::endl;
			MPI_Send(mat, dims[0] * dims[1], MPI_INT, i, 0, MPI_COMM_WORLD);
			cout << myrank << "Receive" << std::endl;
			current_frame++;
			rowsSent++;
			free(mat);
		}

		VideoWriter videoOut("output.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(dims[1], dims[0]));
		
		int ranksWorking = numranks - 1;

		while (1) {
			for (int i = 1; i <= ranksWorking; i++) {
				answer = (int*) malloc(dims[0] * dims[1] * sizeof(int));
				MPI_Recv(answer, dims[0] * dims[1], MPI_INT, i, 0, MPI_COMM_WORLD, &status);
				rowsReceived++;

				Mat frame(dims[0], dims[1], CV_8UC1, Scalar(0, 0, 0));
				// int *matrix = (int*) malloc(dims[0]*dims[1]*sizeof(int*));
				for (int j = 0; j < dims[0]; j++) {
					for (int k = 0; k < dims[1]; k++) {
						frame.at<uchar>(j, k) = (int) answer[j*dims[1]+k];
					}
				}

				imwrite("test.jpg", frame);
				Mat img = imread("test.jpg");
				videoOut.write(img);
				
				// frames.push_back(img.clone());

				// videoOut.write(img);
				// free(matrix);
				free(answer);
			}


			if (rowsReceived >= num_frames-1) {
				mat = (int*) malloc(dims[0]*dims[1]*sizeof(*mat));
				mat[0] = -1;
				for (int i = 1; i < numranks; i++) {
					MPI_Send(mat, dims[1] * dims[0], MPI_INT, i, 0, MPI_COMM_WORLD);
				}
				free(mat);
				break;
			}

			ranksWorking = 0;
			// Set the new image as the appropriate frame somewhere new?
			for(int i = 1; i < numranks; i++) {
				if (rowsSent <= num_frames-1) {
					// Method call to get next mat from video file
					// mat = getFrame(videoIn, dims, current_frame);

					videoIn.set(CV_CAP_PROP_POS_FRAMES, current_frame);

					Mat color;
					videoIn >> color;

					Mat grayImage;
					cvtColor(color, grayImage, CV_BGR2GRAY);

					int width = grayImage.size().width;
					int height = grayImage.size().height;
					dims[0] = height;
					dims[1] = width;
				
					// Allocate 2D array
					// int *mat = (int*) malloc(height*width*sizeof(*matrix));
					mat = (int*) malloc(height*width*sizeof(*mat));
					for (int j = 0; j < height; j++) {
						for (int k = 0; k < width; k++) {
							int intensity = grayImage.at<uchar>(j,k);
							if (intensity > 254) {
								intensity = 254;
							}
							if (intensity < 0) {
								intensity = 0;
							}
							mat[j*width+k] = intensity;
						}
					}

					cout << current_frame << std::endl;
					MPI_Send(mat, dims[1] * dims[0], MPI_INT, i, 0, MPI_COMM_WORLD);
					current_frame++;
					rowsSent++;
					ranksWorking++;
					// Perhaps an array where a rank's index holds the number of the frame they're convolving?
					free(mat);
				}
			}
		}

		// for (int i = 0; i < num_frames; i++) {
		// 	videoOut.write(frames[i]);
		// }

		// Close VideoCapture and close all frames
    	videoIn.release();
		videoOut.release();
    	destroyAllWindows();

	}

	if (myrank != 0) {
		MPI_Bcast(dims, 2, MPI_INT, 0, MPI_COMM_WORLD);//make sure everyone knows the resolution
		mat = (int*) malloc(dims[0] * dims[1] * sizeof(int));
		answer = (int*) malloc(dims[0] * dims[1] * sizeof(int));
		while (1) {
			MPI_Recv(mat, dims[0] * dims[1], MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
			cout << myrank << "Receive" << std::endl;
			if (mat[0] == -1) {
				break;
			}

			answer = convolute_image(mat, dims, kernel, kernel_dimensions);
			// Method call to perform convolution with mat and kernel
			// answer = convolve(mat, kernel);
			MPI_Send(answer, dims[0] * dims[1], MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}




	MPI_Finalize();
	return 0;
}