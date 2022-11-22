#include "material.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace rst
{
	Material::Material(const std::string& filename, const std::string& materialName)
	{
		std::ifstream mtl;
		mtl.open(filename, std::ifstream::in);
		if (mtl.fail())
		{
			std::cerr << "Cannot open:" << filename << std::endl;
			return;
		}
	}
}
