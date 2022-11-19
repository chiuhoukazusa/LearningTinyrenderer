#include "tgaimage.h"
#include "rasterizer.hpp"
#include "myEigen.hpp"
#include "geometry.h"
#include "objLoader.h"
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

    //std::string root = "obj/Elden Ring - Melina_XPS/";
    //rst::Model model(root, "Melina.obj");
    std::string root = "obj/mary/";
    rst::Model model(root, "Marry.obj");
    for (auto& o:model.objects)
    {
        for (auto& m : o.meshes)
        {
            for (auto& t : m.primitives)
            {
                std::shared_ptr<rst::Triangle> t1(new rst::Triangle(t));
                TriangleList.push_back(t1);
            }
        }
    }

    while (true)
    {
        std::string filename = "result/output" + std::to_string(frame);
        TGAImage image(700, 700, TGAImage::RGB);
        
        rst::rasterizer rst(filename, image);
        rst.SetCamera(myEigen::Vector3f(0, 1.8, 5));
        //rst.SetCamera(myEigen::Vector3f(0, 0.8, 2.5));
        rst.SetTheta(angle);
        rst.SetRotateAxis(myEigen::Vector3f(0, 1, 0));
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