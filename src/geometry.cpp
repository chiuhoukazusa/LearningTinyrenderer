#include "geometry.h"
#include "myEigen.hpp"
#include <array>

namespace rst {
	Vertex barycentricLerp(const Triangle& t, const myEigen::Vector2f& pixel)
	{
		Vertex v1 = t.vertex[0]; Vertex v2 = t.vertex[1]; Vertex v3 = t.vertex[2];
		float xa = v1.vertex.x; float ya = v1.vertex.y;
		float xb = v2.vertex.x; float yb = v2.vertex.y;
		float xc = v3.vertex.x; float yc = v3.vertex.y;
		float x = pixel.x; float y = pixel.y;

		float gamma = ((ya - yb) * x + (xb - xa) * y + xa * yb - xb * ya) /
			((ya - yb) * xc + (xb - xa) * yc + xa * yb - xb * ya);
		float beta = ((ya - yc) * x + (xc - xa) * y + xa * yc - xc * ya) /
			((ya - yc) * xb + (xc - xa) * yb + xa * yc - xc * ya);
		float alpha = 1 - beta - gamma;

		return v1 * alpha + v2 * beta + v3 * gamma;
	}
	Vertex barycentricPerspectiveLerp(const Triangle& t, const myEigen::Vector2f& pixel
		, const std::array<myEigen::Vector4f, 3>& v)
	{
		Vertex v1 = t.vertex[0]; Vertex v2 = t.vertex[1]; Vertex v3 = t.vertex[2];
		float xa = v1.vertex.x; float ya = v1.vertex.y;
		float xb = v2.vertex.x; float yb = v2.vertex.y;
		float xc = v3.vertex.x; float yc = v3.vertex.y;
		float x = pixel.x; float y = pixel.y;

		float g = ((ya - yb) * x + (xb - xa) * y + xa * yb - xb * ya) /
			((ya - yb) * xc + (xb - xa) * yc + xa * yb - xb * ya);
		float b = ((ya - yc) * x + (xc - xa) * y + xa * yc - xc * ya) /
			((ya - yc) * xb + (xc - xa) * yb + xa * yc - xc * ya);
		float a = 1 - b - g;

		float W = 1.0f / (a / v[0].w + b / v[1].w + g / v[2].w);
		float beta = (b / v[1].w) * W;
		float gamma = (g / v[2].w)* W;
		float alpha = 1.0f - beta - gamma;
		
		return v1 * alpha + v2 * beta + v3 * gamma;
	}
	Triangle::Triangle() {
		for (int i = 0; i < 3; i++)
		{
			vertex[i].vertex = myEigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
			vertex[i].normal = myEigen::Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
			vertex[i].vertexColor = TGAColor();
		}
	}

	Triangle::Triangle(myEigen::Vector3f v[3])
	{
		for (size_t i = 0; i < 3; i++)
		{
			setVertexPos(i, myEigen::Vector4f(v[i].x, v[i].y, v[i].z, 1));
		}
	}

	Triangle::Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
	{
		vertex[0] = v1;
		vertex[1] = v2;
		vertex[2] = v3;
	}
	void Triangle::setVertex(int index, const Vertex& v) { vertex[index] = v; }
	void Triangle::setVertex(const Vertex v[3])
	{
		vertex[0] = v[0];
		vertex[1] = v[1];
		vertex[2] = v[2];
	}
	void Triangle::setVertexPos(int index, const myEigen::Vector4f& newVert) { vertex[index].vertex = newVert; }
	void Triangle::setVertexPos(const myEigen::Vector4f newVert[3])
	{
		vertex[0].vertex = newVert[0];
		vertex[1].vertex = newVert[1];
		vertex[2].vertex = newVert[2];
	}
	void Triangle::setNormal(int index, const myEigen::Vector4f& newNorm) { vertex[index].normal = newNorm; }
	void Triangle::setNormal(const myEigen::Vector4f newNorm[3])
	{ 
		vertex[0].normal = newNorm[0];
		vertex[0].normal = newNorm[1];
		vertex[0].normal = newNorm[2];
	}
	void Triangle::setColor(int index, const TGAColor& newColor) { vertex[index].vertexColor = newColor; }
	void Triangle::setColor(const TGAColor newColor[3]) 
	{
		vertex[0].vertexColor = newColor[0];
		vertex[1].vertexColor = newColor[1];
		vertex[2].vertexColor = newColor[2];
	}
}