#include "PanoToCube.h"

#include <cstdio>
#include <string>

#define PTC_SHOW_IMAGES 0
#if PTC_SHOW_IMAGES
#  define SHOW_IMAGE(n, i) cv::imshow(n, i)
#else
#  define SHOW_IMAGE(n, i)
#endif // PTC_SHOW_IMAGES

#define TARGET_FACE_DIM 1024

void createFaceAndDisplay(const cv::Mat& input, cv::Mat& faceImg, ptc::Name faceName)
{
    std::string faceNameStr = ptc::faceNameToString(faceName);

    std::printf("[DEBUG] - Processing %s face...\n", faceNameStr.c_str());
    ptc::createCubeMapFace(input, faceImg, faceName, TARGET_FACE_DIM, TARGET_FACE_DIM);
    SHOW_IMAGE(faceNameStr.c_str(), faceImg);
}

int main(int argc, char* argv[])
{
    const cv::Mat input = cv::imread(argv[1]);
    SHOW_IMAGE("input", input);

    cv::Mat front, right, back, left, top, bottom;

    bool succeeded = true;
    succeeded &= createFaceAndDisplay(input, front,  ptc::Name::Front);
    succeeded &= createFaceAndDisplay(input, right,  ptc::Name::Right);
    succeeded &= createFaceAndDisplay(input, back,   ptc::Name::Back);
    succeeded &= createFaceAndDisplay(input, left,   ptc::Name::Left);
    succeeded &= createFaceAndDisplay(input, top,    ptc::Name::Top);
    succeeded &= createFaceAndDisplay(input, bottom, ptc::Name::Bottom);

    if (!succeeded)
    {
        std::printf("[ERROR] - One or more faces could not be created...\n");
        return EXIT_FAILURE;
    }

    // align the bottom
    cv::transpose(bottom, bottom);
    cv::flip(bottom, bottom, 0);

    // and the top
    cv::transpose(top, top);
    cv::flip(top, top, 1);

    const int size = TARGET_FACE_DIM;
    cv::Mat combination(cv::Size(front.size().width * 4, front.size().height * 3), CV_8UC3, cv::Scalar::all(0));
    top.copyTo(combination(cv::Rect(size, 0, size, size)));
    left.copyTo(combination(cv::Rect(0, size, size, size)));
    front.copyTo(combination(cv::Rect(size, size, size, size)));
    right.copyTo(combination(cv::Rect(size*2, size, size, size)));
    back.copyTo(combination(cv::Rect(size*3, size, size, size)));
    bottom.copyTo(combination(cv::Rect(size, size*2, size, size)));

    cv::imwrite("./output/input.png",    input);
    cv::imwrite("./output/front.png",    front);
    cv::imwrite("./output/right.png",    right);
    cv::imwrite("./output/back.png",     back);
    cv::imwrite("./output/left.png",     left);
    cv::imwrite("./output/top.png",      top);
    cv::imwrite("./output/bottom.png",   bottom);
    cv::imwrite("./output/allsides.png", combination);

#if PTC_SHOW_IMAGES
    return cv::waitKey(0);
#else
    return EXIT_SUCCESS;
#endif // PTC_SHOW_IMAGES
}
