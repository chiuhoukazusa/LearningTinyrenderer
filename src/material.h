#pragma once
#include <string>
#include "myEigen.hpp"
#include "tgaimage.h"

namespace rst
{
	class Material
	{
	public:
		Material() = default;
		Material(const std::string& filename, const std::string& root, const std::string& materialName);
	public:
		std::string name;

		myEigen::Vector3f Ke;
		myEigen::Vector3f Kd;
		myEigen::Vector3f Ka;
		myEigen::Vector3f Ks;
		float illum;
		float Ns;
		float d;
		float Ni;

		std::vector<TGAImage> map_Ke;
		std::vector<TGAImage> map_Kd;
		std::vector<TGAImage> map_Ka;
		std::vector<TGAImage> map_Ks;
		std::vector<TGAImage> map_Ns;
		std::vector<TGAImage> map_d;

		myEigen::Vector3f Bump;
		std::vector<TGAImage> map_Bump;
	};

	//int calculate_mipmaplevels(int mipmap_width, int mipmap_height);
	std::vector<TGAImage> generateMipmap(const TGAImage& texture);
}
