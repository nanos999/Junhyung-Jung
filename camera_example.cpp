#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/photo.hpp>

using namespace std;
using namespace cv;

string gstreamer_pipeline(int capture_width, int capture_height, int framerate, int display_width, int display_height) {
    return
        "libcamerasrc ! video/x-raw, "
        "width=(int)" + to_string(capture_width) + ", "
        "height=(int)" + to_string(capture_height) + ", "
        "framerate=(fraction)" + to_string(framerate) + "/1 ! "
        "videoconvert ! videoscale ! "
        "video/x-raw, "
        "width=(int)" + to_string(display_width) + ", "
        "height=(int)" + to_string(display_height) + " ! appsink";
}

int main(int argc, char** argv)
{
    int capture_width = 640;
    int capture_height = 480;
    int framerate = 30;
    int display_width = 640;
    int display_height = 480;

    string pipeline = gstreamer_pipeline(capture_width, capture_height, framerate, display_width, display_height);

    VideoCapture cap(pipeline, CAP_GSTREAMER);

    if (!cap.isOpened()) {
        cerr << "Can't open camera." << endl;
        return -1;
    }

    int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);
    VideoWriter video("outcpp.avi", VideoWriter::fourcc('M','J','P','G'), framerate, Size(frame_width,frame_height));

    cout << "Open Camera\n";
    Mat img;
    int count = 0; int max;

    if (argc > 1) {
        max = int(argv[1]);
    } else {
        max = 50;
    }

    while (count <= max) {
        cap.read(img);
        if (img.empty()) break;
        video.write(img);
        count++;
    }

    cap.release();
    video.release();
    return 0;
}
