#pragma once
#include "../../shader.h"
#include "../../transform.h"
#include "../../config.h"
#include "../light/light.h"

namespace rst
{
    class BlinnPhongShader : public IShader
    {
    public:
        float theta_uniform;
        std::vector<PointLight> lights;

    public:
        BlinnPhongShader() = default;
        ~BlinnPhongShader() = default;

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

            modeling = Modeling(myEigen::Vector3f(0),
                myEigen::Vector3f(1),
                myEigen::Vector3f(rotateAxis), theta_uniform);
            viewing = Camera(eye_pos, gaze_dir, view_up);
            projection = Perspective(zneardis, zfardis, fovY, aspect);

            Transform mvn = modeling * viewing;
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
                myEigen::Vector3f uv = sample(vertex.texcoord, material->map_Bump);


                myEigen::Vector3f uv_vec((double)uv.x * 2.0f - 1.0f,
                    (double)uv.y * 2.0f - 1.0f,
                    (double)uv.z * 2.0f - 1.0f);

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
            //Kd = myEigen::Vector3f(0.5);
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
};
