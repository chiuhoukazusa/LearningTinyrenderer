#pragma once
#include <string>

namespace rst
{
	class Material
	{
	public:
		Material() {};
		Material(const std::string& filename, const std::string& materialName);
	};
}
