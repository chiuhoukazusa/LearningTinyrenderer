#include "tgaimage.h"
#include "rasterizer.hpp"
#include "myEigen.hpp"

using std::cout;
using std::cin;
using std::endl;


const TGAColor white = TGAColor(255, 255, 255, 255);//白色
const TGAColor red = TGAColor(255, 0, 0, 255);//红色


int main(int argc, char** argv) {
    std::string filename = "output.tga";
    TGAImage image(700, 700, TGAImage::RGB);
    rst::rasterizer rst(filename, image);
    myEigen::Vector2i v0(50, 50);
    myEigen::Vector2i v1(0, 50);
    myEigen::Vector2i v2(50, 0);
    myEigen::Vector2i v3(100, 50);
    myEigen::Vector2i v4(50, 100);
    myEigen::Vector2i v5(0, 40);
    myEigen::Vector2i v6(0, 60);
    myEigen::Vector2i v7(100, 40);
    myEigen::Vector2i v8(100, 60);
    myEigen::Vector2i v9(40, 0);
    myEigen::Vector2i v10(60, 0);
    myEigen::Vector2i v11(40, 100);
    myEigen::Vector2i v12(60, 100);

    myEigen::Vector3i q;
    cout << myEigen::toVector4(q) << endl;
    myEigen::Matrixf4x4 a(myEigen::toVector4(q), myEigen::toVector4(q), myEigen::toVector4(q), myEigen::toVector4(q));
    cout << a << endl;
    cout << myEigen::Matrix4x4Transpose(a) << endl;
    cout << myEigen::Matrix4x4CMinor(a, 0, 3) << endl;
    myEigen::Matrixf4x4 b(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 4.0f, 0.0f,
        1.0f, 2.0f, 3.0f, 1.0f);
    cout << myEigen::Matrix4x4Determinant(b) << endl;
    cout << myEigen::Matrix4x4Inverse(b) << endl;
    cout << b * myEigen::Matrix4x4Inverse(b) << endl;

    rst.draw_line(v1, v3);
    rst.draw_line(v2, v4);
    rst.draw_line(v0, v10);
    rst.draw_line(v0, v9);
    rst.draw_line(v0, v5);
    rst.draw_line(v0, v6);
    rst.draw_line(v0, v8);
    rst.draw_line(v0, v12);
    rst.draw_line(v0, v7);
    rst.draw_line(v0, v11);
    rst.output();
    return 0;
}