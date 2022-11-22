#include "objLoader.h"
#include <fstream>
#include <sstream>

namespace rst
{
	Model::Model(const std::string& root, const std::string& filename)
	{
		std::ifstream obj;
		std::string mtlfilename;
		obj.open(root + filename, std::ifstream::in);
		if (obj.fail())
		{
			std::cerr << "Cannot open:" << filename << std::endl;
			return;
		}
		std::string objLine;
		while (!obj.eof())
		{
			std::getline(obj, objLine);
			std::istringstream s(objLine);
			std::string header;
			char trash;
			int cur_obj = objects.size() - 1;
			int cur_mesh;
			if(!objects.empty()) cur_mesh = objects[cur_obj].meshes.size() - 1;
			if (objLine.compare(0, 7, "mtllib ") == 0)
			{
				s >> header;
				s >> mtlfilename;
				mtlfilename = root + mtlfilename;
			}
			else if(objLine.compare(0, 2, "o ") == 0)
			{
				s >> header >> header;
				objects.push_back(Object());
			}
			else if (objLine.compare(0, 2, "v ") == 0)
			{
				if (objects.empty())
				{
					objects.push_back(Object());
				}
				float x, y, z;
				s >> header;
				s >> x >> y >> z;
				verts.push_back(myEigen::Vector3f(x, y, z));
			}
			else if (objLine.compare(0, 3, "vt ") == 0)
			{
				float x, y;
				s >> header;
				s >> x >> y;
				texcoords.push_back(myEigen::Vector2f(x, y));
			}
			else if (objLine.compare(0, 3, "vn ") == 0)
			{
				float x, y, z;
				s >> header;
				s >> x >> y >> z;
				normals.push_back(myEigen::Vector3f(x, y, z));
			}
			else if (objLine.compare(0, 7, "usemtl ") == 0)
			{
				std::string materialName;
				s >> header;
				s >> materialName;
				Mesh mesh(mtlfilename, materialName);
				objects[cur_obj].meshes.push_back(mesh);
			}
			else if (objLine.compare(0, 2, "f ") == 0)
			{
				int v[3], vt[3], vn[3];
				s >> header;
				for (size_t i = 0; i < 3; i++)
				{
					s >> v[i] >> trash >> vt[i] >> trash >> vn[i];
				}
				Vertex vertex[3]
				{ 
					Vertex(verts[v[0] - 1], TGAColor(255, 255, 255), normals[vn[0] - 1], texcoords[vt[0] - 1]),
					Vertex(verts[v[1] - 1], TGAColor(255, 255, 255), normals[vn[1] - 1], texcoords[vt[1] - 1]),
					Vertex(verts[v[2] - 1], TGAColor(255, 255, 255), normals[vn[2] - 1], texcoords[vt[2] - 1])
				};
				objects[cur_obj].meshes[cur_mesh].primitives.push_back(Triangle(vertex));
			}
		}
		obj.close();

	}
}