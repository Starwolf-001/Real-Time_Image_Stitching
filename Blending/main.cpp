#include "opencv2/opencv.hpp"

using namespace cv;

int value = 2;

// Taken from http://www.morethantechnical.com/2011/11/13/just-a-simple-laplacian-pyramid-blender-using-opencv-wcode/

class LaplacianBlending {
private:

    int levels;
    Mat currentImg;
    Mat _down;
    Mat down;
    Mat up;

    Mat_<Vec3f> left;
    Mat_<Vec3f> right;
    Mat_<float> blendMask;

    vector<Mat_<Vec3f>> leftLapPyr,rightLapPyr,resultLapPyr;
    Mat leftSmallestLevel, rightSmallestLevel, resultSmallestLevel;
    //masks are 3-channels for easier multiplication with RGB
    vector<Mat_<Vec3f>> maskGaussianPyramid;

    void buildPyramids() {
        buildLaplacianPyramid(left, leftLapPyr, leftSmallestLevel);
        buildLaplacianPyramid(right, rightLapPyr, rightSmallestLevel);
        buildGaussianPyramid();
    }

    void buildGaussianPyramid() {
        assert(leftLapPyr.size() > 0);

        maskGaussianPyramid.clear();
        cvtColor(blendMask, currentImg, CV_GRAY2BGR);
        // For the highest level
        maskGaussianPyramid.push_back(currentImg);

        currentImg = blendMask;
        for (int l = 1; l < levels + 1; l++) {
            if (leftLapPyr.size() > l) {
                pyrDown(currentImg, _down, leftLapPyr[l].size());
            } else {
                // For the smallest level
                pyrDown(currentImg, _down, leftSmallestLevel.size());
            }

            cvtColor(_down, down, CV_GRAY2BGR);
            maskGaussianPyramid.push_back(down);
            currentImg = _down;
        }
    }

    void buildLaplacianPyramid(const Mat& img, vector<Mat_<Vec3f> >& lapPyr, Mat& smallestLevel) {
        lapPyr.clear();
        Mat currentImg = img;
        for (int l = 0; l < levels; l++) {
            pyrDown(currentImg, down);
            pyrUp(down, up, currentImg.size());
            Mat lap = currentImg - up;
            lapPyr.push_back(lap);
            currentImg = down;
        }
        currentImg.copyTo(smallestLevel);
    }

    Mat_<Vec3f> reconstructImgFromLapPyramid() {
        Mat currentImg = resultSmallestLevel;
        for (int l = levels - 1; l >= 0; l--) {
            pyrUp(currentImg, up, resultLapPyr[l].size());
            currentImg = up + resultLapPyr[l];
        }
        return currentImg;
    }

    void blendLapPyrs() {
        resultSmallestLevel = leftSmallestLevel.mul(maskGaussianPyramid.back()) +
            rightSmallestLevel.mul(Scalar(1.0, 1.0, 1.0) - maskGaussianPyramid.back());

        for (int l = 0; l < levels; l++) {
            Mat A = leftLapPyr[l].mul(maskGaussianPyramid[l]);
            Mat antiMask = Scalar(1.0, 1.0, 1.0) - maskGaussianPyramid[l];
            Mat B = rightLapPyr[l].mul(antiMask);
            Mat_<Vec3f> blendedLevel = A + B;

            resultLapPyr.push_back(blendedLevel);
        }
    }

public:
    LaplacianBlending(const Mat_<Vec3f>& _left, const Mat_<Vec3f>& _right, const Mat_<float>& _blendMask, int _levels):
    left(_left),right(_right),blendMask(_blendMask),levels(_levels)
    {
        assert(_left.size() == _right.size());
        assert(_left.size() == _blendMask.size());
        buildPyramids();
        blendLapPyrs();
    };

    Mat_<Vec3f> blend() {
        return reconstructImgFromLapPyramid();
    }
};

Mat_<Vec3f> LaplacianBlend(const Mat_<Vec3f>& l, const Mat_<Vec3f>& r, const Mat_<float>& m) {
    LaplacianBlending lb(l, r, m, 4);
    return lb.blend();
}

int main(int argc, char** argv) {
    // Input two images
    Mat image_1 = imread("birdseye_blend_1.jpg");
    Mat image_2 = imread("birdseye_blend_2.jpg");

    // Convert to 32 float
    Mat_<Vec3f> leftImage;
    image_1.convertTo (leftImage, CV_32F, 1.0/255.0);

    Mat_<Vec3f> rightImage;
    image_2.convertTo(rightImage, CV_32F, 1.0/255.0);

    Mat_<float> matrix (leftImage.rows, leftImage.cols, 0.0);

    // Take value of each of the input images
    // Default 2 means half of each image_1 and image_2

    matrix (Range::all(), Range (0, matrix.cols / value)) = 1.0;

    Mat_<Vec3f> blend = LaplacianBlend (leftImage, rightImage, matrix);

    // Write image
    imshow("Blended Image", blend);
    waitKey();
}

