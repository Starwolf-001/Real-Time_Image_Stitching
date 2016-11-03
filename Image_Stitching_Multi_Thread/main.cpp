#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <time.h>

// From OpenCV-2.4.13
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/nonfree.hpp"

using namespace cv;
using namespace std;

Mat image_1;
Mat image_2;
Mat gray_image_1;
Mat gray_image_2;
Mat descriptors_object;
Mat descriptors_scene;

int minHessian = 5;

// Keypoints
vector <KeyPoint> keypoints_object;
vector <KeyPoint> keypoints_scene;

// Detect the keypoints using SURF Detector
// Based from Anna Huaman's 'Features2D + Homography to find a known object' Tutorial
SurfFeatureDetector detector (minHessian);

// Calculate Feature Vectors (descriptors)
// Based from  Anna Huaman's 'Features2D + Homography to find a known object' Tutorial
SurfDescriptorExtractor extractor;

void instructions();

/*
 *  ======== main ========
 * Checks for 3 commands at the console when executing
 * Command 1: Image_Stitching.exe
 * Command 2: basic_image_1.jpg
 * Command 3: basic_image_2.jpg
 *
 * argv holds string values of the 2 Input Images
 * Command 2: basic_image_1.jpg
 * Command 3: basic_image_2.jpg
 */

int checkFirstGrayScaleImage() {
    // Check if image files can be read
    if (!gray_image_1.data) {
        cout << "Error Reading Image 1" << endl;
        return 0;
    }
}

int checkSecondGrayScaleImage() {
    // Check if image files can be read
    if (!gray_image_2.data) {
        cout << "Error Reading Image 2" << endl;
        return 0;
    }
}

// Thread 1
void *loadFirstImage(void *) {
    // Check if image files can be read
    checkFirstGrayScaleImage();
    return NULL;
}

// Thread 2
void *loadSecondImage(void *) {
    // Check if image files can be read
    checkSecondGrayScaleImage();
    return NULL;
}

// Thread 3
void *surfDetectObject(void *) {
    //std::vector <KeyPoint> keypoints_object;
    detector.detect (gray_image_2, keypoints_object);
    extractor.compute (gray_image_2, keypoints_object, descriptors_object);
}

// Thread 4
void *surfDetectScene(void *) {
    //std::vector <KeyPoint> keypoints_scene;
    detector.detect (gray_image_1, keypoints_scene);
    extractor.compute (gray_image_1, keypoints_scene, descriptors_scene);
}

