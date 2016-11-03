#include <stdio.h>
#include <iostream>

// From OpenCV-2.4.13
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

Mat image_1;

int main(int argc, char** argv)
{
    // Loads the 1st Input Image and Prepares a Grayscale of the Image
    Mat image_1_input = imread (argv[1]);
    // Rotate column to correct angle
    double angle = -90;

    // get rotation matrix for rotating the image around its center
    Point2f centerOne(image_1_input.cols / 2.0, image_1_input.rows / 2.0);
    Mat rotOne = getRotationMatrix2D(centerOne, angle, 1.0);
    // determine bounding rectangle
    Rect boxOne = RotatedRect(centerOne, image_1_input.size(), angle).boundingRect();
    // adjust transformation matrix
    rotOne.at<double>(0, 2) += boxOne.width / 2.0 - centerOne.x;
    rotOne.at<double>(1, 2) += boxOne.height / 2.0 - centerOne.y;

    warpAffine(image_1_input, image_1, rotOne, boxOne.size());

    // Write image
    imwrite("Update.jpg", image_1);
}
