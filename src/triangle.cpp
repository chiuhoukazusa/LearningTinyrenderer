#include "triangle.h"
#include "myEigen.hpp"

namespace rst {
	Triangle::Triangle() {
		for (int i = 0; i < 3; i++)
		{
			vertex[i] = myEigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
			normal[i] = myEigen::Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
			vertexColor[i] = TGAColor();
		}
	}

	Triangle::Triangle(myEigen::Vector3f v[3])
	{
		for (size_t i = 0; i < 3; i++)
		{
			setVertex(i, myEigen::Vector4f(v[i].x, v[i].y, v[i].z, 1));
		}
	}

	void Triangle::setVertex(int index, const myEigen::Vector4f& newVert) { vertex[index] = newVert; }
	void Triangle::setVertex(const myEigen::Vector4f newVert[3])
	{
		vertex[0] = newVert[0];
		vertex[1] = newVert[1];
		vertex[2] = newVert[2];
	}
	void Triangle::setNormal(int index, const myEigen::Vector4f& newNorm) { normal[index] = newNorm; }
	void Triangle::setNormal(const myEigen::Vector4f newNorm[3])
	{ 
		normal[0] = newNorm[0];
		normal[1] = newNorm[1];
		normal[2] = newNorm[2];
	}
	void Triangle::setColor(int index, const TGAColor& newColor) { vertexColor[index] = newColor; }
	void Triangle::setColor(const TGAColor newColor[3]) 
	{
		vertexColor[0] = newColor[0];
		vertexColor[1] = newColor[1];
		vertexColor[2] = newColor[2];
	}
}