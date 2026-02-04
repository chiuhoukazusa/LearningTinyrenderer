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
#include <cassert>
#include <stdexcept>
#include <memory>

using std::cout;
using std::cin;
using std::endl;

// Rendering Constants
namespace RenderConfig
{
    constexpr float AMBIENT_STRENGTH = 0.1f;
    constexpr float EMISSION_INTENSITY = 0.0f;
    constexpr float COLOR_MAX = 255.0f;
    constexpr float COLOR_SCALE = 1.0f / 255.0f;
    constexpr float INVALID_TEXTURE_SENTINEL = -200.0f;
}

// Camera and scene settings
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

// Shader implementation
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

        // Helper function to check if sample result is valid
        inline bool isValidSample(const myEigen::Vector3f& sample) const
        {
            return !(sample.x == RenderConfig::INVALID_TEXTURE_SENTINEL && 
                     sample.y == RenderConfig::INVALID_TEXTURE_SENTINEL && 
                     sample.z == RenderConfig::INVALID_TEXTURE_SENTINEL);
        }

        // Helper function to convert TGAColor to Vector3f
        inline myEigen::Vector3f colorToVec3(const TGAColor& color) const
        {
            return myEigen::Vector3f(
                color.bgra[2] * RenderConfig::COLOR_SCALE, 
                color.bgra[1] * RenderConfig::COLOR_SCALE, 
                color.bgra[0] * RenderConfig::COLOR_SCALE
            );
        }

		myEigen::Vector3f sample(const myEigen::Vector2f& texcoord, const std::vector<TGAImage>& texture)
		{
			if (texture.empty())
            {
                return myEigen::Vector3f(RenderConfig::INVALID_TEXTURE_SENTINEL, 
                                       RenderConfig::INVALID_TEXTURE_SENTINEL, 
                                       RenderConfig::INVALID_TEXTURE_SENTINEL);
            }

            float mipmap_level = 0;
            if (ddx != -1.0f || ddy != -1.0f)
            {
                const float max_level = static_cast<float>(texture.size() - 1);
                const float d = std::max(ddx * texture[0].width(), ddy * texture[0].height());
                mipmap_level = std::log2(d);
                
                // Clamp mipmap level to valid range
                if (mipmap_level < 0.0f)
                {
                    mipmap_level = 0.0f;
                }
                else if (mipmap_level > max_level)
                {
                    mipmap_level = max_level;
                }
            }

            // Trilinear interpolation
            const int f_mipmap_level = static_cast<int>(floor(mipmap_level));
            const int c_mipmap_level = static_cast<int>(ceil(mipmap_level));
            
            // Sanity check (replaced __debugbreak with assert)
            assert(f_mipmap_level >= 0 && "Mipmap level should not be negative");
            assert(f_mipmap_level < texture.size() && "Mipmap level out of range");

            if (f_mipmap_level == c_mipmap_level)
            {
                // No need for trilinear interpolation
                TGAColor var = bilinearInterpolate(texture[f_mipmap_level],
                    texcoord.x * texture[f_mipmap_level].width(),
                    texcoord.y * texture[f_mipmap_level].height());
                return colorToVec3(var);
            }

            // Trilinear interpolation between two mipmap levels
            TGAColor f_var = bilinearInterpolate(texture[f_mipmap_level],
                texcoord.x * texture[f_mipmap_level].width(),
                texcoord.y * texture[f_mipmap_level].height());
            TGAColor c_var = bilinearInterpolate(texture[c_mipmap_level],
                texcoord.x * texture[c_mipmap_level].width(),
                texcoord.y * texture[c_mipmap_level].height());
            
            const float lerp_weight = mipmap_level - f_mipmap_level;
            TGAColor var = ColorLerp(f_var, c_var, lerp_weight);
            return colorToVec3(var);
		}

        void calculate_Texture(const Material* material, const myEigen::Vector2f& texcoord,
			myEigen::Vector3f& Ka, myEigen::Vector3f& Kd,
			myEigen::Vector3f& Ks, myEigen::Vector3f& Ke)
		{
			// Emission coefficient
			Ke = sample(texcoord, material->map_Ke);
			if (!isValidSample(Ke))
				Ke = material->Ke;

			// Diffuse coefficient
            Kd = sample(texcoord, material->map_Kd);
            if (!isValidSample(Kd))
                Kd = material->Kd;

			// Ambient coefficient
            Ka = sample(texcoord, material->map_Ka);
            if (!isValidSample(Ka))
                Ka = material->Ka;

			// Specular coefficient
            Ks = sample(texcoord, material->map_Ks);
            if (!isValidSample(Ks))
                Ks = material->Ks;
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
            const auto rotateAxis = myEigen::Vector3f(0, 1, 0);

            modeling        = Modeling(myEigen::Vector3f(0),
                                       myEigen::Vector3f(1),
                                       rotateAxis, theta_uniform);
            viewing         = Camera(eye_pos, gaze_dir, view_up);
            projection      = Perspective(zneardis, zfardis, fovY, aspect);

            Transform mvn   = modeling * viewing;
            mvn.toNormal();

            // Model space -> World space -> Camera space
            for (auto& v : primitive.vertex)
            {
                v.worldPos = modeling(v.vertex);
                v.vertex = viewing(modeling(v.vertex));
                v.normal = mvn(v.normal);
            }

            tri_cameraspace = primitive;

            // Camera space -> Clip space
            for (auto& v : primitive.vertex)
            {
                v.vertex = projection(v.vertex);
            }
        }

        virtual TGAColor FragmentShader(Vertex& vertex) override
        {
            myEigen::Vector3f Ka, Kd, Ks, Ke;
            myEigen::Vector3f Normal;

            calculate_Texture(material, vertex.texcoord, Ka, Kd, Ks, Ke);

            Normal = myEigen::Vector3f(vertex.normal.x, vertex.normal.y, vertex.normal.z).Normalize();
            
            // Normal mapping
            if (!material->map_Bump.empty())
            {
                const myEigen::Vector3f uv = sample(vertex.texcoord, material->map_Bump);
           
                // Convert from [0,1] to [-1,1]
                myEigen::Vector3f uv_vec(
                    uv.x * 2.0f - 1.0f,
                    uv.y * 2.0f - 1.0f,
                    uv.z * 2.0f - 1.0f
                );

                const myEigen::Vector3f n = uv_vec.Normalize();

                // Calculate TBN matrix
                const myEigen::Vector3f p1p0 = (tri_cameraspace.vertex[1].vertex - tri_cameraspace.vertex[0].vertex).xyz();
                const myEigen::Vector3f p2p0 = (tri_cameraspace.vertex[2].vertex - tri_cameraspace.vertex[0].vertex).xyz();
                const myEigen::Vector3f fu = myEigen::Vector3f(
                    tri_cameraspace.vertex[1].texcoord.x - tri_cameraspace.vertex[0].texcoord.x,
                    tri_cameraspace.vertex[2].texcoord.x - tri_cameraspace.vertex[0].texcoord.x,
                    0.0f
                );
                const myEigen::Vector3f fv = myEigen::Vector3f(
                    tri_cameraspace.vertex[1].texcoord.y - tri_cameraspace.vertex[0].texcoord.y,
                    tri_cameraspace.vertex[2].texcoord.y - tri_cameraspace.vertex[0].texcoord.y,
                    0.0f
                );
                
                const myEigen::Matrixf3x3 A = myEigen::Matrix3x3Transpose(myEigen::Matrixf3x3(p1p0, p2p0, Normal));
                const auto A_inverse = myEigen::Matrix3x3Inverse(A);

                const myEigen::Vector3f T = (A_inverse * fu).Normalize();
                const myEigen::Vector3f B = (A_inverse * fv).Normalize();

                const myEigen::Matrixf3x3 TBN(T, B, Normal);
                Normal = (TBN * n).Normalize();
            }

            const float Ns = material->Ns;
            const auto pos = vertex.worldPos.xyz();

            myEigen::Vector3f color(1, 1, 1);
            
            // Lighting calculation
            for (const auto& light : lights)
            {
                // Emission term
                const auto emission = Ke * RenderConfig::EMISSION_INTENSITY;

                // Ambient term
                const auto ambient = Ka * RenderConfig::AMBIENT_STRENGTH;

                // Diffuse term
                const auto lightdir = (light.position - pos).Normalize();
                const auto r_2 = (light.position - pos).Norm();
                const auto irradiance = myEigen::dotProduct(Normal, lightdir);
                const auto diffuse = Kd * std::max(0.0f, irradiance) * light.intensity / r_2;

                // Specular term (Blinn-Phong)
                const auto viewdir = (-pos).Normalize();
                const auto h = (viewdir + lightdir).Normalize();
                const auto specular = Ks * light.intensity * std::pow(std::max(0.0f, myEigen::dotProduct(h, Normal)), Ns) / r_2;

                // Combine based on illumination model
                switch (static_cast<int>(material->illum))
                {
                case 0:
                    // Color only
                    color.x *= Kd.x;
                    color.y *= Kd.y;
                    color.z *= Kd.z;
                    break;
                case 1:
                    // Ambient + Diffuse
                    color.x *= emission.x + ambient.x + diffuse.x;
                    color.y *= emission.y + ambient.y + diffuse.y;
                    color.z *= emission.z + ambient.z + diffuse.z;
                    break;
                case 2:
                    // Ambient + Diffuse + Specular (Blinn-Phong)
                    color.x *= emission.x + ambient.x + diffuse.x + specular.x;
                    color.y *= emission.y + ambient.y + diffuse.y + specular.y;
                    color.z *= emission.z + ambient.z + diffuse.z + specular.z;
                    break;
                default:
                    // Fallback to full Blinn-Phong
                    color.x *= emission.x + ambient.x + diffuse.x + specular.x;
                    color.y *= emission.y + ambient.y + diffuse.y + specular.y;
                    color.z *= emission.z + ambient.z + diffuse.z + specular.z;
                    break;
                }
            }
            
            // Scale to [0, 255] and clamp
            color = color * RenderConfig::COLOR_MAX;
            color.x = std::min(color.x, RenderConfig::COLOR_MAX);
            color.y = std::min(color.y, RenderConfig::COLOR_MAX);
            color.z = std::min(color.z, RenderConfig::COLOR_MAX);

            return TGAColor(static_cast<uint8_t>(color.x), 
                           static_cast<uint8_t>(color.y), 
                           static_cast<uint8_t>(color.z));
        }
    };
}