// Main
int main (int argc, char** argv) {
    // If there isn't 3 commands then readme() is called and returns 0;
    if (argc != 3) {
        instructions();
        return 0;
    }
    //-----------------------------------------------------------------------------------------------------------------
    // Threads declared - Note #include <thread> does not run on the complier designed for this project. pthread.h used instead.
    pthread_t t1;
    pthread_t t2;
    pthread_t t3;
    pthread_t t4;
    //-----------------------------------------------------------------------------------------------------------------
    // Windows Timing - Start
    clock_t time1, time2;
    time1 = clock();
    //-----------------------------------------------------------------------------------------------------------------
    // Loads the 1st Input Image and Prepares a Grayscale of the Image
    Mat image_1 = imread (argv[1]);
    // Loads the 2nd Input Image and Prepares a Grayscale of the Image
    Mat image_2 = imread (argv[2]);
    //-----------------------------------------------------------------------------------------------------------------
    // Rotation angle
    //double angle = 90;

    // get rotation matrix for rotating the image around its center
    //Point2f centerOne(image_1_input.cols / 2.0, image_1_input.rows / 2.0);
    //Mat rotOne = getRotationMatrix2D(centerOne, angle, 1.0);
    // determine bounding rectangle
    //Rect boxOne = RotatedRect(centerOne, image_1_input.size(), angle).boundingRect();
    // adjust transformation matrix
    //rotOne.at<double>(0, 2) += boxOne.width / 2.0 - centerOne.x;
    //rotOne.at<double>(1, 2) += boxOne.height / 2.0 - centerOne.y;

    //warpAffine(image_1_input, image_1, rotOne, boxOne.size());

    // get rotation matrix for rotating the image around its center
    //Point2f centerTwo(image_2_input.cols / 2.0, image_2_input.rows/2.0);
    //Mat rotTwo = getRotationMatrix2D(centerTwo, angle, 1.0);
    // determine bounding rectangle
    //Rect boxTwo = RotatedRect(centerTwo, image_2_input.size(), angle).boundingRect();
    // adjust transformation matrix
    //rotTwo.at<double>(0, 2) += boxTwo.width / 2.0 - centerTwo.x;
    //rotTwo.at<double>(1, 2) += boxTwo.height / 2.0 - centerTwo.y;

    //warpAffine(image_2_input, image_2, rotTwo, boxTwo.size());
    //-----------------------------------------------------------------------------------------------------------------
    // Convert to Images to Grayscale
    cvtColor (image_1, gray_image_1, CV_RGB2GRAY);
    cvtColor (image_2, gray_image_2, CV_RGB2GRAY);
    // Thread 1
    pthread_create(&t1, NULL, loadFirstImage, NULL);
    // Thread 2
    pthread_create(&t2, NULL, loadSecondImage, NULL);
    // Wait for Threads t1 and t2 to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    //-----------------------------------------------------------------------------------------------------------------
    // Detect the keypoints using SURF Detector
    // Based from Anna Huaman's 'Features2D + Homography to find a known object' Tutorial
    // SurfFeatureDetector detector (minHessian);
    // Thread 3
    pthread_create(&t3, NULL, surfDetectObject, NULL);
    // Thread 4
    pthread_create(&t4, NULL, surfDetectScene, NULL);
    // Wait for Threads t3 and t4 to finish
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    //-----------------------------------------------------------------------------------------------------------------
    // Matching descriptor vectors using FLANN matcher
    // Based from  Anna Huaman's 'Features2D + Homography to find a known object' Tutorial
    FlannBasedMatcher matcher;
    vector <DMatch> matches;
    matcher.match (descriptors_object, descriptors_scene, matches);
    double max_dist = 0;
    double min_dist = 100;

    // Quick calculation of max and min distances between keypoints
    // Based from  Anna Huaman's 'Features2D + Homography to find a known object' Tutorial
    for (int i = 0; i < descriptors_object.rows; i++) {

        double dist = matches[i].distance;

        if (dist < min_dist) {
            min_dist = dist;
        }
        if (dist > max_dist) {
            max_dist = dist;
        }
    }

    // Use matches that have a distance that is less than 3 * min_dist
    vector <DMatch> good_matches;

    for (int i = 0; i < descriptors_object.rows; i++){
        if (matches[i].distance < 10 * min_dist) {
            good_matches.push_back (matches[i]);
        }
    }

    vector <Point2f> obj;
    vector <Point2f> scene;

    for (int i = 0; i < good_matches.size(); i++) {
        // Get the keypoints from the good matches
        obj.push_back (keypoints_object[good_matches[i].queryIdx].pt);
        scene.push_back (keypoints_scene[good_matches[i].trainIdx].pt);
    }

    // Will only progress if at least four good matches can be found.
    if (good_matches.size() < 4) {
        cout << "Requires at least 4 good_matches" << endl;
        return 0;
    }

    // Find the Homography Matrix
    Mat H = findHomography (obj, scene, CV_RANSAC);
    // Use the Homography Matrix to warp the images
    Mat result;
    warpPerspective (image_2, result, H, Size (image_2.cols + image_1.cols, image_2.rows));
    Mat half (result, Rect (0, 0, image_1.cols, image_1.rows));
    image_1.copyTo (half);

    // Set JPEG lossy compression to minimal
    vector <int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    // 100% best possible quality
    compression_params.push_back(100);

    // Write image
    imwrite("Update.jpg", result, compression_params);
    //-----------------------------------------------------------------------------------------------------------------
    // Windows Timing - End
    time2 = clock();
    float diff = (float)time2 - (float)time1;
    cout << diff << endl;
    //-----------------------------------------------------------------------------------------------------------------
    return 0;
}

/*
 *  ======== readme ========
 *  If the user does not run the executable and input 2 images this 'read me' will display
 */
void instructions() {
    cout << "Welcome to Real-Time  Image Stitching Software" << endl;
    cout << "Created by Michael Cartwright, 2016" << endl;
    cout << "---------------------------------------------------------------------------------" << endl;
    cout << "References:" << endl;
    cout << "Using OpenCV 'Open Computer Vision' package (currently using version 2.4.13)" << endl;
    cout << "Based on 'Features2D + Homography to find a known object' tutorial by Anna Huaman" << endl;
    cout << "Google Maps for sample images" << endl;
    cout << "---------------------------------------------------------------------------------" << endl;
    cout << "To use this software:" << endl;
    cout << "Place the 'Image_Stitching_Prototype.exe' to your C:/ Drive" << endl;
    cout << "You need two images that you want stitched together" << endl;
    cout << "There are some sample images using Google Maps basic and satelite view of QUT" << endl;
    cout << "---------------------------------------------------------------------------------" << endl;
    cout << "To run the application simply open Windows' Command Prompt and Run as Admin" << endl;
    cout << "Type 'cd C:/' to access the C:/ Drive directory" << endl;
    cout << "Type 'Image_Stitching_Prototype.exe' and on the same line the names of two images" << endl;
    cout << "For Example:" << endl;
    cout << "Image_Stitching_Prototype.exe basic_map_image_1.jpg basic_map_image_2.jpg" << endl;
}
