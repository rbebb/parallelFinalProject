#include <iostream>
#include <cstdlib>
#include <string>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/video/video.hpp"
// #include "opencv2/imgcodecs/imgcodecs.hpp"

using namespace cv;
using namespace std;

int* getFrame(VideoCapture videoIn, int* dims, int frame) {
    videoIn.set(CV_CAP_PROP_POS_FRAMES, frame);
    videoIn >> frame;

    Mat grayImage;
    cvtColor(frame, grayImage, CV_BGR2GRAY);

    int width = grayImage.size().width;
    int height = grayImage.size().height;
    dims[0] = height;
    dims[1] = width;
 
    // Allocate 2D array
    int *matrix;
    matrix = (int*) malloc(height*width*sizeof(*matrix));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int intensity = grayImage.at<uchar>(i,j);
            if (intensity > 254) {
                intensity = 254;
            }
            if (intensity < 0) {
                intensity = 0;
            }
            matrix[i*width+j] = intensity;
        }
    }

    return matrix;
}

void matToVideo(string filename, int* mat, int* dims, int frameCount) {
    // Place this line outside of function and pass in VideoWriter object?
    VideoWriter videoOut("output.mp4", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(dims[1], dims[0]));

    int height = dims[0];
    int width = dims[1];

    Mat frame;

    for (int i = 0; i < frameCount; i++) {
        int *matrix;
        matrix = (int*) malloc(height*width*sizeof(*matrix));
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                frame.at<uchar>(i,j) = (int) matrix[i*width*j];
            }
        }
        videoOut.write(frame);
    }

    videoOut.release();
    destroyAllWindows();

    return;
}

int frameCount(VideoCapture video) {
    return video.get(CV_CAP_PROP_FRAME_COUNT);
}

/*
void matToImage(string filename, int* mat, int* dims){
    int height=dims[0];
    int width=dims[1];
    Mat image(height, width, CV_8UC1, Scalar(0,0,0));

    for(int i=0;i<height;i++){
        for(int j=0;j<width;j++){
            image.at<uchar>(i,j) = (int)mat[i*width+j];
        }
    } 
    //namedWindow( "Processed Image", WINDOW_AUTOSIZE );// Create a window for display.

    //imshow( "Processed Image", image );                   // Show our image inside it.
    imwrite(filename,image);
    //waitKey(0);                                          // Wait for a keystroke in the window
    return;
}
/*
