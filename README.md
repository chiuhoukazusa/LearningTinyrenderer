# 05.载入3D模型|模型加载、简单着色与纹理映射

## 本项目代码已托管至github，将会随着博客实时更新进度

每一节的工程我都会创建一个新的分支，分支名由这一节的数字决定。

[https://github.com/chiuhoukazusa/LearningTinyrenderer/tree/05](https://github.com/chiuhoukazusa/LearningTinyrenderer/tree/05)

## 前言

在上一节的工作完成后，我们离渲染出一个正常的三维模型还剩下两步，纹理映射和着色，当然前提是得先把模型给加载了。

这一节先来尝试纹理映射和简单着色模型。

上一节我们说过要引入一个读取obj的头文件，但我觉得完全可以自己写一个简单的objloader，那就不引入了吧。

## OBJ-MTL文件格式

obj是最广泛使用的3维模型的格式，实际上obj只是一个纯文本文件，我们可以梳理出他的大致结构。而mtl也是一个纯文本文件，基本上和obj对应，会记录一些材质信息。

我们以games202的吉祥物为例来理解一下obj和mtl文件的结构：

```obj
# Blender v2.90.1 OBJ File: 'Marry.blend'
# www.blender.org
mtllib Marry.mtl
o 文本
v 0.141234 1.731161 0.414226
v 0.089634 1.739039 0.419244
v 0.094429 1.750718 0.414622
v 0.098634 1.761540 0.410368
......
vt 0.295455 0.000000
vt 0.272727 0.000000
vt 0.284091 0.000000
vt 0.306818 0.000000
......
vn 0.0938 0.6694 0.7369
vn 0.0938 0.6695 0.7369
......
usemtl 材质
s off
f 27/1/1 25/2/1 26/3/1
f 28/4/2 25/2/2 27/1/2
f 29/5/3 25/2/3 28/4/3
f 29/5/4 24/6/4 25/2/4
......
o Body_MC003_Kozakura_Mari_face
v 0.068341 3.046584 0.159396
v 0.052779 3.045725 0.167629
v 0.067417 3.050314 0.165910
......
vt 0.042575 0.471054
vt 0.035748 0.456984
......
vn 0.2510 -0.7130 0.6547
vn 0.3849 -0.6684 0.6365
......
usemtl MC003_Kozakura_Mari
s 1
f 277/275/220 276/276/221 275/277/222
f 278/278/223 275/277/222 276/276/221
......
```

```MTL
# Blender MTL File: 'Marry.blend'
# Material Count: 2

newmtl MC003_Kozakura_Mari
Ns 900.000000
Ka 1.000000 1.000000 1.000000
Kd 0.800000 0.800000 0.800000
Ks 0.000000 0.000000 0.000000
Ke 0.000000 0.000000 0.000000
Ni 1.450000
d 1.000000
illum 1
map_Kd MC003_Kozakura_Mari.png

newmtl 材质
Ns 900.000000
Ka 1.000000 1.000000 1.000000
Kd 0.270588 0.552941 0.874510
Ks 0.000000 0.000000 0.000000
Ke 0.000000 0.000000 0.000000
Ni 1.450000
d 1.000000
illum 1
```

省略号省去了数据信息。前者是Marry.obj，后者是Marry.mtl。

我们可以看到除去前几行的注释信息外，最早载入眼帘的是"mtllib Marry.mtl"，mtllib这个关键词意指与这个obj配对的mtl文件，后面跟着的自然是文件的相对路径，当然也可以省略，这样的话就不使用材质。

然后是"o 文本"，这里的o意思是object，一个obj文件里不一定只有一个object，可以有多个object，相当于一个obj里存了多个模型，当然如果只有一个模型的话这行也可以省略。有些obj在o这个层级下还会有g这个层级，意思是group，分组。这个我们暂时先不考虑。

然后就是object下的一行行属性了。v开头的行储存了顶点信息，vt开头的行储存了纹理坐标信息，vn开头的行储存了法线信息。

f开头的行储存了每一个primitive的信息。f后面跟着三份用空格隔开的数据，指的是一个primitive由三个顶点组成，自然就是三角形图元。然后每份数据以v/vt/vn的顺序储存，当然不管是v还是vt或是vn都是以索引的形式储存，不过这个索引是以1开始的。

而且还要注意！这个索引是不会因为object的改变而重新从1开始计数的，可以看到上面的obj文件里，第一个object的索引都是从1开始的，而第二个object的索引就从200多开始了，也可以说，这个索引是全局计数的。

还有就是f上面也有几行信息，usemtl显然就是对应mtl文件里定义的newmtl了，而s指的是光滑组，"s 1"代表软边，"s off"代表硬边。软边和硬边就决定了我们怎么使用法线信息。如果是s off，那么我们定义的法线就是面法线，也就是三角形三个顶点共享一个法线，如果是开启光滑组，那么三角形三个顶点都会有各自的法线，而三角形内部任意一点都是由三个顶点的法线插值出来。不过我们暂时可以不用考虑光滑组的问题，因为我们可以看到，在202吉祥物的obj文件中的两个object，没开光滑组的那个object的f行里每一行的法线顶点索引都是一样的，也就是说f行里已经帮我们解决了光滑组的问题。

关于MTL文件部分可以参考这个链接里的内容。

http://www.paulbourke.net/dataformats/mtl/

mtl文件详细我们将在后续说到纹理映射的时候再提。

## 读取OBJ文件

我们先不管mtl文件，先按之前说的obj文件格式的标准来写一个简单读取obj文件的objloader试试。

我们不考虑组，也就是g这个关键词，那就是一个obj文件中可能存在多个o(object)，每个object又不一定使用一个material，也就是一个o下可能有多个usemtl，我们按这个逻辑可以梳理出一个大体结构：

```c++
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
```

Model->Object->Mesh

其中Mesh跟material绑定，object用于存储Mesh，Model用于存储Object。

我们目前只支持三角形图元，obj文件其实远不仅仅我们之前说的那些形式，他完全可以支持其他形状的图元，但是我们这里就不考虑了。

material类我们之后讨论mtl读取的时候再作讨论。

我们在model的构造函数里写下具体读取操作：

```c++
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
```

目前我们没有写所谓的纹理映射，但是我们还有一个办法可以检验我们的成果，那就是draw_wireframe()，使用之前写的线框渲染函数来看看究竟能不能读取我们的obj文件。

我们先拿202酱试试刀：

![202girl_wireframe](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450657.gif)

非常成功！debug模式下渲染72帧用时13秒，当然我微调了下相机位置，不然202酱起码一半都在屏幕外面。

202酱的obj文件有3万行左右。 

我们再来试试另一个有30万行obj文件的神秘嘉宾：

![Melina_wireframe](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450731.gif)

debug模式下渲染72帧用时62秒，一下就慢了下来，至于这位神秘嘉宾具体是谁就先不剧透了，主要是因为202酱只有一张纹理贴图，我们为了后续纹理映射的讲解需要更复杂的mtl文件和纹理文件，所以我们又请了一位嘉宾来做我们的讲师。

## 着色

当然我们肯定不会满足于仅仅使用线框模式来渲染我们的三维模型。

我们也不能直接就让模型按照我们上一节的代码直接运行，那样看到的只不过是把上面的渲染结果从线框变成一片实心白色的轮廓而已。

我们实际上要体现的是光影关系，也就是光照效果，这就是我们目前要做的，写一个简单的着色模型。

我们先来讨论着色位于渲染管线的哪个位置。或者更具体一点，片元着色器应该位于渲染管线的哪个位置。

片元着色器紧跟着光栅化之后，位于各种测试，包括但不限于深度测试、模板测试等的前面。

我们定义一个函数指针用于存储我们使用的片元着色函数，同时在深度测试前调用我们的片元着色器，因此我们要修改edge walking或者edge equation的函数：

```c++
"rasterizer.hpp"
    
namespace rst
{
    class rasterizer
    {
        public:
        ......
            std::function<Vertex(fragmentShaderPayload)> fragmentShader;
    }
}

"rasterizer.cpp"
    void rasterizer::rasterize_edge_walking(const Triangle& m, const std::array<myEigen::Vector4f, 3>& clipSpacePos)
	{
		...
		//scan the bottom triangle
		for (int y = std::ceil(t.vertex[0].vertex.y - 0.5f); y < std::ceil(t.vertex[1].vertex.y - 0.5f); y++)
		{
			...
			for (int i = std::ceil(shortVertex.vertex.x - 0.5f); i < std::ceil(longVertex.vertex.x - 0.5f); i++)
			{
				...
				fragmentShaderPayload payload(pixel, light);
				pixel = fragmentShader(payload);
				if (pixel.vertex.z > z_buffer[get_index(i, y)])
				{
					...
				}
			}
		}
		//scan the top triangle
		for (int y = std::ceil(t.vertex[1].vertex.y - 0.5f); y < std::ceil(t.vertex[2].vertex.y - 0.5f); y++)
		{
			...
			for (int i = std::ceil(shortVertex.vertex.x - 0.5f); i < std::ceil(longVertex.vertex.x - 0.5f); i++)
			{
				...
				fragmentShaderPayload payload(pixel, light);
				pixel = fragmentShader(payload);
				if (pixel.vertex.z > z_buffer[get_index(i, y)])
				{
					...
				}
			}
		}

	}
```

光在打到物体表面的着色点再反射到相机的过程可以暂时简单考虑成两个阶段。

第一个阶段我们要计算出着色点接收到的irradiance是多少，也就是有多少的光入射。

第二个阶段我们要计算出有多少光被反射到相机里了。

### 点光源or平行光

我们来考虑简单的光照情况，点光源和平行光。

先来讨论点光源，一个点光源可以由一个位置和一个intensity定义，intensity可以理解为光源产生出的光的“数量”。

实际上intensity的定义为单位立体角上接收到的光通量(power or radiant flux)的大小，单位为cd(坎德拉)，可以用来更好地代替光通量或者说灯泡功率这个光源变量。

![irradiance随着距离平方而减小](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450840.png)

参考这张来自虎书的插图，我们认为一个点光源在所有方向上的intensity都是相同的，或者说，这个点光源是各向同性的。与此相反的是有时候我们会使用一些被称作"spot lights"的光源，它们只在某些方向上投射光线，自然这些光源也就不是各向同性的了。

第一阶段我们需要计算出着色点的irradiance，irradiance的定义是单位面积接收到的光通量。我们整理一下公式：
$$
d\omega=\frac{dA}{r^2}\\
intensity=\frac{d\Phi}{d\omega}=\frac{d\Phi r^2}{dA}\\
irradiance=\frac{d\Phi}{dA}=\frac{intensity}{r^2}
$$
但是我们计算出来的irradiance是仅当光垂直射向着色点的情况下的，如果光和着色点法线存在一个夹角，那么我们再乘上夹角跟着色点法线的余弦值即可。

而对于平行光而言，我们认为是一组平行且没有衰减的光线，类似太阳光，自然没有衰减我们也不会关心平行光的位置，我们只需要存储平行光的intensity和方向即可。

```c++
	class Light
	{
	public:
		virtual ~Light() = default;
		virtual std::string getType() = 0;
	};

	class PointLight : public Light
	{
	public:
		myEigen::Vector3f position;
		float intensity;
	public:
		PointLight(const myEigen::Vector3f& position, const float intensity)
			:position(position), intensity(intensity){}
		std::string getType() { return std::string("PointLight"); }
	};

	class DirectionalLight : public Light
	{
	public:
		myEigen::Vector3f direction;
		float intensity;
	public:
		DirectionalLight(const myEigen::Vector3f& direction, const float intensity)
			:direction(direction), intensity(intensity) {}
		std::string getType() { return std::string("DirectionalLight"); }
	};
```

我们这节编写代码都将以点光源为主。

### Blinn-Phong着色模型

接下来我们讨论被反射出来的光有多少，这中间涉及了光在投射到物体表面的时候究竟发生了什么。

我们来认识一个比较简单的经验模型，也就是Blinn-Phong着色模型，这个模型认为物体表面反射到相机的光线存在四种类型。

分别是自发光(emission)，环境光(ambient)，漫反射(diffuse)和高光反射(specular)。

先来写一下fragment shader的结构：

```c++
	class fragmentShaderPayload
	{
	public:
		fragmentShaderPayload() = default;
		fragmentShaderPayload(const Vertex& vertex, const PointLight& light)
			:vertex(vertex), light(light) {}
		Vertex vertex;
		PointLight light;
	};

	static Vertex BlinnPhongShader(const fragmentShaderPayload& payload)
	{
        //emission
        
		//ambient

		//diffuse

		//specular

		auto color = emission + ambient + diffuse + specular;

		color = color * 255.0f;

		color.x = color.x > 255.0f ? 255.0f : color.x;
		color.y = color.y > 255.0f ? 255.0f : color.y;
		color.z = color.z > 255.0f ? 255.0f : color.z;
		
		Vertex v = payload.vertex;
		v.vertexColor = TGAColor(color.x, color.y, color.z);
		return v;
	}
```

注意一下我们的颜色使用uint_8存储的，也就是范围在0-255之间，如果计算结果大于255溢出，也就是说过曝光的话会重新从0开始计数，我们暂时处理不了过曝光，所以先把输出的颜色三个分量限制在255以内。但是我们做着色计算的时候需要将颜色映射到0-1范围内，大多数公式都是基于颜色用0-1的浮点数表示的基础来进行计算的。

#### 自发光

自发光其实就是直接将着色点试做一个光源。

我们定义ke为发出的光的颜色，或者说是自发光系数，定义Ie为发出的光的intensity。

```c++
		myEigen::Vector3f ke(0, 0, 0);
		float Ie = 0;

		//emission
		auto emission = ke * Ie;
```

我们不打算让模型自发光，所以都设置为0。

#### 环境光

环境光类似于无差别地给模型上一组颜色，用来模拟一种间接光照的效果。实际在现实中的物体在面对单光源的时候不会因为哪个面背向光源就完全不可见，背向光源的表面也会被光线投射在其他表面上反射出来的光线给照亮，这就是我们说的间接光照。

我们定义一个三维颜色向量ka为环境光系数，再定义一个Ia为环境光的intensity，我们认为环境光就是ka*Ia。
$$
\vec{ambient}=\vec{ka}*Ia
$$

```c++
		myEigen::Vector3f ka(0.005, 0.005, 0.005);

		//ambient
		auto ambient = ka * 50;
```

#### 漫反射

漫反射是粗糙表面的反射，Blinn-Phong模型认为漫反射即粗糙表面在接收到光线后均匀地反射，也就是定义一个三维颜色向量kd为漫反射系数，直接乘以着色点接收到的irradiance就可以得到着色结果。kd一般来说就是我们看到的粗糙表面的颜色rgb值。
$$
\vec{diffuse}=\vec{kd}*irradiance=\vec{kd}*\frac{intensity*cos\theta}{r^2}
$$

```c++
		myEigen::Vector3f kd(payload.vertex.vertexColor.bgra[2] / 255.0f, payload.vertex.vertexColor.bgra[1] / 255.0f, payload.vertex.vertexColor.bgra[0] / 255.0f);	

		//diffuse
		auto pos = myEigen::Vector3f(payload.vertex.vertex.x, payload.vertex.vertex.y, payload.vertex.vertex.z);
		auto normal = myEigen::Vector3f(payload.vertex.normal.x, payload.vertex.normal.y, payload.vertex.normal.z).Normalize();
		auto lightdir = (pos - payload.light.position).Normalize();
		auto r_2 = (pos - payload.light.position).Norm();
		auto diffuse = kd * std::max(0.0f, myEigen::dotProduct(normal, lightdir)) * payload.light.intensity
			/ r_2;
```

#### 高光反射

高光反射用于模拟光滑表面上的反射，Blinn-Phong模型认为高光反射与我们眼睛观察角度、着色点表面法线和光照角度有关，因为镜面反射入射角等于出射角，可想而知如果观察角度刚好处于出射角那么接收到的光线会明显更多。

Blinn-Phong模型给出了如下公式：
$$
\vec{h}=\frac{\vec{lightdir}+\vec{eyedir}}{|\vec{lightdir}+\vec{eyedir}|}\\
\vec{specular}=\vec{ks}*(\vec{h}*\vec{normal})^{Ns}*\frac{intensity*cos\theta}{r^2}
$$
除去经验公式特有的系数ks外，高光反射与漫反射不同的地方就是多乘了一个观察角度与光照角度的半程向量和着色点法线的余弦值的Ns次方，这个Ns用来控制高光范围，Ns越大高光范围越小。

值得一提的是，Blinn-Phong模型其实是改进自另一经典模型Phong模型的结果，在Phong模型中半程向量和法线的余弦值被替换成观察角度和光照角度的余弦值，这显然是没有Blinn-Phong模型模拟地更准确的。

```c++
		myEigen::Vector3f ks(0.7937, 0.7937, 0.7937);
		//specular
		auto viewdir = (-pos).Normalize();
		auto h = (viewdir + lightdir).Normalize();
		auto specular = ks * payload.light.intensity * std::pow(std::max(0.0f, myEigen::dotProduct(h, normal)),20.0f)
			/ r_2;
```

注意一下我们填入的一些常量数值，这些数值需要尽量调试到一个合理的范围内，避免出现曝光过度或者环境光太亮的结果。

#### 着色结果

其实真要是认真看了上面内容的同学应该已经一头雾水了，怎么第一阶段还有模有样地在那儿计算能量和irradiance，第二阶段就啥也不管硬套系数了。

这就是经验模型的缺点，第二阶段的计算完全没有与能量有关的计算，更多的是从经验出发拟合出的一个简单模型，让我们看看这个经验模型的结果。

![BlinnPhong](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450909.gif)

模型来自于Fromsoftware的游戏EldenRing的角色Melina，仅作学习用途！

效果其实还不错，虽然肯定是不符合物理规律的，但是看起来还挺准确。

## MTL文件格式

我们之前已经看过了202酱的MTL文件，但是202酱只使用了一张漫反射纹理，我们需要有更全面的MTL文件。

梅琳娜的模型文件来自于法环的游戏解包，我看MMD区都在用，我拿来用一用应该也没事，源文件为FBX格式，我使用了blender导出为obj-mtl格式，并且顺带把所有png纹理改成了tga纹理。

我们将以梅梅的MTL文件中的其中一个材质定义来看看MTL文件是怎么关联纹理和材质的。

```mtl
newmtl 40_+cape_0.1_0_0
Ns 96.078415
Ka 1.000000 1.000000 1.000000
Ke 0.000000 0.000000 0.000000
Ni 1.450000
d 0.000000
illum 1
map_Kd Melina_Cloth_D(Baked).tga
map_Ks Melina_Cloth_S.tga
map_Bump -bm 1.000000 Melina_Cloth_N(Baked).tga
```

可以看到上镜的很多都是老朋友，Ns，Ka，Ke，Kd，Ks都在我们之前的公式上出现过，也就是我们的fragment payload中将会加入一个material类绑定上这个片元应该对应的这些Ka，Kd等数据，然后直接在shader中使用这些现成数据即可。至于没出现过的量，Ni为光密度，用于计算折射的时候使用，因为我们的光照模型没有折射，我们也暂时不考虑透明物体的渲染，我们暂时不考虑。同样的，d指dissolve，指的是当前材质溶解于背景的程度，我们也暂时不考虑。

然后是重量级的illum关键字，illum后跟着的数字为0到10中的一个整数，每个整数代表了一种光照模型，具体可以根据这个博客中的讲解来看。

http://www.paulbourke.net/dataformats/mtl/

中文论坛上的关于mtl文件的解析都是直接搬的这篇文章，但是就搬了前面的部分，后面对光照模型的解析完全没搬。。。大家可以直接翻到这篇文章的最后一部分看下光照模型的详细解析。

我这里就搬几个常见的，illum 0对应的是一个只有颜色光照模型，输出颜色公式是color = Kd，不进行任何光照计算。

illum 2对应的就是我们之前写的Blinn-Phong着色模型，而illum 1对应的则是除去了高光的Blinn-Phong模型，只计算环境光和漫反射项。

剩下的几种暂时用不到就先不提了，可以看看文章里怎么写的。

我们可以看到有些变量使用了map前缀，这意味着这个变量的值取自一张纹理贴图的rgb值，每一个顶点都有一个纹理坐标，又称uv，每张纹理我们都会以左下角为原点，建立一个直角坐标系，两个坐标轴分别为u和v，每张纹理的uv坐标范围为[0, 1]，每当我们需要从纹理中读取rgb值的时候都会以这个顶点对应的uv坐标为标准去查询，而uv是一个二维向量，我们只需像法线、顶点颜色那样当成一个顶点的属性即可，我们的所有线性插值和重心坐标插值以及透视矫正都已经在上一节写好了，我们只需要将texcoord也就是uv加进Vertex类就行。

## 纹理映射

### 读取MTL文件

MTL文件的读取我们放在material类的构造函数里：

```c++
"material.h"
    
#pragma once
#include <string>
#include "myEigen.hpp"
#include "tgaimage.h"

namespace rst
{
	class Material
	{
	public:
		Material() = default;
		Material(const std::string& filename, const std::string& root, const std::string& materialName);
	public:
		std::string name;

		myEigen::Vector3f Ke;
		myEigen::Vector3f Kd;
		myEigen::Vector3f Ka;
		myEigen::Vector3f Ks;
		float illum;
		float Ns;
		float d;
		float Ni;

		TGAImage map_Ke;
		TGAImage map_Kd;
		TGAImage map_Ka;
		TGAImage map_Ks;
		TGAImage map_Ns;
		TGAImage map_d;

		float Bump;
		TGAImage map_Bump;
	};
}

"material.cpp"

#include "material.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace rst
{
	Material::Material(const std::string& filename, const std::string& root, const std::string& materialName)
	{
		name = materialName;

		std::ifstream mtl;
		mtl.open(root + filename, std::ifstream::in);
		if (mtl.fail())
		{
			std::cerr << "Cannot open:" << filename << std::endl;
			return;
		}
		std::string mtlLine;
		std::vector<std::string> mtlDescribe;
		bool isWrite = false;
		while (!mtl.eof())
		{
			std::getline(mtl, mtlLine);

			if (mtlLine.compare(0, 7, "newmtl ") == 0)
				if (mtlLine.compare(7, materialName.size(), materialName) == 0)
					isWrite = true;

			if(isWrite)
			{
				if (mtlLine.size() == 0) break;
				mtlDescribe.push_back(mtlLine);
			}
		}
		for (auto& s : mtlDescribe)
		{
			std::istringstream iss(s);
			std::string header;
			char trash;
			if (s.compare(0, 3, "Ns ") == 0)
			{
				iss >> header;
				iss >> Ns;
			}
			else if (s.compare(0, 3, "Ke ") == 0)
			{
				iss >> header;
				iss >> Ke.x >> Ke.y >> Ke.z;
			}
			else if (s.compare(0, 3, "Ka ") == 0)
			{
				iss >> header;
				iss >> Ka.x >> Ka.y >> Ka.z;
			}
			else if (s.compare(0, 3, "Kd ") == 0)
			{
				iss >> header;
				iss >> Kd.x >> Kd.y >> Kd.z;
			}
			else if (s.compare(0, 3, "Ks ") == 0)
			{
				iss >> header;
				iss >> Ks.x >> Ks.y >> Ks.z;
			}
			else if (s.compare(0, 2, "d ") == 0)
			{
				iss >> header;
				iss >> d;
			}
			else if (s.compare(0, 3, "Ni ") == 0)
			{
				iss >> header;
				iss >> Ni;
			}
			else if (s.compare(0, 6, "illum ") == 0)
			{
				iss >> header;
				iss >> illum;
			}
			else if (s.compare(0, 7, "map_Ke ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				map_Ke = TGAImage();
				map_Ke.read_tga_file(root + textureName);
				map_Ke.flip_vertically();
			}
			else if (s.compare(0, 7, "map_Ka ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				map_Ka = TGAImage();
				map_Ka.read_tga_file(root + textureName);
				map_Ka.flip_vertically();
			}
			else if (s.compare(0, 7, "map_Kd ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				map_Kd = TGAImage();
				map_Kd.read_tga_file(root + textureName);
				map_Kd.flip_vertically();
			}
			else if (s.compare(0, 7, "map_Ks ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				map_Ks = TGAImage();
				map_Ks.read_tga_file(root + textureName);
				map_Ks.flip_vertically();
			}
			else if (s.compare(0, 7, "map_Ns ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				map_Ns = TGAImage();
				map_Ns.read_tga_file(root + textureName);
				map_Ns.flip_vertically();
			}
			else if (s.compare(0, 6, "map_d ") == 0)
			{
				std::string textureName;
				iss >> header;
				iss >> textureName;
				map_d = TGAImage();
				map_d.read_tga_file(root + textureName);
				map_d.flip_vertically();
			}
			else if (s.compare(0, 9, "map_Bump ") == 0)
			{
				std::string textureName;
				iss >> header >> header >> Bump;
				iss >> textureName;
				map_Bump = TGAImage();
				map_Bump.read_tga_file(root + textureName);
				map_Bump.flip_vertically();
			}
		}
	}
}
```

可以看到我们每次读取纹理文件的时候都会水平翻转图像，这是因为我在之前测试的时候怎么也写不对纹理映射，后来我把纹理读了再原封不动输出才发现tgaimage.cpp读取的纹理文件竟然是水平翻转的，具体原因我暂时也没排查出为什么，我猜测也许是tga文件头里的imagedescriptor的读取出了问题？但是使用blender是可以正常打开并且渲染正确的，我觉得也许是代码的问题。但是anyway，我们就先这样翻转着吧，纹理映射的重点还在后头，就是翻转函数的开销其实比较大，给我们的纹理和模型读取陡增了一些计算时间。

### 双线性插值

接下来我们有了material，将它传到fragmentpayload里就可以在shader里读取相关信息了。

但是对于使用纹理存储的变量信息，我们需要根据uv坐标从图上读取出来。

听起来很简单，tga解析器为我们提供了set和get的方法，直接get不就行了？

但是我们的uv坐标在乘以tga的长宽之后显然都是打乱的，几乎不可能恰好取在像素中心，那对于这些采样点我们怎么找他的对应颜色呢？我们当然可以直接找到离采样点最近的像素中心坐标，返回这个像素的rgb值，这种方法被称作nearest，取最近点。

但我们有更准确且比较清晰的办法，那就是双线性插值。

我们之前的线性插值都是一维的两个点，但在纹理这种二维平面上可不使用，可以看这个示意图：

![BilinearInterpolation](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450986.png)

这张图很显然来自games101闫令琪老师的课件。

图里写的非常明白，还附上了伪代码，可以看出就是做三次插值，水平方向两次竖直方向一次，当然反过来应该也是没问题的。

需要时刻注意像素中心坐标不是整数而是x.5的小数。

同时除了插值以外还有一件比较重要的事，就是纹理环绕方式。

uv坐标虽然我们都是基于[0, 1]范围内讨论的，但有时候我们会用到超出这个范围的uv坐标，这时候我们怎么处理这些超范围的坐标？

当然也是有多种方法，大部分情况下默认repeat，就是大于1就再从0开始递增，相当于纹理在边缘处重复，这有点像桌面壁纸平铺的表现形式。

也有其他的情况，比如超出范围就重复边缘处的像素颜色，或者干脆超出范围就不画了，也是可以的，但是最广泛的还是repeat的形式。

```c++
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

		auto c_u1 = myEigen::Vector2f(ceil(uu), floor(vv));
		auto f_u1 = myEigen::Vector2f(floor(uu), floor(vv));

		TGAColor u_interpolated1 = ColorLerp(texture.get(f_u1.x, f_u1.y), texture.get(c_u1.x, c_u1.y), uu - floor(uu));

		auto c_u2 = myEigen::Vector2f(ceil(uu), ceil(vv));
		auto f_u2 = myEigen::Vector2f(floor(uu), ceil(vv));

		TGAColor u_interpolated2 = ColorLerp(texture.get(f_u2.x, f_u2.y), texture.get(c_u2.x, c_u2.y), uu - floor(uu));

		TGAColor out = ColorLerp(u_interpolated2, u_interpolated1, vv - floor(vv));
		return out;
	}
```

至于调用这个函数的代码有点过于冗长了，我就不放在这里了，感兴趣的可以直接从我的代码仓库里查看。

我们看一下渲染效果：

![双线性插值](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450077.gif)

其实到现在debug模式已经有点吃力了，渲染72帧起码180秒往上，其实占用开销大头的还是纹理读取，我现在都快常驻release模式了，这72帧在release模式下算上模型和纹理加载也就30秒上下，我开的是-O2优化。

有个值得一提的事，那就是我发现mtl文件里梅梅的大部分光照模型都是illum 1，也就是不考虑高光反射，但是所有梅梅模型的所有部分都配了高光反射纹理，也就是map_Ks。

这显然是不合理的，问题应该出在fbx和obj-mtl格式的转换上，所以我擅做主张把梅梅的mtl文件里的illum 1全部改成了illum 2。

看看效果：

![高光贴图](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450171.gif)

可以看到最明显的应该是靴子上的高光。

### 法线贴图

说了这么多，其实mtl文件里有一项一直没被我们用到，那就是map_Bump。

我们之前说不管是纹理贴图还是直接写在mtl文件里，都是为了用存储在外部的数据来替换我们公式里的一些变量，自然而然的，表面法线作为一个重要变量显然也可以被替换。

怎么通过一张图来记录表面法线的信息，这不是一个简单的话题。

#### 凹凸贴图

比较原始的一种方案称作凹凸贴图，用一张灰度图来记录高度信息，越亮的像素越“高”，或者说越凸。

这种方法现在用的比较少了，games101中一开始讲的也是凹凸贴图的原理，可以去看视频里怎么说的。

#### 法线贴图

我们的重点还是法线贴图。因为法线在经过标准化后，xyz三个分量基本上就在-1到1的区间内，所以可以很简单地映射到0到255的颜色信息上表示。

说到这里最容易想到的就是直接跟漫反射贴图啥的一样，给每一个uv对应一个颜色存储法线信息，使用的时候直接采样出来直接替换。

这种方式准确的说叫做世界空间法线贴图。以下是一张世界空间法线贴图的例子。

![diablo3_pose_nm](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450282.png)

这张贴图来自于tinyrenderer的迪亚波罗的世界空间法线贴图。可以看到比较五颜六色，使用的时候很方便直接采样就可以。

但是如果我们需要人物做一些动作呢，人物一旦做了一些动作之后，这个法线贴图似乎就不怎么起效了，因为这里的法线指向都是写死的，假设人物只是把手掌翻了一个面，我们的法线读取都会出现问题。

所以我们更多使用的是切线空间法线贴图，以下是另一个例子：

![diablo3_pose_nm_tangent](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450379.png)

这个是切线空间法线贴图，这里存储的信息跟上一张贴图是一模一样的，也是来自tinyrenderer的贴图。

简单来说，就是我们在模型的每个表面构建TBN三个向量作为切线空间的三个基向量。这三个向量分别为Tangent, Bitangent, normal三个向量，所以简写为TBN。

其中，T和B向量分别指向模型u和v在三维空间中延伸的方向，这样说可能比较抽象，需要想象一下。每个模型表面上的点都会有他专属的uv坐标，我们聚焦到模型表面的一个最简单的三角面，就像下面这张图，我们图里画的红线和蓝线就是u和v两个量在三维空间中的“**等势面**”。

![grid_texture](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450484.png)

而N向量就是每个顶点原本的法线，由此我们构建出了一个局部坐标系，我们在切线空间法线贴图中采样到的值就是基于这个局部坐标系的。

**（实际上我说的也不是完全准确。我们规定N向量必须与T和B向量垂直。很显然对于如图所示的扁平的三角形面内，TB看似应该是两个固定的向量，然而因为三角形内的每个顶点对应的法线并不一样，我们构建TBN空间需要明确N与TB垂直，所以实际上对于每个顶点，我们都需要根据每个顶点所拥有的具体的法线值来将这个三角形内的T和B向量稍稍旋转一下来做到跟N向量垂直。这个矛盾主要是因为虽然我们用三角形来模拟物体表面，但是实际上我们做的顶点插值等工作超越了简单的三角形，做到了用三角形来模拟复杂表面的结果，所以我们不能再简单地将物体表面看做是三角形，也就是说我们要求的T和B向量并不是uv在三角形内延伸的方向，而是具体到每一个顶点上延伸的方向）**

所以我们在采样完了法线贴图中的值后，我们需要做的就是将这个法线从局部坐标系变换到模型坐标系，然后再替换掉原有的法线值，这样法线贴图才能正常运作。

这一整串步骤的难点就在于如何得到一个变换矩阵，用来将这个法线从切线空间转换为模型坐标系，这个变换矩阵我们一般也称作TBN矩阵。

我们之前讨论过坐标系变换矩阵怎么写，因为这里不涉及到位置的变换，所以只用一个3x3矩阵就可以做到，同时这个3x3矩阵第一列，第二列和第三列分别为TBN三个向量在模型空间中的值。

N向量就是原本的法线向量，这个不用管，重点在于T和B，我们该如何得到三维空间中u和v的延伸方向向量？

我们看上面的图可以想象，先只关注红线，也就是u的**等势面**，或者可以理解成**等高线**，我们真正想要求得的是u向量本身，也就是这个我们画出来的等高线的**梯度**（gradient），梯度有一个比较简单的求法，假设我们有一个线性函数。$$u=f(x, y, z)=Ax+By+Cz+D$$，这个线性函数的梯度就是(A, B, C)，也就是T向量就是(A, B, C)。同样我们也可以令v=f(x, y, z)，写出一个关于v的线性函数，对他求梯度就可以得到B向量了，最后的N向量就直接使用正交化后的顶点法线就可以了。

我们用以上思路来以p0, p1, p2三个顶点的数据为基础来求解一下T向量：
$$
u_0=u(p_0)=Ax_0+By_0+Cz_0+D\\
u_1=u(p_1)=Ax_1+By_1+Cz_1+D\\
u_2=u(p_2)=Ax_2+By_2+Cz_2+D\\
u_1-u_0=A(x_1-x_0)+B(y_1-y_0)+C(z_1-z_0)\\
u_2-u_0=A(x_2-x_0)+B(y_2-y_0)+C(z_2-z_0)
$$
就只靠后面两个方程可求不出一个唯一解，我们还需要根据N和TB垂直来写出第三个方程组来计算出唯一解。

**（这里第三个方程组实际上就对应着我前面括号内所说的将三角形内看似固定的T和B向量“旋转”到合适位置的过程，如果这个三角形内的所有顶点的法线都相同，也就是使用面法线代替顶点法线，那么三角形内的T和B向量就真的是“固定”的向量了，但大多数时候我们用的插值得到的顶点法线，所以每个顶点对应的T和B向量各不相同）**
$$
0=(A,B,C)\cdot{\vec{N}}=\vec{T}\cdot{\vec{N}}
$$
我们将上面两个方程也改写成向量点乘形式：
$$
\vec{T}\cdot{\vec{p_1p_0}}=u_1-u_0\\
\vec{T}\cdot{\vec{p_2p_0}}=u_2-u_0\\
\vec{T}\cdot{\vec{N}}=0\\
$$
求解这个方程组很简单，改写成矩阵形式就行：
$$
\begin{bmatrix}\vec{p_1p_0}\\
\vec{p_2p_0}\\
\vec{N}
\end{bmatrix}
\cdot\vec{T}=\begin{bmatrix}u_1-u_0\\u_2-u_0\\0\end{bmatrix}
$$

$$
\vec{T}=\begin{bmatrix}\vec{p_1p_0}\\
\vec{p_2p_0}\\
\vec{N}
\end{bmatrix}^{-1}\begin{bmatrix}u_1-u_0\\u_2-u_0\\0\end{bmatrix}
$$

对于B向量也可以如法炮制：
$$
\vec{B}=\begin{bmatrix}\vec{p_1p_0}\\
\vec{p_2p_0}\\
\vec{N}
\end{bmatrix}^{-1}\begin{bmatrix}v_1-v_0\\v_2-v_0\\0\end{bmatrix}
$$
这样我们就求出T向量和B向量了，带上原本的N向量就可以构建出一个切线空间了。

需要注意的是，我们这里使用到了三角形的顶点信息，所以我这里顺带将之前的MVP矩阵变换单独封装成了vertex shader，顺带可以跟我们现在写的fragment shader数据互通。

```c++
                float u = vertex.texcoord.x;
                float v = vertex.texcoord.y;
                float w = material->map_Bump[mipmap_level].width();
                float h = material->map_Bump.height();
                TGAColor uv = bilinearInterpolate(material->map_Bump,
                    u * w, v * h);

                myEigen::Vector3f uv_vec((double)uv.bgra[2] * 2.0f / 255.0 - 1.0f,
                                         (double)uv.bgra[1] * 2.0f / 255.0 - 1.0f,
                                         (double)uv.bgra[0] * 2.0f / 255.0 - 1.0f);

                myEigen::Vector3f			n = uv_vec.Normalize();

                myEigen::Vector3f        p1p0 = (tri_cameraspace.vertex[1].vertex - tri_cameraspace.vertex[0].vertex).xyz();
                myEigen::Vector3f        p2p0 = (tri_cameraspace.vertex[2].vertex - tri_cameraspace.vertex[0].vertex).xyz();
                myEigen::Vector3f	       fu = myEigen::Vector3f(tri_cameraspace.vertex[1].texcoord.x - tri_cameraspace.vertex[0].texcoord.x,
                                                                  tri_cameraspace.vertex[2].texcoord.x - tri_cameraspace.vertex[0].texcoord.x, 0.0f);
                myEigen::Vector3f	       fv = myEigen::Vector3f(tri_cameraspace.vertex[1].texcoord.y - tri_cameraspace.vertex[0].texcoord.y,
                                                                  tri_cameraspace.vertex[2].texcoord.y - tri_cameraspace.vertex[0].texcoord.y, 0.0f);
                myEigen::Matrixf3x3         A = myEigen::Matrix3x3Transpose(myEigen::Matrixf3x3(p1p0, p2p0, Normal));
                auto				A_inverse = myEigen::Matrix3x3Inverse(A);
                auto				A_inverse = myEigen::Matrix3x3Inverse(A);

                myEigen::Vector3f           T = A_inverse * fu;
                myEigen::Vector3f           B = A_inverse * fv;

                myEigen::Matrixf3x3 TBN(T.Normalize(), B.Normalize(), Normal);
                Normal = (TBN * n).Normalize();
```

我们来看看渲染结果：

![Normalmap](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450559.gif)

可以跟之前的结果对比一下，最明显的差别就是靴子上和手臂袖子上的褶皱。可以说效果非常的棒。

### Mipmap

本章的最后一项内容就是mipmap。

当我们采样的贴图精度过大，而渲染区域又过小的时候，我们的采样结果会出现失真。

什么情况才叫过大？就是当屏幕上跨越一格像素，对应的纹理采样跨越了远超一格像素的时候。就像我们现在渲染的梅琳娜，她使用的纹理都是4096x4096的，而我们渲染的图片也就700x700的大小，毫无疑问就会出现这种摩尔纹现象，但是比较轻微。

![mipmap演示](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450641.gif)

可以参考左边的图片，产生的这种纹路我们称之为摩尔纹，是一种失真的表现，而我们对应的反走样策略就是mipmap。

可以对比左右两张图来观察mipmap做了什么，实际上mipmap的就是在远处将贴图精度适当降低，达到一种“模糊”的效果。

在gpu渲染中，mipmap的生成属于是gpu集成的算法，在opengl里只要寥寥几行就可以自动开启mipmap，而现在我们需要聚焦mipmap的相关算法。

其实mipmap的算法说起来非常的简单，我们只需要在读取纹理的时候自动生成低精度的纹理即可，假设我们有一张1024x1024的纹理，那么我们读取这个纹理的时候会自动在内存中创建512x512、256x256、128x128一直到1x1的对应纹理，至于这个过程怎么创建，其实就是每四格像素取平均合成一个像素即可。当我们需要采样纹理的时候，我们就会从对应的mipmap层级的纹理进行采样。

我们先把容易的部分也就是加载mipmap的代码写好：

```c++
	int calculate_mipmaplevels(int mipmap_width, int mipmap_height)
	{
		int mipmap_levels = 1;
		int size = std::min(mipmap_width, mipmap_height);
		while (size > 1)
		{
			mipmap_levels++;
			size = std::max(1, size / 2);
		}
		return mipmap_levels;
	}

	std::vector<TGAImage> generateMipmap(const TGAImage& texture)
	{
		std::vector<TGAImage> mipmap;

		int mipmap_width = texture.width();
		int mipmap_height = texture.height();

		int mipmap_levels = calculate_mipmaplevels(mipmap_width, mipmap_height);

		for (int level = 0; level < mipmap_levels; level++)
		{
			TGAImage mipmap_level_data(mipmap_width, mipmap_height, TGAImage::RGB);
			for (int y = 0; y < mipmap_height; y++)
			{
				for (int x = 0; x < mipmap_width; x++)
				{
					int image_x = x * texture.width() / mipmap_width;
					int image_y = y * texture.height() / mipmap_height;
					TGAColor pixel = texture.get(image_x, image_y);

					mipmap_level_data.set(x, y, pixel);
				}
			}

			mipmap.push_back(mipmap_level_data);

			mipmap_width = std::max(1, mipmap_width / 2);
			mipmap_height = std::max(1, mipmap_height / 2);
		}
		return mipmap;
	}
```

难点是采样的时候怎么计算对应的mipmap层级，参考下图：

![mipmaplevel](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450735.png)

我们需要知道在屏幕空间中跨越一格像素的时候，uv的增量是多少，也就是uv的变化量的模长，然后在生成好的mipmap中找到对应的纹理，使得屏幕空间跨越一格像素时，uv的增量接近于1。

那么问题就在于我们怎么访问到一格像素的相邻像素，实际上，在现代的计算机gpu渲染中，并不是1个1个像素进行渲染的，比较常见的会使用2x2的像素进行渲染，对于这4个像素，我们会计算ddx和ddy，也就是x方向上跨越一格像素时uv的变化量以及y方向上跨越一格像素时uv的变化量（实际上不一定是uv的变化量，可以换成任意顶点属性的变化量），然后对于这4个像素我们会共用ddx和ddy的数值。

那么我们现在要改造一下我们的光栅化代码，从原本的1个像素1个像素渲染的方式改进为一次传入四个像素。

因为edge walking的改造相对有点难度，所以我们将光栅化方式全面换成edge equation，并且对edge equation进行修改：

```c++
	void rasterizer::rasterize_edge_equation(const Triangle& m, IShader& shader, const std::array<myEigen::Vector4f, 3>& clipSpacePos)
	{
		int boundingTop = std::ceil(std::max({ m.vertex[0].vertex.y, m.vertex[1].vertex.y, m.vertex[2].vertex.y }));
		int boundingBottom = std::floor(std::min({ m.vertex[0].vertex.y, m.vertex[1].vertex.y, m.vertex[2].vertex.y }));
		int boundingRight = std::ceil(std::max({ m.vertex[0].vertex.x, m.vertex[1].vertex.x, m.vertex[2].vertex.x }));
		int boundingLeft = std::floor(std::min({ m.vertex[0].vertex.x, m.vertex[1].vertex.x, m.vertex[2].vertex.x }));

		for (int y = boundingBottom; y < boundingTop; y += 2)
		{
			for (int x = boundingLeft; x < boundingRight; x += 2) 
			{
				Vertex pixel[2][2];
				for (int a : {x, x + 1})
				{
					for (int b : {y, y + 1})
					{
						if (insideTriangle(m, (float)a + 0.5, (float)b + 0.5))
						{
							pixel[a - x][b - y] =
								barycentricPerspectiveLerp(m, myEigen::Vector2f(a + 0.5, b + 0.5), clipSpacePos);
						}
					}
				}

				float ddx = -1;
				float ddy = -1;

				for (int i : {0, 1})
				{
					if (!pixel[i][0].empty() && !pixel[i][1].empty())
					{
						ddx = std::max(fabs((pixel[i][1].texcoord - pixel[i][0].texcoord).Length()), ddx);
					}
					if (!pixel[0][i].empty() && !pixel[1][i].empty())
					{
						ddy = std::max(fabs((pixel[0][i].texcoord - pixel[1][i].texcoord).Length()), ddy);
					}
				}

				shader.setddx(ddx);
				shader.setddy(ddy);

				for (int a : {0, 1})
				{
					for (int b : {0, 1})
					{
						if (pixel[a][b].empty()) continue;
						pixel[a][b].vertexColor = shader.FragmentShader(pixel[a][b]);
						if (pixel[a][b].vertex.z > z_buffer[get_index(x + a, y + b)])
						{
							image.set(x + a, y + b, pixel[a][b].vertexColor);
							z_buffer[get_index(x + a, y + b)] = pixel[a][b].vertex.z;
						}
					}
				}
			}
		}
	}
```

我们使用-1的ddx和ddy值用来表示ddx和ddy不存在的情况，也就是四个像素有几个是空像素的情况，这时候我们直接使用长宽最大的mipmap即可。

然后是采样mipmap的过程：

```c++
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

				TGAColor var = bilinearInterpolate(texture[mipmap_level],
					texcoord.x * texture[mipmap_level].width(),
					texcoord.y * texture[mipmap_level].height());
				return myEigen::Vector3f(var.bgra[2] / 255.0, var.bgra[1] / 255.0, var.bgra[0] / 255.0);
			}
			else { return myEigen::Vector3f(-200, -200, -200); }
		}
```

这样我们的mipmap基本完成了，让我们来看看结果：

![MipMap](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450833.gif)

区别不是很大，主要是衣服上的渲染有点差别，原本的法线贴图在衣服上有一定的摩尔纹现象，不过需要改成白模渲染才容易看出来。

#### 三线性插值(Trilinear interpolation)

如果跟着写了之前的代码就会发现一个问题，我们判断mipmaplevel的方式是离散的，也就是不平滑的，假设我们计算出来的mipmaplevel是3.4，那么我们直接使用mipmaplevel为3的纹理进行采样，可实际上我们完全可以这么操作，在mipmaplevel为3和4的纹理上都进行一次采样，然后根据这个level的小数部分也就是0.4为插值权重对两次结果进行一次插值。

因为我们在纹理上采样的过程是双线性插值，我们实际上是进行了两次双线性插值，再将最终结果进行一次插值，所以这个过程被我们称作三线性插值。

我们来简单写一下，稍微改写下之前的函数：

```c++
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
```

基本上就差不多了，注意两个边界情况不需要使用三线性插值，让我们看下结果：

![三线性插值](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202301180450962.gif)

在缩略图情况下看不出差别是正常的，我用photoshop放大看才能看出些许平滑感，实际上三线性插值在这里就是不明显，日后也许有渲染大型场景的机会，这时三线性插值甚至mipmap本身的影响都会比较明显。

## 结语

基本上渲染出这张gif的时候我们的任务就基本完成了，下一节应该轮到阴影的渲染了，这又是一个大问题。

话说前面的一些文章有一些错误，我已经尽可能修改重新上传过了，但是文章一直没什么人看，我想可能跟我的文章的形式有关系，我的文章太长太综合了，等我结束这个系列后我会尽量就几个重点问题再写几篇文章，比如mipmap到三线性插值再到我们这里没有实现的各向异性过滤我会专门再开一篇文章讲一下，包括法线贴图原理我也觉得再开个专门文章讲一下比较好，反正今天之后更新频率会尽可能上去，最近忙的事情基本上告一段落了，除非我一整个寒假都在打火纹，不然文章的更新应该会有所保证。
