#pragma once
#include "tgaimage.h"
#include "myEigen.hpp"
#include "geometry.h"
#include "material.h"
#include <vector>
#include <string>

namespace rst
{
	class Mesh
	{
	public:
		Mesh(const std::string& filename, const std::string& materialname) :material(filename, materialname) {}
	public:
		std::vector<Triangle> primitives;
		Material material;
	};

	class Object
	{
	public:
		Object() {};
	private:

	public:
		Material material();
		std::vector<Mesh> meshes{};
	};

	class Model
	{
	public:
		Model(const std::string& root, const std::string& filename);
	private:
		std::vector<myEigen::Vector3f> verts{};
		std::vector<myEigen::Vector3f> normals{};
		std::vector<myEigen::Vector2f> texcoords{};
	public:
		std::vector<Object> objects{};
	};
}
