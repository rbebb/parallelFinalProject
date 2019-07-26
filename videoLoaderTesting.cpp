/*
 * File is only used to test OpenCV
 */

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

int main() {
    VideoCapture video("../test.mov");

    // Check if file can open
    if (!video.isOpened()) {
        cout <<  "Error opening file" << std::endl;
    }

    double count = video.get(CV_CAP_PROP_FRAME_COUNT);
    video.set(CV_CAP_PROP_POS_FRAMES, 50);
    Mat frame;
    video >> frame;
    imwrite("test.jpg", frame);

    // Close VideoCapture and close all frames
    video.release();
    destroyAllWindows();
}