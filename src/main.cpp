#include "tgaimage.h"
#include "rasterizer.hpp"
#include "myEigen.hpp"
#include <string>
#include <chrono>

using std::cout;
using std::cin;
using std::endl;




int main(int argc, char** argv) {

    uint16_t frame = 0;
    auto begin = std::chrono::steady_clock::now();
    float angle = 0;

    while (true)
    {
        std::string filename = "result\\output" + std::to_string(frame);
        
        TGAImage image(700, 700, TGAImage::RGB);
        rst::rasterizer rst(filename, image);
        rst.SetCamera(myEigen::Vector3f(0, 0, 5));
        rst.SetTheta(angle);
        rst.SetRotateAxis(myEigen::Vector3f(1, 1, 0));
        rst.draw();
        rst.output();
        cout << "frame:" << frame << endl;
        frame++;
        angle += 5;
        if (frame > 100) {
            auto end = std::chrono::steady_clock::now();
            cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
            return 0;
        }
    }
    auto end = std::chrono::steady_clock::now();
    cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
    return 0;
}