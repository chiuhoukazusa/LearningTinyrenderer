#include "tgaimage.h"
#include "rasterizer.hpp"
#include <iostream>
#include "myEigen.hpp"
#include "transform.h"
#include "geometry.h"
#include <array>
#include <algorithm>

namespace rst {
	rasterizer::rasterizer(const std::string& f, const TGAImage& img)
		:filename(f), image(img), width(img.width()), height(img.height()),
		zneardis(0.1f), zfardis(50), fovY(45), aspect(1)
	{
		image.flip_vertically(); /*让坐标原点位于图像左下角*/

		eye_pos = myEigen::Vector3f(0, 0, 0);
		gaze_dir = myEigen::Vector3f(0, 0, -1);
		view_up = myEigen::Vector3f(0, 1, 0);
		z_buffer.resize(img.width() * img.height());
		std::fill(z_buffer.begin(), z_buffer.end(), -std::numeric_limits<float>::infinity());
	}

	void rasterizer::draw_line(const myEigen::Vector2i& v0, const myEigen::Vector2i& v1) {
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

	void rasterizer::output() {
		image.write_tga_file(filename + ".tga");
	}

	void rasterizer::SetCamera(const myEigen::Vector3f& eye_pos)
	{
		this->eye_pos = eye_pos;
	}
	void rasterizer::SetGaze(const myEigen::Vector3f& gaze_dir)
	{
		this->gaze_dir = gaze_dir;
	}
	void rasterizer::SetViewUp(const myEigen::Vector3f& view_up)
	{
		this->view_up = view_up;
	}
	void rasterizer::SetTheta(float theta)
	{
		this->theta = theta;
	}
	void rasterizer::SetRotateAxis(const myEigen::Vector3f& rotateAxis)
	{
		this->rotateAxis = rotateAxis;
	}
	void rasterizer::TurnOnBackCulling()
	{
		this->backCulling = true;
	}
	void rasterizer::TurnOffBackCulling()
	{
		this->backCulling = false;
	}
	void rasterizer::SetVertexOrder(const TriangleVertexOrder& t)
	{
		this->vertexOrder = t;
	}
	TriangleVertexOrder rasterizer::GetVertexOrder()
	{
		auto v = this->vertexOrder;
		return v;
	}
	int rasterizer::get_index(int x, int y)
	{
		return y * width + x;
	}

	void rasterizer::draw(std::vector<std::shared_ptr<rst::Triangle>>& TriangleList)
	{
		Transform projection = Perspective(zneardis, zfardis, fovY, aspect);
		Transform  mv = Camera(eye_pos, gaze_dir, view_up) * Modeling(myEigen::Vector3f(0),
			myEigen::Vector3f(1),
			myEigen::Vector3f(rotateAxis), theta);
		Transform viewport = Viewport(width, height);

		for (const auto& t : TriangleList)
		{
			Triangle NewTriangle = *t;

			//Model Space -> World Space -> Camera Space
			std::array<myEigen::Vector4f, 3> vert
			{
				(mv(t->vertex[0].vertex)),
				(mv(t->vertex[1].vertex)),
				(mv(t->vertex[2].vertex))
			};

			//BackCulling
			if (backCulling) {
				auto v1 = vert[1] - vert[0];
				auto v2 = vert[2] - vert[1];
				auto v = myEigen::crossProduct(myEigen::Vector3f(v1.x, v1.y, v1.z), myEigen::Vector3f(v2.x, v2.y, v2.z));
				auto gaze = myEigen::Vector3f(vert[0].x, vert[0].y, vert[0].z);
				if (vertexOrder == TriangleVertexOrder::counterclockwise)
				{
					if (myEigen::dotProduct(v, gaze) >= 0) return;
				}
				else
				{
					if (myEigen::dotProduct(v, gaze) <= 0) return;
				}
			}

			//Transform Normal
			auto normalMV = mv;
			normalMV.toNormal();
			std::array<myEigen::Vector4f, 3> normal
			{
				(normalMV(t->vertex[0].normal)),
				(normalMV(t->vertex[1].normal)),
				(normalMV(t->vertex[2].normal))
			};

			//Camera Space -> Homogeneous Clipping Space
			for (auto& v : vert)
			{
				v = projection(v);
			}

			//Homogeneous Clipping 
			for (auto& v : vert)
			{
				if (v.w > -zneardis || v.w < -zfardis) return;
			}
			auto clipSpacePos = vert;

			//Homogeneous Clipping Space -> Canonical View Volume(CVV)
			for (auto& v : vert)
			{
				v /= v.w;
			}

			//Canonical View Volume(CVV) -> Screen Space
			for (auto& v : vert)
			{
				v = viewport(v);
			}

			for (size_t i = 0; i < 3; i++)
			{
				NewTriangle.setVertexPos(i, vert[i]);
				NewTriangle.setColor(i, t->vertex[i].vertexColor);
				NewTriangle.setNormal(i, normal[i]);
			}

			//Viewport Clipping
			auto NewVert = clip_Cohen_Sutherland(NewTriangle, clipSpacePos);
			if (NewVert.empty()) return;
			
			for (size_t i = 0; i < NewVert.size() - 2; i++)
			{
				rasterize_edge_walking(Triangle(NewVert[0], NewVert[1 + i], NewVert[2 + i]), clipSpacePos);
				//rasterize_edge_equation(Triangle(NewVert[0], NewVert[1 + i], NewVert[2 + i]), clipSpacePos);
				//rasterize_wireframe(Triangle(NewVert[0], NewVert[1 + i], NewVert[2 + i]));
			}
			
			//rasterize_edge_walking(NewTriangle, clipSpacePos);
			
		}
	}

	static Vertex clip_vert(const Vertex& _v1, const Vertex& _v2, int& code1, const int& code2,
		const int leftBound, const int rightBound, const int bottomBound, const int topBound, 
		const std::array<myEigen::Vector4f, 2>& clipSpacePos)
	{
		const int left = 1;
		const int right = 2;
		const int bottom = 4;
		const int top = 8;

		auto v1 = _v1;
		auto& v2 = _v2;
		if ((left & code1) != 0)
		{
			float lerpNumber = (leftBound - v1.vertex.x) / (v2.vertex.x - v1.vertex.x);
			v1 = perspectiveLerp(_v1, _v2, lerpNumber, clipSpacePos[0], clipSpacePos[1]);
			v1.vertex = lerp(_v1.vertex, _v2.vertex, lerpNumber);
			v1.vertex.x += 0.001f;
		}
		else if ((right & code1) != 0)
		{
			float lerpNumber = (v1.vertex.x - rightBound) / (v1.vertex.x - v2.vertex.x);
			v1 = perspectiveLerp(_v1, _v2, lerpNumber, clipSpacePos[0], clipSpacePos[1]);
			v1.vertex = lerp(_v1.vertex, _v2.vertex, lerpNumber);
			v1.vertex.x -= 0.001f;
		}
		else if ((bottom & code1) != 0)
		{
			float lerpNumber = (bottomBound - v1.vertex.y) / (v2.vertex.y - v1.vertex.y);
			v1 = perspectiveLerp(_v1, _v2, lerpNumber, clipSpacePos[0], clipSpacePos[1]);
			v1.vertex = lerp(_v1.vertex, _v2.vertex, lerpNumber);
			v1.vertex.y += 0.001f;
		}
		else if ((top & code1) != 0)
		{
			float lerpNumber = (v1.vertex.y - topBound) / (v1.vertex.y - v2.vertex.y);
			v1 = perspectiveLerp(_v1, _v2, lerpNumber, clipSpacePos[0], clipSpacePos[1]);
			v1.vertex = lerp(_v1.vertex, _v2.vertex, lerpNumber);
			v1.vertex.y -= 0.001f;
		}

		code1 = 0;
		if (v1.vertex.x < leftBound) code1 = (code1 | left);
		if (v1.vertex.x > rightBound) code1 = (code1 | right);
		if (v1.vertex.y < bottomBound) code1 = (code1 | bottom);
		if (v1.vertex.y > topBound) code1 = (code1 | top);

		return v1;
	}

	Line rasterizer::clip_line(const Line& line,
		const std::array<myEigen::Vector4f, 2>& clipSpacePos)
	{
		Vertex v[2];
		v[0] = line.v1, v[1] = line.v2;
		int code[2];
		code[0] = 0, code[1] = 0;
		const int left = 1;
		const int right = 2;
		const int bottom = 4;
		const int top = 8;

		auto vert1 = v[0];
		auto vert2 = v[1];

		for (size_t i = 0; i < 2; i++)
		{
			if (v[i].vertex.x < 0) code[i] = code[i] | 1;
			if (v[i].vertex.x > width) code[i] = code[i] | 2;
			if (v[i].vertex.y < 0) code[i] = code[i] | 4;
			if (v[i].vertex.y > height) code[i] = code[i] | 8;
		}

		while (code[0] != 0 || code[1] != 0)
		{
			if ((code[0] & code[1]) != 0) return Line();
			vert1 = clip_vert(vert1, vert2, code[0], code[1], 0, width, 0, height, { clipSpacePos[0], clipSpacePos[1] });
			vert2 = clip_vert(vert2, vert1, code[1], code[0], 0, width, 0, height, { clipSpacePos[1], clipSpacePos[0] });
		}
		return Line(vert1, vert2);
	}

	std::vector<Vertex> rasterizer::clip_Cohen_Sutherland(const Triangle& t,
		const std::array<myEigen::Vector4f, 3>& clipSpacePos)
	{
		auto v = t.vertex;

		std::array<Line, 3> line
		{
			Line(v[0], v[1]),
			Line(v[1], v[2]),
			Line(v[2], v[0])
		};

		line[0] = clip_line(line[0], { clipSpacePos[0], clipSpacePos[1] });
		line[1] = clip_line(line[1], { clipSpacePos[1], clipSpacePos[2] });
		line[2] = clip_line(line[2], { clipSpacePos[2], clipSpacePos[0] });

		int cnt = 0;
		for (auto& i : line) { if (i.empty()) cnt++; }

		if (cnt == 3) return {};
		else if (cnt == 2)
		{
			return {};
		}
		else if (cnt == 1)
		{
			for (size_t i = 0; i < 3; i++)
			{
				if (line[i].empty())
				{
					if (i == 0) {
						line[i].v1 = line[2].v2;
						line[i].v2 = line[1].v1;
					}
					else if (i == 1)
					{
						line[i].v1 = line[0].v2;
						line[i].v2 = line[2].v1;
					}
					else if (i == 2)
					{
						line[i].v1 = line[1].v2;
						line[i].v2 = line[0].v1;
					}
				}
			}
		}


		std::vector<Vertex> newVert;
		newVert.reserve(6);

		if (fabs(line[2].v2.vertex.x - line[0].v1.vertex.x) < 0.0001f && fabs(line[2].v2.vertex.y - line[0].v1.vertex.y) < 0.0001f)
		{
			newVert.emplace_back(line[0].v1);
		}
		else
		{
			newVert.emplace_back(line[2].v2);
			newVert.emplace_back(line[0].v1);
		}
		if (fabs(line[0].v2.vertex.x - line[1].v1.vertex.x) < 0.0001f && fabs(line[0].v2.vertex.y - line[1].v1.vertex.y) < 0.0001f)
		{
			newVert.emplace_back(line[1].v1);
		}
		else
		{
			newVert.emplace_back(line[0].v2);
			newVert.emplace_back(line[1].v1);
		}
		if (fabs(line[1].v2.vertex.x - line[2].v1.vertex.x) < 0.0001f && fabs(line[1].v2.vertex.y - line[2].v1.vertex.y) < 0.0001f)
		{
			newVert.emplace_back(line[2].v1);
		}
		else
		{
			newVert.emplace_back(line[1].v2);
			newVert.emplace_back(line[2].v1);
		}

		return newVert;
	}

	void rasterizer::rasterize_wireframe(const Triangle& m)
	{
		myEigen::Vector2i a(std::floor(m.vertex[0].vertex.x + 0.5), std::floor(m.vertex[0].vertex.y + 0.5));
		myEigen::Vector2i b(std::floor(m.vertex[1].vertex.x + 0.5), std::floor(m.vertex[1].vertex.y + 0.5));
		myEigen::Vector2i c(std::floor(m.vertex[2].vertex.x + 0.5), std::floor(m.vertex[2].vertex.y + 0.5));

		draw_line(a, b);
		draw_line(b, c);
		draw_line(c, a);
	}

	void rasterizer::rasterize_edge_walking(const Triangle& m, const std::array<myEigen::Vector4f, 3>& clipSpacePos)
	{
		Triangle t = m;
		auto csp = clipSpacePos;
		if (t.vertex[0].vertex.y > t.vertex[1].vertex.y)
			std::swap(t.vertex[0], t.vertex[1]); std::swap(csp[0], csp[1]);
		if (t.vertex[0].vertex.y > t.vertex[2].vertex.y)
			std::swap(t.vertex[0], t.vertex[2]); std::swap(csp[0], csp[2]);
		if (t.vertex[1].vertex.y > t.vertex[2].vertex.y)
			std::swap(t.vertex[1], t.vertex[2]); std::swap(csp[1], csp[2]);
		float longEdge = t.vertex[2].vertex.y - t.vertex[0].vertex.y;
		if (longEdge == 0) { return; }
		//scan the bottom triangle
		for (int y = std::ceil(t.vertex[0].vertex.y - 0.5f); y < std::ceil(t.vertex[1].vertex.y - 0.5f); y++)
		{
			float shortEdge = t.vertex[1].vertex.y - t.vertex[0].vertex.y;

			float shortLerp = ((float)y + 0.5f - t.vertex[0].vertex.y) / shortEdge;
			float longLerp = ((float)y + 0.5f - t.vertex[0].vertex.y) / longEdge;
			
			Vertex shortVertex = lerp(t.vertex[0], t.vertex[1], shortLerp);
			Vertex longVertex = lerp(t.vertex[0], t.vertex[2], longLerp);
			Vertex shortVertexC = perspectiveLerp(t.vertex[0], t.vertex[1], shortLerp, csp[0], csp[1]);
			Vertex longVertexC = perspectiveLerp(t.vertex[0], t.vertex[2],longLerp, csp[0], csp[2]);

			if (shortVertex.vertex.x > longVertex.vertex.x)
				std::swap(shortVertex, longVertex);
			for (int i = std::ceil(shortVertex.vertex.x - 0.5f); i < std::ceil(longVertex.vertex.x - 0.5f); i++)
			{
				float lerpNumber = ((float)i + 0.5f - shortVertex.vertex.x) / (longVertex.vertex.x - shortVertex.vertex.x);
				Vertex pixel = perspectiveLerp(shortVertex, longVertex, lerpNumber, shortVertexC.vertex, longVertexC.vertex);
				if (pixel.vertex.z > z_buffer[get_index(i, y)])
				{
					image.set(i, y, pixel.vertexColor);
					z_buffer[get_index(i, y)] = pixel.vertex.z;
				}
			}
		}
		//scan the top triangle
		for (int y = std::ceil(t.vertex[1].vertex.y - 0.5f); y < std::ceil(t.vertex[2].vertex.y - 0.5f); y++)
		{
			float shortEdge = t.vertex[2].vertex.y - t.vertex[1].vertex.y;

			float shortLerp = ((float)y + 0.5f - t.vertex[1].vertex.y) / shortEdge;
			float longLerp = ((float)y + 0.5f - t.vertex[0].vertex.y) / longEdge;
			
			Vertex shortVertex = lerp(t.vertex[1], t.vertex[2], shortLerp);
			Vertex longVertex = lerp(t.vertex[0], t.vertex[2], longLerp);
			Vertex shortVertexC = perspectiveLerp(t.vertex[1], t.vertex[2], shortLerp, csp[1], csp[2]);
			Vertex longVertexC = perspectiveLerp(t.vertex[0], t.vertex[2], longLerp, csp[0], csp[2]);
			
			if (shortVertex.vertex.x > longVertex.vertex.x)
				std::swap(shortVertex, longVertex);
			for (int i = std::ceil(shortVertex.vertex.x - 0.5f); i < std::ceil(longVertex.vertex.x - 0.5f); i++)
			{
				float lerpNumber = ((float)i + 0.5f - shortVertex.vertex.x) / (longVertex.vertex.x - shortVertex.vertex.x);
				Vertex pixel = perspectiveLerp(shortVertex, longVertex, lerpNumber, shortVertexC.vertex, longVertexC.vertex);
				if (pixel.vertex.z > z_buffer[get_index(i, y)])
				{
					image.set(i, y, pixel.vertexColor);
					z_buffer[get_index(i, y)] = pixel.vertex.z;
				}
			}
		}

	}


	static int TopLeftRules(const float side, const myEigen::Vector4f& v1, const myEigen::Vector4f& v2,
		const rst::TriangleVertexOrder& Order)
	{
		if (Order == TriangleVertexOrder::counterclockwise) {
			if (fabs(side) < 1e-6) {
				return ((v1.y > v2.y) || (v1.y == v2.y && v1.x > v2.x)) ? 0 : -1;
			} else { return 0; }
		}
		else
		{
			if (fabs(side) < 1e-6) {
				return ((v1.y < v2.y) || (v1.y == v2.y && v1.x < v2.x)) ? 0 : -1;
			}
			else { return 0; }
		}
	}

	bool rasterizer::insideTriangle(const Triangle& m, const float x, const float y)
	{
		myEigen::Vector4f v1 = m.vertex[0].vertex;
		myEigen::Vector4f v2 = m.vertex[1].vertex;
		myEigen::Vector4f v3 = m.vertex[2].vertex;

		float side1 = (v2.y - v1.y) * x + (v1.x - v2.x) * y + v2.x * v1.y - v1.x * v2.y;
		float side2 = (v3.y - v2.y) * x + (v2.x - v3.x) * y + v3.x * v2.y - v2.x * v3.y;
		float side3 = (v1.y - v3.y) * x + (v3.x - v1.x) * y + v1.x * v3.y - v3.x * v1.y;

		side1 += TopLeftRules(side1, v1, v2, vertexOrder);
		side2 += TopLeftRules(side2, v2, v3, vertexOrder);
		side3 += TopLeftRules(side3, v3, v1, vertexOrder);

		return (side1 >= 0 && side2 >= 0 && side3 >= 0) || (side1 <= 0 && side2 <= 0 && side3 <= 0);
	}

	void rasterizer::rasterize_edge_equation(const Triangle& m, const std::array<myEigen::Vector4f, 3>& clipSpacePos)
	{
		int boundingTop = std::ceil(std::max({ m.vertex[0].vertex.y, m.vertex[1].vertex.y, m.vertex[2].vertex.y }));
		int boundingBottom = std::floor(std::min({ m.vertex[0].vertex.y, m.vertex[1].vertex.y, m.vertex[2].vertex.y }));
		int boundingRight = std::ceil(std::max({ m.vertex[0].vertex.x, m.vertex[1].vertex.x, m.vertex[2].vertex.x }));
		int boundingLeft = std::floor(std::min({ m.vertex[0].vertex.x, m.vertex[1].vertex.x, m.vertex[2].vertex.x }));

		for (int y = boundingBottom; y < boundingTop; y++)
		{
			for (int x = boundingLeft; x < boundingRight; x++) 
			{
				if (insideTriangle(m, (float)x + 0.5, (float)y + 0.5)) 
				{
					Vertex pixel = barycentricPerspectiveLerp(m, myEigen::Vector2f(x + 0.5, y + 0.5), clipSpacePos);
					if (pixel.vertex.z > z_buffer[get_index(x, y)]) 
					{
						image.set(x, y, pixel.vertexColor);
						z_buffer[get_index(x, y)] = pixel.vertex.z;
					}
				}
			}
		}
	}


}
