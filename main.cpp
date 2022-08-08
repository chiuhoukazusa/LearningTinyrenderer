#include "tgaimage.h"
#include "rasterizer.hpp"


const TGAColor white = TGAColor(255, 255, 255, 255);//白色
const TGAColor red = TGAColor(255, 0, 0, 255);//红色


int main(int argc, char** argv) {
    std::string filename = "output.tga";
    TGAImage image(100, 100, TGAImage::RGB);
    rst::rasterizer rst(filename, image);
    rst.draw_line(0, 50, 100, 50);
    rst.draw_line(50, 0, 50, 100);
    rst.draw_line(50, 50, 60, 0);
    rst.draw_line(50, 50, 40, 0);
    rst.draw_line(50, 50, 0, 40);
    rst.draw_line(50, 50, 0, 60);
    rst.draw_line(50, 50, 100, 60);
    rst.draw_line(50, 50, 60, 100);
    rst.draw_line(50, 50, 100, 40);
    rst.draw_line(50, 50, 40, 100);
    rst.output();
    return 0;
}