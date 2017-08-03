#pragma once

#define _USE_MATH_DEFINES

#include <cmath>

#if !defined(M_PI)
#    define M_PI 3.1415926535897f
#endif // !defined(M_PI)

#if !defined(M_HALF_PI)
#    define M_HALF_PI M_PI / 2.f
#endif // !defined M_HALF_PI

#include <opencv2/opencv.hpp>

namespace ptc
{
    enum class Name
    {
        Front,
        Right,
        Back,
        Left,
        Top,
        Bottom,

        NumFaces
    };

    struct Face
    {
        Name name;
        float polarCoords[2];
    };

    /// enum to string
    std::string faceNameToString(Name faceName)
    {
        switch (faceName)
        {
            case Name::Front:  return "Front";
            case Name::Right:  return "Right";
            case Name::Back:   return "Back";
            case Name::Left:   return "Left";
            case Name::Top:    return "Top";
            case Name::Bottom: return "Bottom";
            default:           return "UNKNOWN";
        }
    }

    /// define our six cube faces
    static const Face facesTable[6] =
    {
        { Name::Front,  {        0.f,        0.f } },
        { Name::Right,  { +M_HALF_PI,        0.f } },
        { Name::Back,   {      +M_PI,        0.f } },
        { Name::Left,   { -M_HALF_PI,        0.f } },
        { Name::Top,    {        0.f, -M_HALF_PI } },
        { Name::Bottom, {        0.f, +M_HALF_PI } },
    };

    // map a part of the equirectangular panorama (in) to a cube face
    // (face). The desired width and height are given by width and height.
    static bool createCubeMapFace(const cv::Mat &in, cv::Mat &face,
                                  Name faceName, int width, int height)
    {
        // we have to enforce input image dimensions to be equirectangular
        if (in.size().height != in.size().width / 2)
        {
            return false;
        }

        const int faceId = (int)faceName;

        const float inWidth  = in.cols;
        const float inHeight = in.rows;

        cv::Mat mapx(height, width, CV_32F);
        cv::Mat mapy(height, width, CV_32F);

        // calculate adjacent (ak) and opposite (an) of the
        // triangle that is spanned from the sphere center 
        // to our cube face
        const float an = sin(M_PI / 4);
        const float ak = cos(M_PI / 4);

        const float ftu = facesTable[faceId].polarCoords[0];
        const float ftv = facesTable[faceId].polarCoords[1];

        // for each point in the target image, 
        // calculate the corresponding source coordinates
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // map face pixel coordinates to [-1, 1] on plane
                float nx = (float)y / (float)height - 0.5f;
                float ny = (float)x / (float)width  - 0.5f;

                nx *= 2;
                ny *= 2;

                // map [-1, 1] plane coords to [-an, an]
                // thats the coordinates in respect to a unit sphere 
                // that contains our box
                nx *= an; 
                ny *= an; 

                float u, v;

                // project from plane to sphere surface
                if (ftv == 0)
                {
                    // center faces
                    u = atan2(nx, ak);
                    v = atan2(ny * cos(u), ak);
                    u += ftu; 
                }
                else if (ftv > 0)
                { 
                    // bottom face 
                    float d = sqrt(nx * nx + ny * ny);
                    v = M_PI / 2 - atan2(d, ak);
                    u = atan2(ny, nx);
                }
                else
                {
                    // top face
                    float d = sqrt(nx * nx + ny * ny);
                    v = -M_PI / 2 + atan2(d, ak);
                    u = atan2(-ny, nx);
                }

                // map from angular coordinates to [-1, 1], respectively
                u = u / (M_PI); 
                v = v / (M_PI / 2);

                // warp around, if our coordinates are out of bounds
                while (v < -1)
                {
                    v += 2;
                    u += 1;
                } 

                while (v > 1)
                {
                    v -= 2;
                    u += 1;
                } 

                while(u < -1)
                {
                    u += 2;
                }

                while(u > 1)
                {
                    u -= 2;
                }

                // map from [-1, 1] to in texture space
                u = u / 2.0f + 0.5f;
                v = v / 2.0f + 0.5f;

                u = u * (inWidth  - 1);
                v = v * (inHeight - 1);

                // save the result for this pixel in map
                mapx.at<float>(x, y) = u;
                mapy.at<float>(x, y) = v; 
            }
        }

        // recreate output image if it has wrong size or type
        if (face.cols != width || face.rows != height || face.type() != in.type())
        {
            face = cv::Mat(width, height, in.type());
        }

        // run actual resampling using OpenCV's remap
        cv::remap(in, face, mapx, mapy, CV_INTER_CUBIC, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

        return true;
    }

}; // namespace ptc

