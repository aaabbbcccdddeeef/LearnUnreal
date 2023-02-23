#include <thread>

#include "tgaimage.h"
#include <filesystem>
#include <iostream>

#include "model.h"
const TGAColor white = {255, 255, 255, 255};
const TGAColor red = {255, 0, 0, 255};
const TGAColor green = TGAColor(0,   255, 0,   255);
const TGAColor blue = TGAColor(0,   0, 255,   255);

Model* model = nullptr;
const int width = 800;
const int height = 800;

namespace 
{
    void DrawLine_Drepcated(int startX, int startY, int endX, int endY,TGAColor drawColor,  TGAImage& outImage)
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

    // 循环中的插值部分提到循环外以提升性能
    void DrawLine4(int startX, int startY, int endX, int endY,TGAColor drawColor,  TGAImage& outImage)
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

        int dx = endX - startX;
        int dy = endY - startY;

        const float derror = std::abs(float(dy)/dx);
        float error = 0;

        int y = startY;
        for (int x = startX; x < endX; ++x)
        {
            // stepp为true时，用来scan的x,y是旋转过的，设置到outImage时需要
            if (bIsSteep)
            {
                outImage.set(y, x, drawColor);
            }
            else
            {
                outImage.set(x, y, drawColor);
            }

            error += derror;
            if (error > 0.5f)
            {
                y += (endY > startY ? 1 : -1);
                error -= 1;
            }
        }
    }

    // float 计算换成int 以提高性能
    void DrawLine5(int startX, int startY, int endX, int endY,TGAColor drawColor,  TGAImage& outImage)
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

        int dx = endX - startX;
        int dy = endY - startY;

        const int derror2 = std::abs(dy) * 2;
        int error = 0;

        int y = startY;
        for (int x = startX; x < endX; ++x)
        {
            // stepp为true时，用来scan的x,y是旋转过的，设置到outImage时需要
            if (bIsSteep)
            {
                outImage.set(y, x, drawColor);
            }
            else
            {
                outImage.set(x, y, drawColor);
            }

            error += derror2;
            if (error > dx)
            {
                y += (endY > startY ? 1 : -1);
                error -= dx * 2;
            }
        }
    }

    void DrawLine(int startX, int startY, int endX, int endY,TGAColor drawColor,  TGAImage& outImage)
    {
        DrawLine5(startX, startY, endX, endY, drawColor, outImage);
    }

    void DrawLine(Vec2i startPoint, Vec2i endPoint,TGAColor drawColor,  TGAImage& outImage)
    {
        DrawLine5(startPoint.x, startPoint.y, endPoint.x, endPoint.y, drawColor, outImage);
    }

    
    void DrawTriangle_Line(Vec2i t0, Vec2i t1, Vec2i t2, TGAColor color, TGAImage &image)
    {
        DrawLine(t0, t1, color, image);
        DrawLine(t1, t2, color, image);
        DrawLine(t2, t0, color, image);
    }

    // 这个是old school的从上到下扫描，绘制点的方式绘制的，不适合并行化
    void DrawTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAColor color, TGAImage &image)
    {
        // 先对3个顶点按y排序
        if (t0.y > t1.y)
        {
            std::swap(t0, t1);
        }
        if (t0.y > t2.y)
        {
            std::swap(t0, t2);
        }
        if (t1.y > t2.y)
        {
            std::swap(t1, t2);
        }

        int totalHeight = t2.y - t0.y;
        for (int i = 0; i < totalHeight; ++i)
        {
            // 这里的secondHalf意思是扫描是按三角形中间的y点上半和下半把三角形做区分
            bool bIsSecondHalf = (i > t1.y - t0.y) || (t1.y == t0.y);
            int segmentHeight = bIsSecondHalf ? t2.y - t1.y : t1.y - t0.y;
            float alpha = (float)i/totalHeight;
            float beta = (float)(i - (bIsSecondHalf ? t1.y - t0.y : 0))/ segmentHeight;
            Vec2i A = t0 + (t2 - t0) * alpha;
            Vec2i B = bIsSecondHalf ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
            if (A.x > B.x)
            {
                std::swap(A, B);
            }

            for (int j = A.x; j <= B.x; ++j)
            {
                image.set(j, t0.y + i, color);
            }
        }

        
    }

    void DrawTriangle(Vec2i *pts, TGAColor color, TGAImage &image)
    {
        DrawTriangle(pts[0], pts[1], pts[2], color, image);
    }

    /**
     * \brief 
     * \param pts 这里的pts表示三角形的三个顶点，
     * \param p 要计算的点p 
     * \return 点p按三个顶点计算得到的重心坐标
     * 这里的实现参考https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
     */
    Vec3f ComputeBaryCentric(Vec2i*  pts, Vec2i P)
    {
        // Vec3f u = Vec3f(pts[2][0]-pts[0][0], pts[1][0]-pts[0][0], pts[0][0]-P[0])^Vec3f(pts[2][1]-pts[0][1], pts[1][1]-pts[0][1], pts[0][1]-P[1]);
        // if (std::abs(u.z) < 1)
        // {
        //     return Vec3f(-1, 1, 1);
        // }
        //
        // return Vec3f(1.f - (u.x + u.y)/u.z, u.y/u.z, u.x/u.z);
        return Vec3f(-1, 1, 1);
        
    }

    Vec3f ComputeBaryCentric(Vec3f  A,Vec3f  B, Vec3f  C, Vec3f P)
    {
        Vec3f s[2];
        for (int i=2; i--; ) {
            s[i][0] = C[i]-A[i];
            s[i][1] = B[i]-A[i];
            s[i][2] = A[i]-P[i];
        }
        Vec3f u = cross(s[0], s[1]);
        if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
            return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
        return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
    }

    Vec3f ComputeBaryCentric(Vec3f* pts, Vec3f P)
    {
        return  ComputeBaryCentric(pts[0], pts[1], pts[2], P);
    }

    
    void DrawTriangle2(Vec2i *pts, TGAColor color, TGAImage &image)
    {
#pragma region ComputeBoundingBox
#pragma endregion
        Vec2i bboxMin(image.get_width() - 1, image.get_height() - 1);
        Vec2i bboxMax(0, 0);
        Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
        for (int i = 0; i < 3; ++i)
        {
            bboxMin.x = std::max(0, std::min(bboxMin.x, pts[i].x));
            bboxMin.y = std::max(0, std::min(bboxMin.y, pts[i].y));

            bboxMax.x = std::min(clamp.x, std::max(bboxMax.x, pts[i].x));
            bboxMax.y = std::min(clamp.y, std::max(bboxMax.y, pts[i].x));
        }

        Vec2i P;
        for (int i = bboxMin.x; i <= bboxMax.x; ++i)
        {
            for (int j = bboxMin.y; j <= bboxMax.y; ++j)
            {
                P = Vec2i(i, j);
                Vec3f bc_Screen = ComputeBaryCentric(pts, P);
                if (bc_Screen.x < 0 || bc_Screen.y < 0 || bc_Screen.z < 0)
                {
                    continue;
                }
                image.set(P.x, P.y, color);
            }
        }
    }

    /**
     * \brief 加入zbuffer之后的 draw triangle
     * \param pts 
     * \param zbuffer 
     * \param color 
     * \param image 
     */
    void DrawTriangle3(Vec3f* pts, float* zbuffer, TGAColor color, TGAImage& image)
    {
        Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vec2f bboxMax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
        Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
                bboxMax[j] = std::min(clamp[j], std::max(bboxMax[j], pts[i][j]));       
            }
        }

        Vec3f curPoint;
        for (int i = bboxmin.x; i < bboxMax.x; ++i)
        {
            for (int j = bboxmin.y; j < bboxMax.y; ++j)
            {
                curPoint = Vec3f(i, j, 0); 
                Vec3f bcScreen = ComputeBaryCentric(pts, curPoint);
                if (bcScreen.x < 0 || bcScreen.y < 0 || bcScreen.y < 0)
                {
                    continue;
                }
                for (int i = 0; i < 3; ++i)
                {
                    curPoint.z += pts[i].z * bcScreen[i];
                }
                
                const int curPixelIndex = int(curPoint.x + curPoint.y * width);
                if (zbuffer[curPixelIndex] < curPoint.z)
                {
                    zbuffer[curPixelIndex] = curPoint.z;
                    image.set(curPoint.x, curPoint.y, color);
                }
                // image.set(curPoint.x, curPoint.y, color);
            }
        }
    }

    Vec3f WorldToScreen(const Vec3f& v )
    {
        return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
    }
}



