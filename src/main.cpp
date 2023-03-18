#include "tgaimage.h"
#include "rasterizer.hpp"
#include "myEigen.hpp"
#include "geometry.h"
#include "objLoader.h"
#include "shader.h"
#include "transform.h"
#include "config.h"
#include "shader/shadersrc/BlinnPhongShader.h"
#include <string>
#include <chrono>
#include <random>
using std::cout;
using std::cin;
using std::endl;


//loadShader
namespace rst
{

}

int main(int argc, char** argv)
{
    uint16_t frame = 0;
    auto begin = std::chrono::steady_clock::now();
    float angle = 0;

    std::vector<std::shared_ptr<rst::Mesh>> MeshList;
    
    //load model
    std::string root = "obj/Elden Ring - Melina_XPS/";
    rst::Model model(root, "Melina.obj");
    //std::string root = "obj/diablo3_pose/";
    //rst::Model model(root, "diablo3_pose.obj");
    //std::string root = "obj/mary/";
    //rst::Model model(root, "Marry.obj");

    for (auto& o:model.objects)
    {
        for (auto& m : o.meshes)
        {
            std::shared_ptr<rst::Mesh> m1 = std::make_shared<rst::Mesh>(m);
            MeshList.push_back(m1);
        }
    }

    rst::BlinnPhongShader shader;
    light.position = rst::Camera(eye_pos, gaze_dir, view_up)(light.position);
    shader.lights.push_back(light);

    while (true)
    {
        std::string filename = "result/output" + std::to_string(frame);
        TGAImage image(700, 700, TGAImage::RGB);
        for (int i = 0; i < 700; i++)
        {
            for (int j = 0; j < 700; j++)
            {
                image.set(i, j, TGAColor(255, 255, 255, 255));
            }
        }
        
        rst::rasterizer rst(filename, image, light);
        //rst.SetCamera(myEigen::Vector3f(0, 1.8, 5));
        rst.SetCamera(eye_pos);
        rst.SetTheta(angle);

        shader.theta_uniform = angle;

        rst.SetRotateAxis(myEigen::Vector3f(0, 1, 0));
        //rst.TurnOnBackCulling();
        rst.draw(MeshList, shader);
        rst.output();
        cout << "frame:" << frame << endl;
        frame++;
        angle += 5;
        if (frame > 1) {
            auto end = std::chrono::steady_clock::now();
            cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
            return 0;
        }
    }
    auto end = std::chrono::steady_clock::now();
    cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
    return 0;
}