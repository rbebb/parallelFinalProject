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

using namespace std;
using namespace cv;

//TODO: Document
int main(int argc, char **argv) {
    VideoCapture videoIn("../../test.mp4");
    VideoWriter videoOut("output.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(1280, 720));

    int width = 1280;
    int height = 720;
    int frameCount = videoIn.get(CV_CAP_PROP_FRAME_COUNT);

    for (int i = 0; i < frameCount; i++) {
        videoIn.set(CV_CAP_PROP_POS_FRAMES, i);
        Mat frame;
        videoIn >> frame;
        videoOut.write(frame);
    }

    videoIn.release();
    videoOut.release();
    destroyAllWindows();
}