namespace Lesson1
{
    void TestDrawSimpleModel(int argc, char** argv)
    {
        std::filesystem::path curWorkingDirectory = std::filesystem::current_path();
        std::cout << curWorkingDirectory.string() << std::endl;

        // TGAImage image2(100, 100, TGAImage::RGB);
        // DrawLine3(80, 40, 13, 20, red, image2);
        // image2.flip_horizontally();
        // image2.write_tga_file("output3.tga");
        //
        //
        // TGAImage image(100, 100, TGAImage::RGB);
        // // image.set(52, 41, red);
        // // DrawLine(13, 20, 80, 40,  white, image);
        // // DrawLine4(13, 20, 80, 40,  white, image); 
        // // DrawLine4(20, 13, 40, 80, red, image); 
        // DrawLine5(80, 40, 13, 20, red, image);
        // image.flip_horizontally();
        // image.write_tga_file("output5.tga");
    
        if (2 == argc)
        {
            model = new Model(argv[1]);
        }
        else
        {
            // model = new Model("../../QxTinyRender/obj/floor.obj");
        
            model = new Model("african_head.obj");

        }

        TGAImage image(width, height, TGAImage::RGB);
        for (int i = 0; i < model->nfaces(); ++i)
        {
            std::vector<int> face = model->face(i);
            for (int j = 0; j < 3; ++j)
            {
                Vec3f v0 = model->vert(face[j]);
                Vec3f v1 = model->vert(face[(j + 1)%3]);

                int startX, endX, startY, endY;
                startX = (v0.x + 1.) * width /2;
                startY = (v0.y + 1.) * height /2;
                endX = (v1.x + 1.) * width /2;
                endY = (v1.y + 1.) * height /2;

                DrawLine5(startX, startY, endX, endY, white, image);
            }
        }
        image.flip_vertically();
        image.write_tga_file("testModel.tga");
        delete model;
    }
}


