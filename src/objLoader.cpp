#include "objLoader.h"
#include <fstream>
#include <sstream>

Model::Model(std::string filename)
{
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail())
	{
		std::cerr << "Cannot open:" << filename << std::endl;
		return;
	}
	std::string line;
	while (!in.eof())
	{
		std::getline(in, line);
		std::istringstream s(line);
		std::string header;
		char trash;
		if (line.compare(0, 2, "v ") == 0)
		{
			float x, y, z;
			s >> header;
			s >> x >> y >> z;
			verts.push_back(myEigen::Vector3f(x, y, z));
		}
		else if (line.compare(0, 3, "vt ") == 0)
		{
			float x, y;
			s >> header;
			s >> x >> y;
			texcoords.push_back(myEigen::Vector2f(x, y));
		}
		else if (line.compare(0, 3, "vn ") == 0)
		{
			float x, y, z;
			s >> header;
			s >> x >> y >> z;
			normals.push_back(myEigen::Vector3f(x, y, z));
		}
		else if(line.compare(0, 2, "f ") == 0)
		{
			int v[3], vt[3], vn[3];
			s >> header;
			for (size_t i = 0; i < 3; i++)
			{
				s >> v[i] >> trash >> vt[i] >> trash >> vn[i];
			}
			myEigen::Vector3f vertex[3]{ verts[v[0] - 1], verts[v[1] - 1], verts[v[2] - 1] };
			meshes.push_back(rst::Triangle(vertex));
		}
	}
}