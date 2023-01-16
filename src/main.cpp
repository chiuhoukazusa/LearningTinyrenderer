#include "tgaimage.h"
#include "rasterizer.hpp"
#include "myEigen.hpp"
#include "geometry.h"
#include "objLoader.h"
#include "shader.h"
#include "transform.h"
#include <string>
#include <chrono>
#include <random>

using std::cout;
using std::cin;
using std::endl;

//auto eye_pos   = myEigen::Vector3f(0, 0, 2.5);
auto eye_pos = myEigen::Vector3f(0, 0.8, 2.5);
auto gaze_dir  = myEigen::Vector3f(0, 0, -1);
auto view_up   = myEigen::Vector3f(0, 1, 0);
float theta    = 0;
int width      = 700;
int height     = 700;

float zneardis = 0.1f;
float zfardis  = 50;
float fovY     = 45;
float aspect   = 1;

rst::PointLight light = rst::PointLight(myEigen::Vector3f(20, 20, 20), 1400.0);

//loadShader
namespace rst
{
    class Shader : public IShader
    {
    public:
        float theta_uniform;
        std::vector<PointLight> lights;

    private:
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
				int mipmap_level = 0;
				if (ddx != -1.0f || ddy != -1.0f)
				{
					int i = 0;
					for (auto& map : texture)
					{
						float d = std::max(ddx * map.width(), ddy * map.height());
						if (std::max((int)std::log2(d), 0) == 0)
						{
							mipmap_level = i;
							break;
						}
						i++;
					}
				}

                //if (mipmap_level == 10) __debugbreak();

				TGAColor var = bilinearInterpolate(texture[mipmap_level],
					texcoord.x * texture[mipmap_level].width(),
					texcoord.y * texture[mipmap_level].height());
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

    public:
        Shader() = default;

        virtual void setmtl(Material& _material) override
        {
            material = &_material;
        }

        virtual void setddx(const float _ddx) override
        {
            ddx = _ddx;
        }

        virtual void setddy(const float _ddy) override
        {
            ddy = _ddy;
        }

        virtual void VertexShader(Triangle& primitive) override
        {
            auto rotateAxis = myEigen::Vector3f(0, 1, 0);

            modeling        = Modeling(myEigen::Vector3f(0),
                                       myEigen::Vector3f(1),
                                       myEigen::Vector3f(rotateAxis), theta_uniform);
            viewing         = Camera(eye_pos, gaze_dir, view_up);
            projection      = Perspective(zneardis, zfardis, fovY, aspect);

            Transform mvn   = modeling * viewing;
            mvn.toNormal();

            //model space -> world space -> camera space
            for (auto& v : primitive.vertex)
            {
                v.worldPos = modeling(v.vertex);
                v.vertex = viewing(modeling(v.vertex));
                v.normal = mvn(v.normal);
            }

            tri_cameraspace = primitive;

            //camera space -> clip space
            for (auto& v : primitive.vertex)
            {
                v.vertex = projection(v.vertex);
            }

        }

