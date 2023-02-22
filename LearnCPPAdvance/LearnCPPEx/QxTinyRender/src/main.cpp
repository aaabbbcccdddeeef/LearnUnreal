#include <thread>

#include "tgaimage.h"

namespace 
{
    void DrawLine(int startX, int startY, int endX, int endY,TGAColor drawColor,  TGAImage& outImage)
    {
        const int imageWidth = 100;
        const float step = 1.f/(float)imageWidth;
        for (float i = 0; i < 1.f; i += step)
        {
            int x = startX + (endX - startX) * i;
            int y = startY + (endY - startY) * i;
            outImage.set(x, y, drawColor);
        }
    }

    void DrawLine2(int startX, int startY, int endX, int endY,TGAColor drawColor,  TGAImage& outImage)
    {
        for (int x = startX; x < endX; ++x)
        {
            float t = (x - startX)/(float)(endX - startX);
            int y = (1. - t)* startY + t * endY;
            outImage.set(x, y, drawColor);
        }
    }

    void DrawLine3(int startX, int startY, int endX, int endY,TGAColor drawColor,  TGAImage& outImage)
    {
        bool bIsSteep = false;

        // y更抖时交换x/y
        if (abs(endX - startX) < abs(endY - startY))
        {
            std::swap(startX, startY);
            std::swap(endX, endY);
            bIsSteep = true;
        }

        // start > endX时交换起点终点，使后面的扫描一定从左到右
        if (startX > endX)
        {
            std::swap(startX, endX);
            std::swap(startY, endY);
        }

        for (int x = startX; x < endX; ++x)
        {
            float t = (x - startX)/(float)(endX - startX);
            int y = (1. - t)* startY + t * endY;
            // stepp为true时，用来scan的x,y是旋转过的，设置到outImage时需要
            if (bIsSteep)
            {
                outImage.set(y, x, drawColor);
            }
            else
            {
                outImage.set(x, y, drawColor);
            }
        }
    }
}


const TGAColor white = {255, 255, 255, 255};
const TGAColor red = {255, 0, 0, 255};
int main(int argc, char** argv)
{
    TGAImage image(100, 100, TGAImage::RGB);
    // image.set(52, 41, red);
    // DrawLine(13, 20, 80, 40,  white, image);
    DrawLine3(13, 20, 80, 40,  white, image); 
    DrawLine3(20, 13, 40, 80, red, image); 
    DrawLine3(80, 40, 13, 20, red, image);
    image.flip_horizontally();
    image.write_tga_file("output.tga");

    return 0;
}