#pragma once
#include "tgaimage.h"
#include "myEigen.hpp"
#include "transform.h"
#include "geometry.h"
#include "model.h"
#include "shader.h"
#include <functional>

namespace rst {

	enum class TriangleVertexOrder
	{
		counterclockwise,
		clockwise
	};

	class rasterizer {
		public:
			rasterizer(const std::string& filename, const TGAImage &image, const PointLight& light);
			void draw_line(const myEigen::Vector2i&, const myEigen::Vector2i&);
			void output();

			void SetCamera(const myEigen::Vector3f& eye_pos);
			void SetGaze(const myEigen::Vector3f& gaze_dir);
			void SetViewUp(const myEigen::Vector3f& view_up);
			void SetTheta(float theta);
			void SetRotateAxis(const myEigen::Vector3f& rotateAxis);

			void TurnOnBackCulling();
			void TurnOffBackCulling();
			void SetVertexOrder(const TriangleVertexOrder& t);
			TriangleVertexOrder GetVertexOrder();
			void draw(std::vector<std::shared_ptr<rst::Triangle>>& TriangleList);
			void setFragmentShader(std::function<Vertex(fragmentShaderPayload)> fragmentShader);
		private:
			std::vector<Vertex> clip_Cohen_Sutherland(const Triangle& t, const std::array<myEigen::Vector4f, 3>& clipSpacePos);
			Line clip_line(const Line& line, const std::array<myEigen::Vector4f, 2>& clipSpacePos);
			bool insideTriangle(const Triangle& m, const float x, const float y);
			void rasterize_wireframe(const Triangle& m);
			void rasterize_edge_walking(const Triangle& m, const std::array<myEigen::Vector4f, 3>& clipSpacePos);
			void rasterize_edge_equation(const Triangle& m, const std::array<myEigen::Vector4f, 3>& clipSpacePos);

		private:
			const TGAColor white = TGAColor(255, 255, 255, 255);//°×É«
			const TGAColor red = TGAColor(255, 0, 0, 255);//ºìÉ«
			std::string filename;
			int width, height;
			TGAImage image;
			TriangleVertexOrder vertexOrder = TriangleVertexOrder::counterclockwise;
			bool backCulling = false;

			myEigen::Vector3f eye_pos;
			myEigen::Vector3f gaze_dir;
			myEigen::Vector3f view_up;
			myEigen::Vector3f rotateAxis;
			float zneardis;
			float zfardis;
			float fovY;
			float aspect;
			float theta;

			PointLight light;
			std::function<Vertex(fragmentShaderPayload)> fragmentShader;
			std::vector<float> z_buffer;
			int get_index(int x, int y);
	};
}