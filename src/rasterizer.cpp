#include "tgaimage.h"
#include "rasterizer.hpp"
#include <iostream>
#include "myEigen.hpp"
#include "transform.h"
#include "triangle.h"

rst::rasterizer::rasterizer(std::string f, TGAImage& img)
	:filename(f), image(img), width(img.width()), height(img.height()), 
	zneardis(0.1), zfardis(50), fovY(45), aspect(1)
{	
	image.flip_vertically(); /*让坐标原点位于图像左下角*/

	eye_pos = myEigen::Vector3f(0, 0, 0);
	gaze_dir = myEigen::Vector3f(0, 0, -1);
	view_up = myEigen::Vector3f(0, 1, 0);
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
	image.write_tga_file(filename + ".tga");
}

void rst::rasterizer::SetCamera(myEigen::Vector3f eye_pos)
{
	this->eye_pos = eye_pos;
}
void rst::rasterizer::SetGaze(myEigen::Vector3f gaze_dir)
{
	this->gaze_dir = gaze_dir;
}
void rst::rasterizer::SetViewUp(myEigen::Vector3f view_up)
{
	this->view_up = view_up;
}
void rst::rasterizer::SetTheta(float theta)
{
	this->theta = theta;
}
void rst::rasterizer::SetRotateAxis(myEigen::Vector3f rotateAxis)
{
	this->rotateAxis = rotateAxis;
}

void rst::rasterizer::draw()
{
	Transform mvp = Perspective(zneardis, zfardis, fovY, aspect)
		* Camera(eye_pos, gaze_dir, view_up) * Modeling(myEigen::Vector3f(0),
			myEigen::Vector3f(1),
			myEigen::Vector3f(rotateAxis), theta);
	Transform viewport = Viewport(width, height);

	for (size_t i = 0; i < 3; i++)
	{
		m.t.vertex[i] = mvp(m.t.vertex[i]);
		m.t.vertex[i] /= m.t.vertex[i].w;
		m.t.vertex[i] = viewport(m.t.vertex[i]);
	}

	m.t.setColor(0, TGAColor(255, 0, 0, 0));
	m.t.setColor(1, TGAColor(0, 255, 0, 0));
	m.t.setColor(2, TGAColor(0, 0, 255, 0));

	rasterize_wireframe(m);
}

void rst::rasterizer::rasterize_wireframe(const model& m)
{
	myEigen::Vector2i a(m.t.vertex[0].x, m.t.vertex[0].y);
	myEigen::Vector2i b(m.t.vertex[1].x, m.t.vertex[1].y);
	myEigen::Vector2i c(m.t.vertex[2].x, m.t.vertex[2].y);

	draw_line(a, b);
	draw_line(b, c);
	draw_line(c, a);
}
