#include <stdio.h>
#include <iostream>
#include <time.h>

// From OpenCV-2.4.13
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main() {
    //-----------------------------------------------------------------------------------------------------------------
    // Windows Timing - Start
    clock_t time1, time2;
    time1 = clock();
    //-----------------------------------------------------------------------------------------------------------------

    const int threshValue = 20;
     // Set to 5%
    const float borderThresh = 0.05f;

    Mat input_img = imread("Update.jpg");
    Mat input_img_gray;

    // Convert to Images to Grayscale
    cvtColor (input_img, input_img_gray, CV_RGB2GRAY);

    // Threshold
    Mat thresholded;
    threshold(input_img_gray, thresholded, threshValue, 255, THRESH_BINARY);
    morphologyEx(thresholded, thresholded, MORPH_CLOSE,
    // Rectangle Structure and Borders
    getStructuringElement(MORPH_RECT, Size(3, 3)),
    Point(-1, -1), 2, BORDER_CONSTANT, Scalar(0));

    Point thresholdLength;
    Point borderRectangle;

    for (int row = 0; row < thresholded.rows; row++) {
        if (countNonZero(thresholded.row(row)) > borderThresh * thresholded.cols) {
            thresholdLength.y = row;
            break;
        }
    }

    for (int col = 0; col < thresholded.cols; col++) {
        if (countNonZero(thresholded.col(col)) > borderThresh * thresholded.rows) {
            thresholdLength.x = col;
            break;
        }
    }

    for (int row = thresholded.rows - 1; row >= 0; row--) {
        if (countNonZero(thresholded.row(row)) > borderThresh * thresholded.cols) {
            borderRectangle.y = row;
            break;
        }
    }

    for (int col = thresholded.cols - 1; col >= 0; col--) {
        if (countNonZero(thresholded.col(col)) > borderThresh * thresholded.rows) {
            borderRectangle.x = col;
            break;
        }
    }

    // Rectangle Region of Interest (roi)
    Rect roi(thresholdLength, borderRectangle);
    Mat cropped = input_img(roi);

    imwrite("Update_o.jpg", cropped);

    //-----------------------------------------------------------------------------------------------------------------
    // Windows Timing - End
    time2 = clock();
    float diff = (float)time2 - (float)time1;
    cout << diff << endl;
    //-----------------------------------------------------------------------------------------------------------------

    return 0;
}
