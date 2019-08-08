#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/core/core.hpp"
//#include "opencv2/video/video.hpp"
//#include "opencv2/imgcodecs/imgcodecs.hpp"

using namespace cv;
using namespace std;

Vector<Mat> images;

int* videoLoader(char* filename, int* dims, int frame) {
    VideoCapture video(filename);

    // Check if file can open
    if (!video.isOpened()) {
        cout <<  "Error opening file" << std::endl;
    }

    video.set(CV_CAP_PROP_POS_FRAMES, frame);
    Mat frame;
    video >> frame;
    
    // Close VideoCapture and close all frames
    video.release();
    destroyAllWindows();


///////////////////////////////////////////////////////////////////
    int width=gray_image.size().width;
    int height=gray_image.size().height;
    dims[0]=height;
    dims[1]=width;

    cout << "Height: " << dims[0] << endl;
    cout << "Width : " << dims[1] << endl;
 
    // Allocate 2d array
    int *matrix;
    matrix = (int*) malloc(height*width*sizeof(*matrix));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int intensity=gray_image.at<uchar>(i,j);
            if (intensity > 255) {
                intensity = 255;
            }
            if (intensity < 0) {
                intensity = 0;
            }
            matrix[i*width+j] = intensity;
        }
    }

    return matrix;
}

///////////////////////////////////////////////////////////////////

/*
extern "C" void matToImage(char* filename, int* mat, int* dims){
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
*/
void matToVideo(char* filename, int* mat, int* dims){
    int height=dims[0];
    int width=dims[1];
    Mat video(height, width, CV_8UC1, Scalar(0,0,0));

    for(int i=0;i<height;i++){
        for(int j=0;j<width;j++){
            image.at<uchar>(i,j) = (int)mat[i*width+j];
        }
    } 

    // Save video
    imwrite(filename, video);

    return;
}

double frameCount(char* fileName) {
    VideoCapture video(fileName);
    return video.get(CV_CAP_PROP_FRAME_COUNT);
}