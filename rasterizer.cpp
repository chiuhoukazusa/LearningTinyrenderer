#include "tgaimage.h"
#include "rasterizer.hpp"
#include <iostream>

rst::rasterizer::rasterizer(std::string f, TGAImage& img) {
	filename = f;
	image = img;
	width = image.width();
	height = image.height();
	image.flip_vertically();//让坐标原点位于图像左下角
}

void rst::rasterizer::draw_line(int x0, int y0, int x1, int y1) {
	int x, y, dx, dy, dxa, dya, px, py, xb, yb, i;
	x = x0;
	y = y0;
	dx = x1 - x0;
	dy = y1 - y0;
	dxa = abs(dx);
	dya = abs(dy);
	px = 2 * dya - dxa;
	py = 2 * dxa - dya;
	image.set(x, y, red);
	if (dya < dxa) {
		if (dx > 0) {
			x = x0;
			y = y0;
			xb = x1;
		}
		else
		{
			x = x1;
			y = y1;
			xb = x0;
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
			x = x0;
			y = y0;
			yb = y1;
		}
		else
		{
			x = x1;
			y = y1;
			yb = y0;
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