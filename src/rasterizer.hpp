#pragma once
#include "tgaimage.h"
#include "myEigen.hpp"
#include "transform.h"
#include "triangle.h"
#include "model.h"

namespace rst {

	class rasterizer {
		public:
			rasterizer(std::string filename, TGAImage &image);
			void draw_line(myEigen::Vector2i, myEigen::Vector2i);
			void output();

			void SetCamera(myEigen::Vector3f eye_pos);
			void SetGaze(myEigen::Vector3f gaze_dir);
			void SetViewUp(myEigen::Vector3f view_up);
			void SetTheta(float theta);
			void SetRotateAxis(myEigen::Vector3f rotateAxis);

			void draw();
			void rasterize_wireframe(const model& m);

			model m;
		private:
			const TGAColor white = TGAColor(255, 255, 255, 255);//°×É«
			const TGAColor red = TGAColor(255, 0, 0, 255);//ºìÉ«
			std::string filename;
			int width, height;
			TGAImage image;

			myEigen::Vector3f eye_pos;
			myEigen::Vector3f gaze_dir;
			myEigen::Vector3f view_up;
			myEigen::Vector3f rotateAxis;
			float zneardis;
			float zfardis;
			float fovY;
			float aspect;
			float theta;
	};
}