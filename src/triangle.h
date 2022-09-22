#pragma once
#include "myEigen.hpp"
#include "tgaimage.h"

namespace rst {
	class Triangle {
	public:
		myEigen::Vector4f vertex[3];
		myEigen::Vector4f normal[3];
		TGAColor vertexColor[3];

		Triangle();
		Triangle(myEigen::Vector3f v[3]);

		void setVertex(int index, const myEigen::Vector4f& newVert);
		void setVertex(const myEigen::Vector4f newVert[3]);
		void setNormal(int index, const myEigen::Vector4f& newNorm);
		void setNormal(const myEigen::Vector4f newNorm[3]);
		void setColor(int index, const TGAColor& newColor);
		void setColor(const TGAColor newColor[3]);
	};
}