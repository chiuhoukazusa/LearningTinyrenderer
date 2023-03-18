#pragma once
//#pragma optimize ("", off)
#include <string>
#include "myEigen.hpp"
#include "tgaimage.h"
#include "material.h"
#include "geometry.h"
#include "transform.h"

namespace rst 
{
	inline TGAColor bilinearInterpolate(const TGAImage& texture, float u, float v)
	{
		//return texture.get(u, v);
		while (u > texture.width())
		{
			u -= texture.width();
		}
		while (v > texture.height())
		{
			v -= texture.height();
		}
		while (u < 0)
		{
			u += texture.width();
		}
		while (v < 0)
		{
			v += texture.height();
		}

		float uu = u - 0.5f;
		float vv = v - 0.5f;

		//if (uu < 0) { __debugbreak(); }
		//if (vv < 0) { __debugbreak(); }

		auto c_u1 = myEigen::Vector2f(ceil(uu), floor(vv));
		auto f_u1 = myEigen::Vector2f(floor(uu), floor(vv));

		TGAColor u_interpolated1 = ColorLerp(texture.get(f_u1.x, f_u1.y), texture.get(c_u1.x, c_u1.y), uu - floor(uu));

		auto c_u2 = myEigen::Vector2f(ceil(uu), ceil(vv));
		auto f_u2 = myEigen::Vector2f(floor(uu), ceil(vv));

		TGAColor u_interpolated2 = ColorLerp(texture.get(f_u2.x, f_u2.y), texture.get(c_u2.x, c_u2.y), uu - floor(uu));

		TGAColor out = ColorLerp(u_interpolated2, u_interpolated1, vv - floor(vv));
		return out;
	}

	class IShader {
	public:
		virtual void setmtl(Material& material) = 0;
		virtual void setddx(const float ddx) = 0;
		virtual void setddy(const float ddy) = 0;
		virtual void VertexShader(Triangle& primitive) = 0;
		virtual TGAColor FragmentShader(Vertex& vertex) = 0;

        Material* material;
        Vertex vertex;
        float ddx = 1;
        float ddy = 1;
        Triangle tri_cameraspace;

        Transform modeling;
        Transform viewing;
        Transform projection;

        myEigen::Vector3f sample(const myEigen::Vector2f texcoord, const std::vector<TGAImage>& texture)
        {
            if (!texture.empty())
            {
                float mipmap_level = 0;
                if (ddx != -1.0f || ddy != -1.0f)
                {
                    float max_level = texture.size() - 1;
                    float d = std::max(ddx * texture[0].width(), ddy * texture[0].height());
                    mipmap_level = std::log2(d);
                    if (mipmap_level < 0)
                    {
                        mipmap_level = 0;
                        TGAColor var = bilinearInterpolate(texture[mipmap_level],
                            texcoord.x * texture[mipmap_level].width(),
                            texcoord.y * texture[mipmap_level].height());
                        return myEigen::Vector3f(var.bgra[2] / 255.0, var.bgra[1] / 255.0, var.bgra[0] / 255.0);
                    }
                    if (mipmap_level > max_level)
                    {
                        mipmap_level = max_level;
                        TGAColor var = bilinearInterpolate(texture[mipmap_level],
                            texcoord.x * texture[mipmap_level].width(),
                            texcoord.y * texture[mipmap_level].height());
                        return myEigen::Vector3f(var.bgra[2] / 255.0, var.bgra[1] / 255.0, var.bgra[0] / 255.0);
                    }
                }

                int f_mipmap_level = floor(mipmap_level);
                int c_mipmap_level = ceil(mipmap_level);

                if (f_mipmap_level < 0) __debugbreak();

                TGAColor f_var = bilinearInterpolate(texture[f_mipmap_level],
                    texcoord.x * texture[f_mipmap_level].width(),
                    texcoord.y * texture[f_mipmap_level].height());
                TGAColor c_var = bilinearInterpolate(texture[c_mipmap_level],
                    texcoord.x * texture[c_mipmap_level].width(),
                    texcoord.y * texture[c_mipmap_level].height());
                TGAColor var = ColorLerp(f_var, c_var, mipmap_level - f_mipmap_level);
                return myEigen::Vector3f(var.bgra[2] / 255.0, var.bgra[1] / 255.0, var.bgra[0] / 255.0);
            }
            else { return myEigen::Vector3f(-200, -200, -200); }
        }

        void calculate_Texture(const Material* material, const myEigen::Vector2f texcoord,
            myEigen::Vector3f& Ka, myEigen::Vector3f& Kd,
            myEigen::Vector3f& Ks, myEigen::Vector3f& Ke)
        {
            //Ke
            Ke = sample(texcoord, material->map_Ke);
            if (Ke.x == -200 && Ke.y == -200 && Ke.z == -200)
                Ke = material->Ke;

            //Kd
            Kd = sample(texcoord, material->map_Kd);
            if (Kd.x == -200 && Kd.y == -200 && Kd.z == -200)
                Kd = material->Kd;

            //Ka
            Ka = sample(texcoord, material->map_Ka);
            if (Ka.x == -200 && Ka.y == -200 && Ka.z == -200)
                Ka = material->Ke;

            //Ks
            Ks = sample(texcoord, material->map_Ks);
            if (Ks.x == -200 && Ks.y == -200 && Ks.z == -200)
                Ks = material->Ke;

        }
	};
}
//#pragma optimize ("", on)