int main(int argc, char** argv)
{
    try
    {
        uint16_t frame = 0;
        auto begin = std::chrono::steady_clock::now();
        float angle = 0;

        std::vector<std::shared_ptr<rst::Mesh>> MeshList;
        
        // Load model
        std::string root = "obj/diablo3_pose/";
        rst::Model model(root, "diablo3_pose.obj");
        
        // Alternative models (commented out)
        // std::string root = "obj/diablo3_pose/";
        // rst::Model model(root, "diablo3_pose.obj");
        // std::string root = "obj/mary/";
        // rst::Model model(root, "Marry.obj");

        for (auto& o : model.objects)
        {
            for (auto& m : o.meshes)
            {
                MeshList.push_back(std::make_shared<rst::Mesh>(m));
            }
        }

        rst::Shader shader;
        light.position = rst::Camera(eye_pos, gaze_dir, view_up)(light.position);
        shader.lights.push_back(light);

        // Render loop
        while (true)
        {
            std::string filename = "result/output" + std::to_string(frame);
            TGAImage image(700, 700, TGAImage::RGB);
            
            rst::rasterizer rst(filename, image, light);
            rst.SetCamera(eye_pos);
            rst.SetTheta(angle);
            shader.theta_uniform = angle;
            rst.SetRotateAxis(myEigen::Vector3f(0, 1, 0));
            
            rst.draw(MeshList, shader);
            rst.output();
            
            cout << "frame:" << frame << endl;
            frame++;
            angle += 5;
            
            if (frame > 71)
            {
                auto end = std::chrono::steady_clock::now();
                cout << "Total time: " 
                     << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() 
                     << " seconds" << endl;
                return 0;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown error occurred" << endl;
        return 1;
    }
    
    return 0;
}
