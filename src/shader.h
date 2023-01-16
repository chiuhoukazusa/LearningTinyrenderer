#pragma once
#pragma optimize ("", off)
#include <string>
#include "myEigen.hpp"
#include "tgaimage.h"
#include "material.h"
#include "geometry.h"

namespace rst 
{
	class Light
	{
	public:
		virtual ~Light() = default;
		virtual std::string getType() = 0;
	};

	class PointLight : public Light
	{
	public:
		myEigen::Vector3f position;
		float intensity;
	public:
		PointLight(const myEigen::Vector3f& position, const float intensity)
			:position(position), intensity(intensity){}
		std::string getType() { return std::string("PointLight"); }
	};

	class DirectionalLight : public Light
	{
	public:
		myEigen::Vector3f direction;
		float intensity;
	public:
		DirectionalLight(const myEigen::Vector3f& direction, const float intensity)
			:direction(direction), intensity(intensity) {}
		std::string getType() { return std::string("DirectionalLight"); }
	};

	inline TGAColor bilinearInterpolate(const TGAImage& texture, float u, float v)
	{
		//return texture.get(u, v);
		while (u > texture.width())
		{
			u -= texture.width();
		}
		while (v > texture.height())
		{
			v -= texture.height();
		}
		while (u < 0)
		{
			u += texture.width();
		}
		while (v < 0)
		{
			v += texture.height();
		}

		float uu = u - 0.5f;
		float vv = v - 0.5f;

		//if (uu < 0) { __debugbreak(); }
		//if (vv < 0) { __debugbreak(); }

		auto c_u1 = myEigen::Vector2f(ceil(uu), floor(vv));
		auto f_u1 = myEigen::Vector2f(floor(uu), floor(vv));

		TGAColor u_interpolated1 = ColorLerp(texture.get(f_u1.x, f_u1.y), texture.get(c_u1.x, c_u1.y), uu - floor(uu));

		auto c_u2 = myEigen::Vector2f(ceil(uu), ceil(vv));
		auto f_u2 = myEigen::Vector2f(floor(uu), ceil(vv));

		TGAColor u_interpolated2 = ColorLerp(texture.get(f_u2.x, f_u2.y), texture.get(c_u2.x, c_u2.y), uu - floor(uu));

		TGAColor out = ColorLerp(u_interpolated2, u_interpolated1, vv - floor(vv));
		return out;
	}

	class IShader {
	public:
		virtual void setmtl(Material& material) = 0;
		virtual void setddx(const float ddx) = 0;
		virtual void setddy(const float ddy) = 0;
		virtual void VertexShader(Triangle& primitive) = 0;
		virtual TGAColor FragmentShader(Vertex& vertex) = 0;
	};
}
#pragma optimize ("", on)