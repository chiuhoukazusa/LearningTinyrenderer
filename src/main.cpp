#include "tgaimage.h"
#include "rasterizer.hpp"
#include "myEigen.hpp"
#include "geometry.h"
#include "objLoader.h"
#include "shader.h"
#include "transform.h"
#include <string>
#include <chrono>
#include <random>

using std::cout;
using std::cin;
using std::endl;

//load shader
auto eye_pos  = myEigen::Vector3f(0, 0.8, 2.5);
auto gaze_dir = myEigen::Vector3f(0, 0, -1);
auto view_up  = myEigen::Vector3f(0, 1, 0);
float theta   = 0;
int width     = 700;
int height    = 700;

struct Properties
{
    rst::Transform projection = rst::Perspective(0.1, 50, 45, width / height);
    rst::Transform view       = rst::Camera(eye_pos, gaze_dir, view_up);
    rst::Transform model      = rst::Modeling(myEigen::Vector3f(0),
                                myEigen::Vector3f(1),
                                myEigen::Vector3f(0, 1, 0), theta);
    rst::PointLight light     = rst::PointLight(myEigen::Vector3f(0, 500, -200), 30000.0);
};

int main(int argc, char** argv) {

    uint16_t frame = 0;
    auto begin = std::chrono::steady_clock::now();
    float angle = 0;

    std::vector<std::shared_ptr<rst::Triangle>> TriangleList;
    Properties p;
    rst::Transform mv = p.view * p.model;
    p.light.position = mv(p.light.position);
    
    //load model
    std::string root = "obj/Elden Ring - Melina_XPS/";
    rst::Model model(root, "Melina.obj");
    //std::string root = "obj/mary/";
    //rst::Model model(root, "Marry.obj");
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
        
        rst::rasterizer rst(filename, image, p.light);
        //rst.SetCamera(myEigen::Vector3f(0, 1.8, 5));
        rst.SetCamera(eye_pos);
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