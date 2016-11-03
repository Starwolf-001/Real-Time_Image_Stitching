#include <stdio.h>
#include <iostream>

// From OpenCV-2.4.13
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

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
int main (int argc, char** argv) {

    // If there isn't 3 commands then readme() is called and returns 0;
    if (argc != 3) {
        instructions();
        return 0;
    }

    // Load the 2 Input Images and Prepares 2 Grayscale Images
    Mat image_1 = imread (argv[1]);
    Mat image_2 = imread (argv[2]);
    Mat gray_image_1;
    Mat gray_image_2;

    // Convert to Images to Grayscale
    cvtColor (image_1, gray_image_1, CV_RGB2GRAY);
    cvtColor (image_2, gray_image_2, CV_RGB2GRAY);

    // Display Input Images
    //imshow ("First Input Image", image_1);
    //imshow ("Second Input Image", image_2);

    // Check if image files can be read
    if (!gray_image_1.data) {
        std::cout << "Error Reading Image 1" << std::endl;
        return 0;
    }
    if (!gray_image_2.data) {
        std::cout << "Error Reading Image 2" << std::endl;
        return 0;
    }

    // Detect the keypoints using SURF Detector
    // Based from Anna Huaman's 'Features2D + Homography to find a known object' Tutorial
    int minHessian = 50;

    SurfFeatureDetector detector (minHessian);

    std::vector <KeyPoint> keypoints_object, keypoints_scene;

    detector.detect (gray_image_2, keypoints_object);
    detector.detect (gray_image_1, keypoints_scene);

    // Calculate Feature Vectors (descriptors)
    // Based from  Anna Huaman's 'Features2D + Homography to find a known object' Tutorial
    SurfDescriptorExtractor extractor;

    Mat descriptors_object, descriptors_scene;

    extractor.compute (gray_image_2, keypoints_object, descriptors_object);
    extractor.compute (gray_image_1, keypoints_scene, descriptors_scene);

    // Matching descriptor vectors using FLANN matcher
    // Based from  Anna Huaman's 'Features2D + Homography to find a known object' Tutorial
    FlannBasedMatcher matcher;
    std::vector <DMatch> matches;
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
    }

    // Use matches tha have a distance that is less than 3 * min_dist
    std::vector <DMatch> good_matches;

    for (int i = 0; i < descriptors_object.rows; i++){
        if (matches[i].distance < 3 * min_dist) {
            good_matches.push_back (matches[i]);
        }
    }

    std::vector <Point2f> obj;
    std::vector <Point2f> scene;

    for (int i = 0; i < good_matches.size(); i++) {
        // Get the keypoints from the good matches
        obj.push_back (keypoints_object[good_matches[i].queryIdx].pt);
        scene.push_back (keypoints_scene[good_matches[i].trainIdx].pt);
    }

    // Find the Homography Matrix
    Mat H = findHomography (obj, scene, CV_RANSAC);
    // Use the Homography Matrix to warp the images
    cv::Mat result;
    warpPerspective (image_2, result, H, cv::Size (image_2.cols + image_1.cols, image_2.rows));
    cv::Mat half (result, cv::Rect (0, 0, image_1.cols, image_1.rows));
    image_1.copyTo (half);

    // Show Stitched Images
    imshow ("Result", result);

    // Write image
    imwrite("Update.jpg", result);

    waitKey (0);
    return 0;
 }

/*
 *  ======== readme ========
 *  If the user does not run the executable and input 2 images this 'read me' will display
 */
void instructions() {
    std::cout << "Welcome to Michael Cartwright's Prototype Image Stitching Software" << std::endl;
    std::cout << "---------------------------------------------------------------------------------" << std::endl;
    std::cout << "References:" << std::endl;
    std::cout << "Using OpenCV 'Open Computer Vision' package (currently using version 2.4.13)" << std::endl;
    std::cout << "Based on 'Features2D + Homography to find a known object' tutorial by Anna Huaman" << std::endl;
    std::cout << "Google Maps for sample images" << std::endl;
    std::cout << "---------------------------------------------------------------------------------" << std::endl;
    std::cout << "To use this software:" << std::endl;
    std::cout << "Place the 'Image_Stitching_Prototype.exe' to your C:/ Drive" << std::endl;
    std::cout << "You need two images that you want stitched together" << std::endl;
    std::cout << "There are some sample images using Google Maps basic and satelite view of QUT" << std::endl;
    std::cout << "---------------------------------------------------------------------------------" << std::endl;
    std::cout << "To run the application simply open Windows' Command Prompt" << std::endl;
    std::cout << "Type 'cd C:/' to access the C:/ Drive directory" << std::endl;
    std::cout << "Type 'Image_Stitching_Prototype.exe' and on the same line the names of two images" << std::endl;
    std::cout << "For Example:" << std::endl;
    std::cout << "Image_Stitching_Prototype.exe basic_map_image_1.jpg basic_map_image_2.jpg" << std::endl;
}
