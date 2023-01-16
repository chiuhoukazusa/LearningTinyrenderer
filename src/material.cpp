#include "material.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace rst
{
	Material::Material(const std::string& filename, const std::string& root, const std::string& materialName)
	{
		name = materialName;

		std::ifstream mtl;
		mtl.open(root + filename, std::ifstream::in);
		if (mtl.fail())
		{
			std::cerr << "Cannot open:" << filename << std::endl;
			return;
		}
		std::string mtlLine;
		std::vector<std::string> mtlDescribe;
		bool isWrite = false;
		while (!mtl.eof())
		{
			std::getline(mtl, mtlLine);

			if (mtlLine.compare(0, 7, "newmtl ") == 0)
				if (mtlLine.compare(7, materialName.size(), materialName) == 0)
					isWrite = true;

			if(isWrite)
			{
				if (mtlLine.size() == 0) break;
				mtlDescribe.push_back(mtlLine);
			}
		}
		for (auto& s : mtlDescribe)
		{
			std::istringstream iss(s);
			std::string header;
			char trash;
			if (s.compare(0, 3, "Ns ") == 0)
			{
				iss >> header;
				iss >> Ns;
			}
			else if (s.compare(0, 3, "Ke ") == 0)
			{
				iss >> header;
				iss >> Ke.x >> Ke.y >> Ke.z;
			}
			else if (s.compare(0, 3, "Ka ") == 0)
			{
				iss >> header;
				iss >> Ka.x >> Ka.y >> Ka.z;
			}
			else if (s.compare(0, 3, "Kd ") == 0)
			{
				iss >> header;
				iss >> Kd.x >> Kd.y >> Kd.z;
			}
			else if (s.compare(0, 3, "Ks ") == 0)
			{
				iss >> header;
				iss >> Ks.x >> Ks.y >> Ks.z;
			}
			else if (s.compare(0, 2, "d ") == 0)
			{
				iss >> header;
				iss >> d;
			}
			else if (s.compare(0, 3, "Ni ") == 0)
			{
				iss >> header;
				iss >> Ni;
			}
			else if (s.compare(0, 6, "illum ") == 0)
			{
				iss >> header;
				iss >> illum;
			}
			else if (s.compare(0, 7, "map_Ke ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				TGAImage map = TGAImage();
				map.read_tga_file(root + textureName);
				map.flip_vertically();
				map_Ke = generateMipmap(map);
				//map_Ke.write_tga_file(root + "/read_result/" + textureName + ".tga");
			}
			else if (s.compare(0, 7, "map_Ka ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				TGAImage map = TGAImage();
				map.read_tga_file(root + textureName);
				map.flip_vertically();
				map_Ka = generateMipmap(map);
				//map_Ka.write_tga_file(root + "/read_result/" + textureName + ".tga");
			}
			else if (s.compare(0, 7, "map_Kd ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				TGAImage map = TGAImage();
				map.read_tga_file(root + textureName);
				map.flip_vertically();
				map_Kd = generateMipmap(map);
				//map_Kd.write_tga_file(root + "/read_result/" + textureName + ".tga");
			}
			else if (s.compare(0, 7, "map_Ks ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				TGAImage map = TGAImage();
				map.read_tga_file(root + textureName);
				map.flip_vertically();
				map_Ks = generateMipmap(map);
				//map_Ks.write_tga_file(root + "/read_result/" + textureName + ".tga");
			}
			else if (s.compare(0, 7, "map_Ns ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				TGAImage map = TGAImage();
				map.read_tga_file(root + textureName);
				map.flip_vertically();
				map_Ns = generateMipmap(map);
				//map_Ns.write_tga_file(root + "/read_result/" + textureName + ".tga");
			}
			else if (s.compare(0, 6, "map_d ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				TGAImage map = TGAImage();
				map.read_tga_file(root + textureName);
				map.flip_vertically();
				map_d = generateMipmap(map);
				//map_d.write_tga_file(root + "/read_result/" + textureName + ".tga");
			}
			else if (s.compare(0, 9, "map_Bump ") == 0)
			{
				std::string textureName;
				iss >> header >> header >> header;
				iss >> textureName;
				TGAImage map = TGAImage();
				map.read_tga_file(root + textureName);
				map.flip_vertically();
				map_Bump = generateMipmap(map);
				//map_Bump.write_tga_file(root + "/read_result/" + textureName + ".tga");
			}
		}

	}

	int calculate_mipmaplevels(int mipmap_width, int mipmap_height)
	{
		int mipmap_levels = 1;
		int size = std::min(mipmap_width, mipmap_height);
		while (size > 1)
		{
			mipmap_levels++;
			size = std::max(1, size / 2);
		}
		return mipmap_levels;
	}

	std::vector<TGAImage> generateMipmap(const TGAImage& texture)
	{
		std::vector<TGAImage> mipmap;

		int mipmap_width = texture.width();
		int mipmap_height = texture.height();

		int mipmap_levels = calculate_mipmaplevels(mipmap_width, mipmap_height);

		for (int level = 0; level < mipmap_levels; level++)
		{
			TGAImage mipmap_level_data(mipmap_width, mipmap_height, TGAImage::RGB);
			for (int y = 0; y < mipmap_height; y++)
			{
				for (int x = 0; x < mipmap_width; x++)
				{
					int image_x = x * texture.width() / mipmap_width;
					int image_y = y * texture.height() / mipmap_height;
					TGAColor pixel = texture.get(image_x, image_y);

					mipmap_level_data.set(x, y, pixel);
				}
			}

			mipmap.push_back(mipmap_level_data);

			mipmap_width = std::max(1, mipmap_width / 2);
			mipmap_height = std::max(1, mipmap_height / 2);
		}
		return mipmap;
	}
}
