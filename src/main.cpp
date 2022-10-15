#include "tgaimage.h"
#include "rasterizer.hpp"
#include "myEigen.hpp"
#include "geometry.h"
#include <string>
#include <chrono>
#include <random>

using std::cout;
using std::cin;
using std::endl;




int main(int argc, char** argv) {

    uint16_t frame = 0;
    auto begin = std::chrono::steady_clock::now();
    float angle = 0;

    std::vector<std::shared_ptr<rst::Triangle>> TriangleList;

    {
        myEigen::Vector3f v[3];
        v[0] = myEigen::Vector3f(2, 0, -2);
        v[1] = myEigen::Vector3f(0, 2, -2);
        v[2] = myEigen::Vector3f(-2, 0, -2);
        std::shared_ptr<rst::Triangle> t1(new rst::Triangle(v));
        t1->setColor(0, TGAColor(255, 0, 0));
        t1->setColor(1, TGAColor(0, 255, 0));
        t1->setColor(2, TGAColor(0, 0, 255));
        TriangleList.push_back(t1);
    }
    {
        myEigen::Vector3f v[3];
        v[0] = myEigen::Vector3f(3.5, -1, -5);
        v[1] = myEigen::Vector3f(2.5, 1.5, -5);
        v[2] = myEigen::Vector3f(-1, 0.5, -5);
        std::shared_ptr<rst::Triangle> t1(new rst::Triangle(v));
        t1->setColor(0, TGAColor(255, 0, 0));
        t1->setColor(1, TGAColor(0, 255, 0));
        t1->setColor(2, TGAColor(0, 0, 255));
        TriangleList.push_back(t1);
    }

    while (true)
    {
        std::string filename = "result\\output" + std::to_string(frame);
        TGAImage image(700, 700, TGAImage::RGB);
        
        rst::rasterizer rst(filename, image);
        rst.SetCamera(myEigen::Vector3f(0, 0, 5));
        rst.SetTheta(angle);
        rst.SetRotateAxis(myEigen::Vector3f(1, 1, 0));
        //rst.TurnOnBackCulling();
        rst.draw(TriangleList);
        rst.output();
        cout << "frame:" << frame << endl;
        frame++;
        angle += 5;
        if (frame > 71) {
            auto end = std::chrono::steady_clock::now();
            cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
            return 0;
        }
    }
    auto end = std::chrono::steady_clock::now();
    cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
    return 0;
}