        virtual TGAColor FragmentShader(Vertex& vertex) override
        {
            myEigen::Vector3f Ka;
            myEigen::Vector3f Kd;
            myEigen::Vector3f Ks;
            myEigen::Vector3f Ke;
            myEigen::Vector3f Normal;

            calculate_Texture(material, vertex.texcoord, Ka, Kd, Ks, Ke);

            Normal = myEigen::Vector3f(vertex.normal.x, vertex.normal.y, vertex.normal.z).Normalize();
            //Bump
            if (!material->map_Bump.empty())
            {
                int mipmap_level = 0;
                if (ddx != -1.0f || ddy != -1.0f)
                {
                    int i = 0;
                    for (auto& map : material->map_Bump)
                    {
                        float d = std::max(ddx * map.width(), ddy * map.height());
                        if (std::max((int)std::log2(d), 0) == 0)
                        {
                            mipmap_level = i;
                            break;
                        }
                        i++;
                    }
                }

                float u = vertex.texcoord.x;
                float v = vertex.texcoord.y;
                float w = material->map_Bump[mipmap_level].width();
                float h = material->map_Bump[mipmap_level].height();
                TGAColor uv = bilinearInterpolate(material->map_Bump[mipmap_level],
                    u * w, v * h);

                myEigen::Vector3f uv_vec((double)uv.bgra[2] * 2.0f / 255.0 - 1.0f,
                                         (double)uv.bgra[1] * 2.0f / 255.0 - 1.0f,
                                         (double)uv.bgra[0] * 2.0f / 255.0 - 1.0f);

                myEigen::Vector3f			n = uv_vec.Normalize();

                myEigen::Vector3f        p1p0 = (tri_cameraspace.vertex[1].vertex - tri_cameraspace.vertex[0].vertex).xyz();
                myEigen::Vector3f        p2p0 = (tri_cameraspace.vertex[2].vertex - tri_cameraspace.vertex[0].vertex).xyz();
                myEigen::Vector3f	       fu = myEigen::Vector3f(tri_cameraspace.vertex[1].texcoord.x - tri_cameraspace.vertex[0].texcoord.x,
                                                                  tri_cameraspace.vertex[2].texcoord.x - tri_cameraspace.vertex[0].texcoord.x,
                                                                  0.0f);
                myEigen::Vector3f	       fv = myEigen::Vector3f(tri_cameraspace.vertex[1].texcoord.y - tri_cameraspace.vertex[0].texcoord.y,
                                                                  tri_cameraspace.vertex[2].texcoord.y - tri_cameraspace.vertex[0].texcoord.y,
                                                                  0.0f);
                myEigen::Matrixf3x3         A = myEigen::Matrix3x3Transpose(myEigen::Matrixf3x3(p1p0, p2p0, Normal));
                auto				A_inverse = myEigen::Matrix3x3Inverse(A);

                myEigen::Vector3f           T = A_inverse * fu;
                myEigen::Vector3f           B = A_inverse * fv;

                myEigen::Matrixf3x3 TBN(T.Normalize(), B.Normalize(), Normal);
                Normal = (TBN * n).Normalize();
                //Normal = n;
                //Normal = myEigen::Vector3f(vertex.normal.x, vertex.normal.y, vertex.normal.z).Normalize();
            }

            //Vertex a = payload.vertex;
            //a.vertexColor = TGAColor(payload.vertex.texcoord.x * 255.0f, payload.vertex.texcoord.y * 255.0f, 255.0f);
            //return a;
            Kd = myEigen::Vector3f(0.5);
            //Ks = Kd;
            float Ns = material->Ns;
            float Ie = 0;
            auto pos = vertex.worldPos.xyz();

            myEigen::Vector3f color(1, 1, 1);
            for (auto& light : lights)
            {
                //emission
                auto emission = Ke * Ie;

                //ambiant
                auto ambient = Ka * 0.1;

                //if (!payload.material.map_Bump.empty())
                    //__debugbreak();

                //diffuse
                auto lightdir = (light.position - pos).Normalize();
                auto r_2 = (light.position - pos).Norm();
                auto irradiance = myEigen::dotProduct(Normal, lightdir);
                auto diffuse = Kd * std::max(0.0f, irradiance) * light.intensity
                    / r_2;

                //specular
                auto viewdir = (-pos).Normalize();
                auto h = (viewdir + lightdir).Normalize();
                auto specular = Ks * light.intensity * std::pow(std::max(0.0f, myEigen::dotProduct(h, Normal)), Ns)
                    / r_2;

                if (material->illum == 0)
                {
                    color.x *= Kd.x;
                    color.y *= Kd.y;
                    color.z *= Kd.z;
                }
                else if (material->illum == 1)
                {
                    color.x *= emission.x + ambient.x + diffuse.x;
                    color.y *= emission.y + ambient.y + diffuse.y;
                    color.z *= emission.z + ambient.z + diffuse.z;
                }
                else if (material->illum == 2)
                {
                    color.x *= emission.x + ambient.x + diffuse.x + specular.x;
                    color.y *= emission.y + ambient.y + diffuse.y + specular.y;
                    color.z *= emission.z + ambient.z + diffuse.z + specular.z;
                }
            }
            color = color * 255.0f;

            color.x = color.x > 255.0f ? 255.0f : color.x;
            color.y = color.y > 255.0f ? 255.0f : color.y;
            color.z = color.z > 255.0f ? 255.0f : color.z;

            //if (color.x == 0 && color.y == 0 && color.z == 0)
               // __debugbreak();

            return TGAColor(color.x, color.y, color.z);
        }
    };
}

int main(int argc, char** argv)
{
    uint16_t frame = 0;
    auto begin = std::chrono::steady_clock::now();
    float angle = 0;

    std::vector<std::shared_ptr<rst::Mesh>> MeshList;
    
    //load model
    std::string root = "obj/Elden Ring - Melina_XPS/";
    rst::Model model(root, "Melina.obj");
    //std::string root = "obj/diablo3_pose/";
    //rst::Model model(root, "diablo3_pose.obj");
    //std::string root = "obj/mary/";
    //rst::Model model(root, "Marry.obj");

    for (auto& o:model.objects)
    {
        for (auto& m : o.meshes)
        {
            std::shared_ptr<rst::Mesh> m1 = std::make_shared<rst::Mesh>(m);
            MeshList.push_back(m1);
        }
    }

    rst::Shader shader;
    light.position = rst::Camera(eye_pos, gaze_dir, view_up)(light.position);
    shader.lights.push_back(light);

    while (true)
    {
        std::string filename = "result/output" + std::to_string(frame);
        TGAImage image(700, 700, TGAImage::RGB);
        
        rst::rasterizer rst(filename, image, light);
        //rst.SetCamera(myEigen::Vector3f(0, 1.8, 5));
        rst.SetCamera(eye_pos);
        rst.SetTheta(angle);

        shader.theta_uniform = angle;

        rst.SetRotateAxis(myEigen::Vector3f(0, 1, 0));
        //rst.TurnOnBackCulling();
        rst.draw(MeshList, shader);
        rst.output();
        cout << "frame:" << frame << endl;
        frame++;
        angle += 5;
        if (frame > 71) {
            auto end = std::chrono::steady_clock::now();
            cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
            return 0;
        }
    }
    auto end = std::chrono::steady_clock::now();
    cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
    return 0;
}