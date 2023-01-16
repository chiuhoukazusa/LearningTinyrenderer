#pragma once
#include "myEigen.hpp"
#include "tgaimage.h"

namespace rst {
	struct Vertex
	{
		myEigen::Vector4f vertex;
		TGAColor vertexColor;
		myEigen::Vector4f normal;
		myEigen::Vector2f texcoord;
		myEigen::Vector4f worldPos;
		Vertex() :
			vertex(), vertexColor(), normal(), texcoord() {}
		Vertex(const myEigen::Vector4f& vertex,
			const TGAColor& vertexColor,
			const myEigen::Vector4f& normal,
			const myEigen::Vector2f& texcoord) :
			vertex(vertex), vertexColor(vertexColor), normal(normal), texcoord(texcoord){}

		Vertex(const myEigen::Vector3f& vertex,
			const TGAColor& vertexColor,
			const myEigen::Vector3f& normal,
			const myEigen::Vector2f& texcoord) :
			vertex(vertex.x, vertex.y, vertex.z, 1), vertexColor(vertexColor),
			normal(normal.x, normal.y, normal.z, 0), texcoord(texcoord){}
		Vertex(const myEigen::Vector4f& vertex,
			const TGAColor& vertexColor,
			const myEigen::Vector4f& normal,
			const myEigen::Vector2f& texcoord,
			const myEigen::Vector4f& worldPos) :
			vertex(vertex), vertexColor(vertexColor), normal(normal), texcoord(texcoord), worldPos(worldPos) {}

		Vertex(const myEigen::Vector3f& vertex,
			const TGAColor& vertexColor,
			const myEigen::Vector3f& normal,
			const myEigen::Vector2f& texcoord,
			const myEigen::Vector3f& worldPos) :
			vertex(vertex.x, vertex.y, vertex.z, 1), vertexColor(vertexColor),
			normal(normal.x, normal.y, normal.z, 0), texcoord(texcoord), 
			worldPos(worldPos.x, worldPos.y, worldPos.z, 1){}
		
		Vertex operator+(const Vertex& v) const {
			return Vertex(vertex + v.vertex, vertexColor + v.vertexColor,
						  normal + v.normal, texcoord + v.texcoord, worldPos + v.worldPos);
		}
		Vertex operator*(const float x) const {
			return Vertex(vertex * x, vertexColor * x, normal * x, texcoord * x, worldPos * x);
		}
		Vertex& operator=(const Vertex& v) {
			vertex = v.vertex;
			vertexColor = v.vertexColor;
			normal = v.normal;
			texcoord = v.texcoord;
			worldPos = v.worldPos;
			return *this;
		}
		bool empty()
		{
			if (vertex.x == 0 && vertex.y == 0 && vertex.z == 0 && vertex.w == 0 &&
				(int)vertexColor.bgra[0] == 0 && (int)vertexColor.bgra[1] == 0 && (int)vertexColor.bgra[2] == 0 && (int)vertexColor.bgra[3] == 0 &&
				normal.x == 0 && vertex.y == 0 && vertex.z == 0 && vertex.w == 0 &&
				texcoord.x == 0 && texcoord.y == 0)
			{
				return true;
			}
			return false;
		}
	};

	inline Vertex lerp(const Vertex& v1, const Vertex& v2, const float t) {
		return Vertex{
			lerp(v1.vertex, v2.vertex, t),
			ColorLerp(v1.vertexColor, v2.vertexColor, t),
			lerp(v1.normal, v2.normal, t),
			lerp(v1.texcoord, v2.texcoord, t),
			lerp(v1.worldPos, v2.worldPos, t)
		};
	}
	inline Vertex perspectiveLerp(const Vertex& v1, const Vertex& v2, const float t,
		const myEigen::Vector4f& v1c, const myEigen::Vector4f& v2c)
	{
		float correctLerp = t * v2c.w / ((1 - t) * v1c.w + t * v2c.w);
		return lerp(v1, v2, correctLerp);
	}

	inline myEigen::Vector4f perspectiveLerp(const myEigen::Vector4f& v1, const myEigen::Vector4f& v2, const float t,
		const myEigen::Vector4f& v1c, const myEigen::Vector4f& v2c)
	{
		float correctLerp = t * v2c.w / ((1 - t) * v1c.w + t * v2c.w);
		return lerp(v1, v2, correctLerp);
	}

	struct Line
	{
		Vertex v1;
		Vertex v2;

		Line() :isNull(true){}
		Line(const Vertex& v1, const Vertex& v2) :v1(v1), v2(v2) {}
		Line(const Vertex& v1, const Vertex& v2, const bool isNull) :v1(v1), v2(v2), isNull(isNull) {}
		bool empty() { return isNull; }

	private:
		bool isNull = false;
	};

	struct Triangle {
	public:
		Vertex vertex[3];

		Triangle();
		Triangle(myEigen::Vector3f v[3]);
		Triangle(Vertex v[3]);
		Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3);

		void setVertex(int index, const Vertex& v);
		void setVertex(const Vertex v[3]);
		void setVertexPos(int index, const myEigen::Vector4f& newVert);
		void setVertexPos(const myEigen::Vector4f newVert[3]);
		void setWorldPos(int index, const myEigen::Vector4f& newVert);
		void setWorldPos(const myEigen::Vector4f newVert[3]);
		void setNormal(int index, const myEigen::Vector4f& newNorm);
		void setNormal(const myEigen::Vector4f newNorm[3]);
		void setColor(int index, const TGAColor& newColor);
		void setColor(const TGAColor newColor[3]);
	};

	Vertex barycentricLerp(const Triangle& t, const myEigen::Vector2f& pixel);
	Vertex barycentricPerspectiveLerp(const Triangle& t, const myEigen::Vector2f& pixel,
		const std::array<myEigen::Vector4f, 3>& v);
}