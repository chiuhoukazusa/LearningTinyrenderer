#pragma once
#include "../../transform.h"

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
			:position(position), intensity(intensity) {}
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

}