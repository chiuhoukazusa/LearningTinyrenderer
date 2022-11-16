#pragma once
#include "tgaimage.h"
#include "myEigen.hpp"
#include "geometry.h"
#include <vector>
#include <string>

class Model {
public:
	Model(const std::string filename);
private:
	std::vector<myEigen::Vector3f> verts{};
	std::vector<myEigen::Vector3f> normals{};
	std::vector<myEigen::Vector2f> texcoords{};

public:
	std::vector<rst::Triangle> meshes{};
};