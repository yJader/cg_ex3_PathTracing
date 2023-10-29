#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#define WORK_PATH "/home/jader/graphics/ex3"

extern bool open_msaa;

const char *getModelPath(const char *filename)
{
    char workpath[1024]; // 定义一个足够大的字符数组来存储路径

    if (getcwd(workpath, sizeof(workpath)) != NULL)
    {
        printf("当前工作目录：%s\n", workpath);
    }
    else
    {
        perror("获取当前工作目录失败");
    }

    std::string fullPath = workpath;
    fullPath += "/src/models/";
    fullPath += filename;
    char *pathCopy = new char[fullPath.length() + 1];
    strcpy(pathCopy, fullPath.c_str());

    return pathCopy;
}

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char **argv)
{
    int spp = 16;
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        // 检查是否为输出选项
        if (arg == "-spp")
        {
            // 关闭反走样
            // printf("已关闭反走样\n");
            i++;

            if (sqrt(spp) * sqrt(spp) == spp)
            {
                spp = atoi(argv[i]);
            }
            else
            {
                printf("spp必须为完全平方数");
                return 0;
            }
        }
        else if (arg == "-cmsaa")
        {
            open_msaa = false;
        }
        else if (arg == "-h")
        {
            std::cout << "用法：" << std::endl;
            std::cout << "-spp \t指定采样数, 默认为16 (完全平方数)" << std::endl;
            std::cout << "-cmsaa \t 关闭抗锯齿" << std::endl;
            std::cout << "-h  \t显示帮助信息" << std::endl;
            return 0;
        }
    }

    char pictureFilePath[100];
    if (open_msaa)
    {
        printf("====MSAA已开启====\n");
        sprintf(pictureFilePath, "./images/spp%d_msaa.ppm", spp);
    }
    else
    {
        printf("====MSAA已关闭====\n");
        sprintf(pictureFilePath, "./images/spp%d.ppm", spp);
    }

    // Change the definition here to change resolution
    Scene scene(784, 784);

    Material *red = new Material(DIFFUSE, Vector3f(0.0f));
    red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    Material *green = new Material(DIFFUSE, Vector3f(0.0f));
    green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    Material *white = new Material(DIFFUSE, Vector3f(0.0f));
    white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    Material *light = new Material(DIFFUSE, (8.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) + 15.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) + 18.4f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)));
    light->Kd = Vector3f(0.65f);

    MeshTriangle floor(getModelPath("cornellbox/floor.obj"), white);
    MeshTriangle shortbox(getModelPath("cornellbox/shortbox.obj"), white);
    MeshTriangle tallbox(getModelPath("cornellbox/tallbox.obj"), white);
    MeshTriangle left(getModelPath("cornellbox/left.obj"), red);
    MeshTriangle right(getModelPath("cornellbox/right.obj"), green);
    MeshTriangle light_(getModelPath("cornellbox/light.obj"), light);

    scene.Add(&floor);
    scene.Add(&shortbox);
    scene.Add(&tallbox);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);
    scene.buildBVH();

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene, spp, pictureFilePath);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}