#pragma once
#include "tgaimage.h"
#include "myEigen.hpp"

namespace rst {

	class rasterizer {
		public:
			rasterizer(std::string filename, TGAImage &image);
			void draw_line(myEigen::Vector2i, myEigen::Vector2i);
			void output();

		private:
			const TGAColor white = TGAColor(255, 255, 255, 255);//°×É«
			const TGAColor red = TGAColor(255, 0, 0, 255);//ºìÉ«
			std::string filename;
			int width, height;
			TGAImage image;
	};
}