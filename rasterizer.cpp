#include "tgaimage.h"
#include "rasterizer.hpp"
#include <iostream>
#include "myEigen.hpp"

rst::rasterizer::rasterizer(std::string f, TGAImage& img)
	:filename(f), image(img), width(img.width()), height(img.height())
{	
	image.flip_vertically(); /*让坐标原点位于图像左下角*/
}

void rst::rasterizer::draw_line(myEigen::Vector2i v0, myEigen::Vector2i v1) {
	int x, y, dx, dy, dxa, dya, px, py, xb, yb, i;
	x = v0.x;
	y = v0.y;
	dx = v1.x - v0.x;
	dy = v1.y - v0.y;
	dxa = abs(dx);
	dya = abs(dy);
	px = 2 * dya - dxa;
	py = 2 * dxa - dya;
	image.set(x, y, red);
	if (dya < dxa) {
		if (dx > 0) {
			x = v0.x;
			y = v0.y;
			xb = v1.x;
		}
		else
		{
			x = v1.x;
			y = v1.y;
			xb = v0.x;
		}
		for (i = 0; x <= xb; i++) {
			x++;
			if (px > 0) {
				if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0)) {
					y++;
				}
				else
				{
					y--;
				}
				px += 2 * dya - 2 * dxa;
			}
			else
			{
				px += 2 * dya;
			}
			image.set(x, y, red);
		}
	}
	else
	{
		if (dy > 0) {
			x = v0.x;
			y = v0.y;
			yb = v1.y;
		}
		else
		{
			x = v1.x;
			y = v1.y;
			yb = v0.y;
		}
		for (i = 0; y <= yb; i++) {
			y++;
			if (py > 0) {
				if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0)) {
					x++;
				}
				else
				{
					x--;
				}
				py += 2 * dxa - 2 * dya;
			}
			else
			{
				py += 2 * dxa;
			}
			image.set(x, y, red);
		}
	}
}

void rst::rasterizer::output() {
	image.write_tga_file(filename);
}