TGAColor operator*(const TGAColor inColor , float inMulti)
{
    return TGAColor(inColor.r * inMulti, inColor.g * inMulti, inColor.b * inMulti, inColor.a * inMulti);
}

namespace Lesson2
{
    void DrawTestModel()
    {
        model = new Model("african_head.obj");
        TGAImage image(width, height, TGAImage::RGB);

        Vec3f lightDir(0, 0, -1);
        for (int i=0; i<model->nfaces(); i++) { 
            std::vector<int> face = model->face(i);
            Vec3f worldPositions[3];
            Vec2i screen_coords[3]; 
            for (int j=0; j<3; j++) { 
                Vec3f world_coords = model->vert(face[j]); 
                screen_coords[j] = Vec2i((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2.);
                worldPositions[j] = world_coords;
            }

            Vec3f normal = cross((worldPositions[2] - worldPositions[0]), (worldPositions[1] - worldPositions[0]));
            normal.normalize();
            float intensity = normal * lightDir;
            TGAColor litColor = white;
            litColor = litColor * intensity;
            if (intensity <= 0)
            {
                continue;
            }
            DrawTriangle(screen_coords, litColor, image); 
        }
        
        image.flip_vertically();
        image.write_tga_file("testModel2.tga");
        delete model;
    }
}

/**
 * \brief 这一节实现了添加zbuffer，通过zbuffer实现遮挡
 */
namespace Lesson3
{
    void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int ybuffer[]) {
        if (p0.x>p1.x) {
            std::swap(p0, p1);
        }
        for (int x=p0.x; x<=p1.x; x++) {
            float t = (x-p0.x)/(float)(p1.x-p0.x);
            int y = p0.y*(1.-t) + p1.y*t + .5;
            if (ybuffer[x]<y) {
                ybuffer[x] = y;
                image.set(x, 0, color);
            }
        }
    }

