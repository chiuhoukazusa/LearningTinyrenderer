#pragma once
#include <string>
#include "myEigen.hpp"
#include "tgaimage.h"
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
		float instensity;
	public:
		PointLight(const myEigen::Vector3f& position, const float instensity)
			:position(position), instensity(instensity){}
		std::string getType() { return std::string("PointLight"); }
	};

	class DirectionalLight
	{
	public:
		myEigen::Vector3f direction;
		float instensity;
	public:
		DirectionalLight(const myEigen::Vector3f& direction, const float instensity)
			:direction(direction), instensity(instensity) {}
		std::string getType() { return std::string("DirectionalLight"); }
	};

	class fragmentShaderPayload
	{
	public:
		fragmentShaderPayload() = default;
		fragmentShaderPayload(const Vertex& vertex, const PointLight& light)
			:vertex(vertex), light(light) {}
		Vertex vertex;
		PointLight light;
	};

	static Vertex BlinnPhongShader(const fragmentShaderPayload& payload)
	{
		myEigen::Vector3f ka(0.005, 0.005, 0.005);
		myEigen::Vector3f kd(payload.vertex.vertexColor.bgra[2], payload.vertex.vertexColor.bgra[1], payload.vertex.vertexColor.bgra[0]);
		myEigen::Vector3f ks(0.7937, 0.7937, 0.7937);

		//ambiant
		auto ambient = ka * payload.light.instensity;

		//diffuse
		auto pos = myEigen::Vector3f(payload.vertex.vertex.x, payload.vertex.vertex.y, payload.vertex.vertex.z);
		auto normal = myEigen::Vector3f(payload.vertex.normal.x, payload.vertex.normal.y, payload.vertex.normal.z).Normalize();
		auto lightdir = (pos - payload.light.position).Normalize();
		auto r_2 = (pos - payload.light.position).Norm();
		auto diffuse = kd * std::max(0.0f, myEigen::dotProduct(normal, lightdir)) * payload.light.instensity
			/ r_2;

		auto color = ambient + diffuse;

		Vertex v = payload.vertex;
		v.vertexColor = TGAColor(color.x, color.y, color.z);
		return v;
	}
}