    void Test()
    {
        { // just dumping the 2d scene (yay we have enough dimensions!)
            TGAImage scene(width, height, TGAImage::RGB);

            // scene "2d mesh"
            DrawLine(Vec2i(20, 34),   Vec2i(744, 400), red, scene);
            DrawLine(Vec2i(120, 434), Vec2i(444, 400), green, scene);
            DrawLine(Vec2i(330, 463), Vec2i(594, 200), blue, scene);

            // screen line
            DrawLine(Vec2i(10, 10), Vec2i(790, 10),white, scene);

            scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
            scene.write_tga_file("scene.tga");
        }

        {
            TGAImage render(width, 16, TGAImage::RGB);
            int ybuffer[width];
            for (int i=0; i<width; i++) {
                ybuffer[i] = std::numeric_limits<int>::min();
            }
            rasterize(Vec2i(20, 34),   Vec2i(744, 400), render, red,   ybuffer);
            rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
            rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue,  ybuffer);

            // 1-pixel wide image is bad for eyes, lets widen it
            for (int i=0; i<width; i++) {
                for (int j=1; j<16; j++) {
                    render.set(i, j, render.get(i, 0));
                }
            }
            render.flip_vertically(); // i want to have the origin at the left bottom corner of the image
            render.write_tga_file("render.tga");
        }
    }

    void DrawTestModel()
    {
        model = new Model("african_head.obj");
        TGAImage image(width, height, TGAImage::RGB);
        float* zbuffer = new float[width * height];
        for (int i = 0; i < width * height; ++i)
        {
            zbuffer[i] = -std::numeric_limits<float>::max();
        }

        Vec3f lightDir(0, 0, -1);
        for (int i=0; i<model->nfaces(); i++) { 
            std::vector<int> face = model->face(i);
            Vec3f screen_coords[3];
            Vec3f worldPositions[3];
            for (int j=0; j<3; j++) { 
                Vec3f world_coords = model->vert(face[j]); 
                screen_coords[j] = Vec3f((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2., world_coords.z);
                worldPositions[j] = world_coords;
            }

            Vec3f normal = cross(worldPositions[2] - worldPositions[0], worldPositions[1] - worldPositions[0]);
            normal.normalize();

            float intensity = normal * lightDir ;
            if (intensity <= 0)
            {
                continue;
            }
            TGAColor litColor = white;  //TGAColor(rand()%255, rand()%255, rand()%255, 255);
            litColor = litColor * intensity;
            DrawTriangle3(screen_coords, zbuffer, litColor , image); 
        }
        image.flip_vertically();
        image.write_tga_file("testModel3.tga");
        delete model;
    }
}

int main(int argc, char** argv)
{

    // TGAImage frame(200, 200, TGAImage::RGB); 
    // Vec2i pts[3] = {Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)}; 
    // // DrawTriangle2(pts,  white, frame);
    // DrawTriangle2(pts, white, frame);
    // frame.flip_vertically(); // to place the origin in the bottom left corner of the image 
    // frame.write_tga_file("framebuffer.tga");
    // return 0;
    // Lesson2::TestDrawModel();

    // Lesson3::Test();
    Lesson3::DrawTestModel();
    Lesson2::DrawTestModel();
}