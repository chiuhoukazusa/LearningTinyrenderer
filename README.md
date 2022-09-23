# 03.HelloTriangles|MVP变换

## 本项目代码已托管至github，将会随着博客实时更新进度

每一节的工程我都会创建一个新的分支，分支名由这一节的数字决定。

[https://github.com/chiuhoukazusa/LearningTinyrenderer/tree/03](https://github.com/chiuhoukazusa/LearningTinyrenderer/tree/03)

## 前言

在上一节中，我们实现了一个基本可用的3d数学库，我们马上就会使用到他们。

这一节中我们将实现渲染的第一个流程，MVP变换。这一节的目标就是实现games101的作业1的效果，我们将会传入跟101作业中一样的参数，并检验产生的三角形是否一致。

MVP变换并不是有了上一节的矩阵运算法则基础就一下子就可以讲清的，要理解MVP变换，要先了解矩阵与图形变换的关系。

本节引用的图片来自《Fundamentals of Computer Graphics Fifth Edition》和《Physically Based Rendering From Theorty To Implementation Third Edition》，下文将分别简述为虎书和PBRT-v3。

## 变换矩阵(Transformation Matrix)

### 从线性变换到仿射变换(Affine Transformation)

大部分情况下，我们的变换都是在变换坐标，而非变换向量。

我们可以使用一个矩阵来左乘一个坐标，将这个坐标转变为另一个坐标，这就是我们常说的线性变换。在线性变换中，变换一个三维坐标需要左乘一个三阶方阵。

在这个基础下，我们可以使用矩阵来进行缩放、旋转等变换，但唯独平移不行，如果我们需要将一个坐标进行平移，我们只能采取以下公式：
$$
x^{'}=x+x_t\\y^{'}=y+y_t\\z^{'}=z+z_t
$$
这样的公式显然没法化成一个矩阵左乘一个坐标的形式，这就意味着平移不属于线性变换。如果我们硬要讲平移加入我们的变换系统中，我们可以单独将平移分为一类，与线性变换错开，线性变换时使用矩阵左乘，平移时使用坐标与向量相加的形式。

这当然也是可行的，但是我们可以有更聪明的办法。我们可以将变换矩阵由3维变成4维，给所有的3维坐标增加一个w分量来变成4维坐标：
$$
\begin{bmatrix}x^{'}\\y^{'}\\z^{'}\\w\end{bmatrix}=
\begin{bmatrix}1\quad0\quad0\quad x_t\\
0\quad 1\quad 0\quad y_t\\
0\quad 0\quad 1\quad z_t\\
0\quad 0\quad 0\quad 1
\end{bmatrix}
\begin{bmatrix}x\\y\\z\\w\end{bmatrix}=
\begin{bmatrix}x+x^{'}\\y+y^{'}\\z+z^{'}\\w\end{bmatrix}
$$
在这里，我们实际上引入了齐次坐标的概念。我们在这里规定w=1，当且仅当w=1时的坐标才是我们规定的“合法”的点的坐标，如果w不为1，我们就将这个坐标的各个分量都除以w，让他变为标准的(x, y, z, 1)形式。特别的，当w分量等于0的时候，(x, y, z, 0)将代表一个向量。

依照上述公式，我们成功将平移也纳入了矩阵左乘坐标的体系中，我们将这种变换称之为仿射变换。下面我来讲述仿射变换下各种变换的形式，同时我们需要定义一下Transform类：

```C++
namespace rst {
	class Transform {
	public:
		Transform(){}
		Transform(const float mat[4][4]) {
			m = myEigen::Matrixf4x4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
									mat[1][0], mat[1][1], mat[1][2], mat[1][3],
									mat[2][0], mat[2][1], mat[2][2], mat[2][3],
									mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
			mInv = myEigen::Matrix4x4Inverse(m);
		}
		Transform(const myEigen::Matrixf4x4& m) :m(m), mInv(myEigen::Matrix4x4Inverse(m)) {}
		Transform(const myEigen::Matrixf4x4& m, const myEigen::Matrixf4x4& mInv) :m(m), mInv(myEigen::Matrix4x4Inverse(mInv)) {}


	private:
		myEigen::Matrixf4x4 m, mInv;
	};
}
```

可以直观地理解，矩阵的逆在几何上意味着逆变换，所以一个Transform类不仅要维护变换矩阵，还需要维护这个变换矩阵的逆矩阵。

### 平移(Translate)

其实平移已经差不多讲完了，我在这里把之前刚刚写了一遍的矩阵公式重复一遍：
$$
\begin{bmatrix}x^{'}\\y^{'}\\z^{'}\\w\end{bmatrix}=
\begin{bmatrix}1\quad0\quad0\quad x_t\\
0\quad 1\quad 0\quad y_t\\
0\quad 0\quad 1\quad z_t\\
0\quad 0\quad 0\quad 1
\end{bmatrix}
\begin{bmatrix}x\\y\\z\\w\end{bmatrix}=
\begin{bmatrix}x+x^{'}\\y+y^{'}\\z+z^{'}\\w\end{bmatrix}
$$
这其实很容易转换成代码：

```c++
	Transform Translate(const myEigen::Vector3f delta) {
		myEigen::Matrixf4x4 m(1.0f, 0.0f, 0.0f, delta.x,
							0.0f, 1.0f, 0.0f, delta.y,
							0.0f, 0.0f, 1.0f, delta.z,
							0.0f, 0.0f, 0.0f, 1.0f);
		myEigen::Matrixf4x4 mInv(1.0f, 0.0f, 0.0f, -delta.x,
							0.0f, 1.0f, 0.0f, -delta.y,
							0.0f, 0.0f, 1.0f, -delta.z,
							0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m, mInv);
	}
```

### 缩放(Scale)

缩放也是一种比较简单的变换形式，只要将坐标的三个分量各自乘以一个比例就可以做到。这里放一张虎书的插图，虽然是2d的，但是可以自行脑补成3d。

![缩放](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202209231208950.png)

我们可以写出缩放矩阵公式：
$$
\begin{bmatrix}x^{'}\\y^{'}\\z^{'}\\1\end{bmatrix}=
\begin{bmatrix}S_x\quad0\quad0\quad 0\\
0\quad S_y\quad 0\quad 0\\
0\quad 0\quad S_z\quad 0\\
0\quad 0\quad 0\quad 1
\end{bmatrix}
\begin{bmatrix}x\\y\\z\\1\end{bmatrix}=
\begin{bmatrix}xS_x\\ySy\\zSz\\1\end{bmatrix}
$$
这是一个非常简单的矩阵，我们也可以写出非常简单的代码：

```C++
	Transform Scale(float x, float y, float z) {
		myEigen::Matrixf4x4 m(x, 0.0f, 0.0f, 0.0f,
							0.0f, y, 0.0f, 0.0f,
							0.0f, 0.0f, z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f);
		myEigen::Matrixf4x4 mInv(1.0f / x, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f / y, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f / z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m, mInv);
	}

	Transform Scale(const myEigen::Vector3f& scale) {
		return Scale(scale.x, scale.y, scale.z);
	}
```

### 旋转(Rotate)

旋转算是一个比较复杂的变换，在三维空间中的旋转需要一个旋转轴和一个角度。仔细想想就很复杂，我们先从简单固定的旋转开始讲起：

#### 从2维固定点旋转到3维固定轴旋转

![旋转图解](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202209231208984.png)

如这个来自虎书的插图所示，我们对一个二维图像绕原点进行旋转，可以选择几个特定点进行研究，在此省略推导过程，直接给出公式：
$$
rotate(φ)=\begin{bmatrix}cosφ\quad -sinφ\\sinφ\quad\quad cosφ\end{bmatrix}
$$
同样我们也可以推广到3维。值得注意的是，3维旋转将是绕固定轴旋转，这与2维沿固定点旋转不同。我在这里直接给出绕x, y, z三根固定轴的仿射变换旋转公式：
$$
rotate-z(φ)=\begin{bmatrix}cosφ\quad -sinφ\quad0\quad0\\sinφ\quad\quad cosφ\quad0\quad0\\0\quad\quad\quad0\quad\quad\quad1\quad0\\0\quad\quad\quad0\quad\quad\quad0\quad1\end{bmatrix}
$$

$$
rotate-x(φ)=\begin{bmatrix}1\quad\quad0\quad\quad\quad\quad0\quad0\\0\quad cosφ\quad -sinφ\quad0\\0\quad sinφ\quad\quad cosφ\quad0\\0\quad\quad0\quad\quad\quad\quad0\quad1\end{bmatrix}
$$

$$
rotate-y(φ)=\begin{bmatrix}\quad cosφ\quad0\quad sinφ\quad0\\\quad0\quad\quad1\quad\quad0\quad\quad0\\-sinφ\quad0\quad cosφ\quad0\\\quad0\quad\quad0\quad\quad0\quad\quad1\end{bmatrix}
$$

需要注意，沿y轴坐标旋转的公式与另外两个公式有些许的不一样，注意不要混淆。

顺带一提，可以证明这些旋转矩阵都是正交矩阵(orthogonal matrices)，即逆矩阵与转置矩阵相等，我们可以直接调用转置矩阵来求出逆矩阵，而不是调用逆矩阵函数来增加计算开支。

```C++
	constexpr float MY_PI = 3.14159265359f;
	Transform RotateX(float theta) {
		float rad = theta / 180.0f * MY_PI;
		myEigen::Matrixf4x4 m(1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, std::cos(rad), -std::sin(rad), 0.0f,
								0.0f, std::sin(rad),  std::cos(rad), 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m, myEigen::Matrix4x4Transpose(m));
	}

	Transform RotateY(float theta) {
		float rad = theta / 180.0f * MY_PI;
		myEigen::Matrixf4x4 m(std::cos(rad), 0.0f, std::sin(rad), 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f,
								-std::sin(rad), 0.0f, std::cos(rad), 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m, myEigen::Matrix4x4Transpose(m));
	}

	Transform RotateZ(float theta) {
		float rad = theta / 180.0f * MY_PI;
		myEigen::Matrixf4x4 m(std::cos(rad), -std::sin(rad), 0.0f, 0.0f,
								std::sin(rad), std::cos(rad), 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m, myEigen::Matrix4x4Transpose(m));
	}
```

#### 绕任意轴旋转

有了以上三个固定轴旋转的公式，我们其实已经可以举一反三出任意轴的旋转了。

给定一个任意的旋转轴的单位向量w和一个固定角度α，我们可以将旋转分解为以下步骤：

1.对这个向量所在的整个坐标系映射到另一个坐标系，使得z轴与向量w共线，记录下这次旋转的矩阵和逆矩阵。

2.沿z轴，按照右手螺旋定则旋转角度α。

3.将1中记录的逆矩阵提取出来左乘变换后的坐标系，使向量恢复原位。

这么做我们确实可以做到沿任意轴旋转任意角度，不过我们有更聪明一点的办法来计算它。

### 罗德里格斯旋转公式(Rodrigues' Rotation Formula)

为了解决绕任意轴旋转的问题，我们引入了这个公式。

这个公式的思路就是：

1. 构建一个新的直角坐标系，以旋转轴为新坐标系的其中一根轴a，以原向量的垂直于旋转轴的分量为另一根轴b（同时记录下原向量平行于旋转轴的分量），两根轴叉乘获得第三根轴c。
2. 然后平行于旋转轴的分量不变，我们将垂直于旋转轴的分量在bc所在平面上旋转θ角度。
3. 最后将旋转后的这个分量与评选与旋转轴的分量合称为新的向量。

注意，我们现在讨论的是用这个公式旋转向量，实际上我们需要旋转的是坐标，我们可以将坐标与旋转轴单位向量起点连起来当做原向量来计算，将计算出来的结果向量加到旋转轴起点坐标就是我们需要得到的结果。

有了思路，我们需要将它化为公式：



<img src="https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202209231208030.png" alt="Rotate arbitrary axis" style="zoom:33%;" />

这张图来自于PBRT-v3的pdf中的图。不过好像有点错误，跟我纸质版书上同一页的图长得不太一样，大家可以自行把$\vec{v^{'}_1}$和$\vec{v_1}$、$\vec{v^{'}_2}$和$\vec{v_2}$替换一下位置

其中，$\vec{a}$为旋转轴上的单位向量，$\vec{v}$是我们需要旋转的坐标与$\vec{a}$起点相连接而成的向量，θ为我们需要旋转的角度，α为$\vec{v}$与$\vec{a}$的夹角，我们最终需要得到的旋转后的向量即是$\vec{v^{'}}$。

先将$\vec{v}$做正交分解，获得一个与$\vec{a}$共线的分量$\vec{v_c}$，和一个与$\vec{a}$垂直的分量$\vec{v_1}$。容易得到这几个向量之间的关系：
$$
\vec{v_c}=\vec{a}||\vec{v}||cosα=\vec{a}(\vec{v}\cdot\vec{a})\\
\vec{v_1}=\vec{v}-\vec{v_c}
$$
我们再将$\vec{v_1}$与$\vec{a}$做叉乘，获得与这两个向量都垂直且模长与$\vec{v_1}$相等的$\vec{v_2}$：
$$
\vec{v_2}=\vec{v_1}×\vec{a}
$$
我们接下来需要获得$\vec{v^{'}_1}$的表达式，因为$\vec{v^{'}_1}$处在以$\vec{v_1}$和$\vec{v_2}$为基的直角坐标系中，我们显然可以用后面这两个向量来表示$\vec{v^{'}_1}$，我们只需求出$\vec{v^{'}_1}$分别在$\vec{v_1}$和$\vec{v_2}$上的分量就行：
$$
\vec{v^{'}_1}=\vec{v_1}cosθ+\vec{v_2}sinθ
$$
然后我们合成新的向量，就是我们所要求的结果：
$$
\vec{v^{'}}=\vec{v_c}+\vec{v^{'}_1}
$$
到这里我们基本上已经完成整个公式的推导了，不过还差最后一步，将这个公式化成矩阵形式，并单独提取出这个变换矩阵以让我们足够写成代码。这个其实并不简单，我这里偷个懒直接把变换矩阵的公式写在下面。

想推导的可以自己推导一下，我这里给一些思路，其实在整个向量旋转中，平行于轴的分量一直是不变的，所以我们要求的旋转矩阵R就是$\vec{v_1}$转化为$\vec{v^{'}_1}$的旋转矩阵R，这样大幅简化了问题。我这里直接给出结果：
$$
R=Icosθ+(1-cosθ)\begin{bmatrix}\vec{a}.x\\\vec{a}.y\\\vec{a}.z\end{bmatrix}
\begin{bmatrix}\vec{a}.x\quad\vec{a}.y\quad\vec{a}.z\end{bmatrix}+
sinθ\begin{bmatrix}0\quad-\vec{a}.z\quad\vec{a}.y\\
\vec{a}.z\quad0\quad\quad-\vec{a}.x\\
-\vec{a}.y\quad\vec{a}.x\quad\quad0\end{bmatrix}
$$
其中I为单位矩阵，这个应该才是更多人所熟知的罗德里格斯公式。我们最后再将这个矩阵转化为仿射矩阵就行。当然，我们也可以直接将公式里的矩阵全部替换为四阶方阵：
$$
R=Icosθ+(1-cosθ)\begin{bmatrix}\vec{a}.x\\\vec{a}.y\\\vec{a}.z\\0\end{bmatrix}
\begin{bmatrix}\vec{a}.x\quad\vec{a}.y\quad\vec{a}.z\quad0\end{bmatrix}+
sinθ\begin{bmatrix}0\quad-\vec{a}.z\quad\vec{a}.y\quad0\\
\vec{a}.z\quad0\quad\quad-\vec{a}.x\quad0\\
-\vec{a}.y\quad\vec{a}.x\quad\quad0\quad0\\
0\quad\quad0\quad\quad\quad\quad0\quad1\end{bmatrix}
$$
但是如果将这个公式改写成这样的话，有一个比较严重的问题不得不被考虑到：这个公式最后的结果中，最右下角的元素可能会算出1以外的数字，这无疑会影响到我们变换出来的图形，所以我们需要时刻让这个右下角的元素为1。

```C++
	Transform Rotate(const myEigen::Vector3f& axis, float theta) {
		float rad = theta / 180.0f * MY_PI;
		float cos = std::cos(rad);
		float sin = std::sin(rad);

		myEigen::Vector4f a(axis.x, axis.y, axis.z, 0);
		if (a.Norm() != 0)a.Normalize();
		myEigen::Matrixf4x4 L(a.x * a.x, a.x * a.y, a.x * a.z, 0.0f,
								a.y * a.x, a.y * a.y, a.y * a.z, 0.0f,
								a.z * a.x, a.z * a.y, a.z * a.z, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);
		myEigen::Matrixf4x4 N(0.0f, -a.z, a.y, 0.0f,
								a.z, 0.0f, -a.x, 0.0f,
								-a.y, a.x, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);
		myEigen::Matrixf4x4 m;

		m = myEigen::Matrix4x4Identity() * cos + L * (1 - cos) + N * sin;
		m.m[3][3] = 1.0f;
		return Transform(m, myEigen::Matrix4x4Transpose(m));
	}
```

值得注意的是，尽管我们经历了那么多的运算，实际得出的旋转矩阵依旧是正交矩阵，即逆与转置相等，所以不需要再让我们的数学库大费周章地去调用逆的函数进行运算。

### 法向量变换矩阵、运算与撤销变换

在这之前，我们讨论的都是坐标的变换矩阵，但是在渲染器中，三角形的每个顶点除了会维护自己的坐标以外，还会维护这个顶点的法向量，那么法向量是否适用我们之前的变换矩阵呢？

很可惜，不符合，一个法向量跟他的顶点做了同样的矩阵变换后，这个法向量很有可能就不再是新的顶点的法向量了。但是幸运的是，我们可以得知一个顶点的切向量跟顶点做了同样的变换后依然是切向量，从这点入手我们可以推导出法向量变换矩阵和顶点变换矩阵M之间的关系。

省略推导步骤，直接给出结果，其实结果非常简单：

我这里就直接给出法向量的变换公式：
$$
\vec{v^{'}}=(M^{-1})^T\vec{n}
$$
顺带我们把之前没有写的运算符重载和撤销变换一起写了：

```C++
    Transform operator*(const Transform& t) {
        Transform ans(this->m * t.m, myEigen::Matrix4x4Inverse(this->m * t.m));
        return ans;
    }	
	template<typename T>
	myEigen::Vector3<T> Transform::operator()(const myEigen::Vector3<T>& v) const 
	{
		myEigen::Vector4<T> v1(v.x, v.y, v.z, 1);
		v1 = m * v;
		myEigen::Vector3<T> v2(v2.x, v2.y, v2.z);
		return v2;
	}
	template<typename T>
	myEigen::Vector4<T> Transform::operator()(const myEigen::Vector4<T>& v) const 
	{
		return m * v;
	}

	Transform Transform::toNormal() {
		m = myEigen::Matrix4x4Transpose(myEigen::Matrix4x4Inverse(m));
		mInv = myEigen::Matrix4x4Inverse(m);
	}

	void Transform::undo() {
		std::swap(m, mInv);
	}
```

### 坐标系变换(Coordinate Transformations)

之前我们的讨论都只是局限在对于坐标的变换，但是现在让我们下点猛料。

我们都知道相对运动的原理，有没有一种可能，我们可以让坐标固定不动，用变换整个坐标系来替代坐标变换呢？

假设我们现在在玩极限竞速地平线5或者GT7这种赛车游戏，我们怎么表示我们的车辆与城市的相对坐标变化？如果我们第一视角开车，往车窗外看去的话会发现城市在向后移动，我们车里的东西都是静止的，我们有两种表示这种情况的方法：一种是让整个城市都向后运动，赛车不动，还有一种是认为城市是相对静止的，我们整个赛车的坐标系在不断向前运动。这两种方法都是可行的，我个人感觉后一种更直观更合理一点。如果你处在一个运动中的坐标系的话，最好是让整个坐标系在不断的移动。

一个坐标系需要由一个原点和三个基向量构成，因为我们讨论的都是直角坐标系，所以三个基向量都是两两垂直的，我们在这里假设三个基向量分别为$\vec{X^{'}}$$\vec{Y^{'}}$$\vec{Z^{'}}$，而坐标系的原点为e。再假设一个坐标系为我们熟知的$\vec{X}$$\vec{Y}$$\vec{Z}$，原点为0。现在假设有一个点p，则他在两个坐标系中的表现形式为：
$$
P=(x_p,y_p,z_p)=0+x_p\vec{X}+y_p\vec{Y}+z_p\vec{Z}\\
\quad=(x^{'}_p,y^{'}_p,z^{'}_p)=e+x_p\vec{X^{'}}+y_p\vec{Y^{'}}+z_p\vec{Z^{'}}
$$
接下来推导这两个坐标的关系，其实很容易，我这里就略过直接给结果了：
$$
P_{\vec{X}\vec{Y}\vec{Z}}=\begin{bmatrix}\vec{X^{'}}\quad\vec{Y^{'}}\quad\vec{Z^{'}}\quad\vec{e}\\
0\quad0\quad0\quad1\end{bmatrix}
P_{\vec{X^{'}}\vec{Y^{'}}\vec{Z^{'}}}\\
\begin{bmatrix}\vec{X^{'}}\quad\vec{Y^{'}}\quad\vec{Z^{'}}\quad\vec{e}\\
0\quad0\quad0\quad1\end{bmatrix}=
\begin{bmatrix}1\quad0\quad0\quad x_{e}\\
0\quad1\quad0\quad y_{e}\\
0\quad0\quad1\quad z_{e}\\
0\quad0\quad0\quad1\end{bmatrix}
\begin{bmatrix} x_\vec{X^{'}}\quad x_\vec{Y^{'}}\quad x_\vec{Z^{'}}\quad 0\\
 y_\vec{X^{'}}\quad y_\vec{Y^{'}}\quad y_\vec{Z^{'}}\quad 0\\
 z_\vec{X^{'}}\quad z_\vec{Y^{'}}\quad z_\vec{Z^{'}}\quad 0\\
0\quad\quad0\quad\quad0\quad\quad1\end{bmatrix}
$$
注意这里的变换公式是哪个坐标系变换到哪个坐标系，如果反着使用的话就得使用这个矩阵的逆矩阵，这里给出代码：

```c++
	Transform WolrdToNewCoordinate(const myEigen::Vector3f& newX, const myEigen::Vector3f& newY, const myEigen::Vector3f& newZ,
		const myEigen::Vector3f& newZero)
	{
		myEigen::Vector4f NewX(newX.x, newX.y, newX.z, 0);
		myEigen::Vector4f NewY(newY.x, newY.y, newY.z, 0);
		myEigen::Vector4f NewZ(newZ.x, newZ.y, newZ.z, 0);
		myEigen::Vector4f NewZero(newZero.x, newZero.y, newZero.z, 0);
		NewX.Normalize();
		NewY.Normalize();
		NewZ.Normalize();

		myEigen::Matrixf4x4 mInv(NewX, NewY, NewZ, NewZero);
		return Transform(myEigen::Matrix4x4Inverse(mInv), mInv);
	}

	Transform NewToWorldCoordinate(const myEigen::Vector3f& newX, const myEigen::Vector3f& newY, const myEigen::Vector3f& newZ,
		const myEigen::Vector3f& newZero)
	{
		myEigen::Vector4f NewX(newX.x, newX.y, newX.z, 0);
		myEigen::Vector4f NewY(newY.x, newY.y, newY.z, 0);
		myEigen::Vector4f NewZ(newZ.x, newZ.y, newZ.z, 0);
		myEigen::Vector4f NewZero(newZero.x, newZero.y, newZero.z, 0);
		NewX.Normalize();
		NewY.Normalize();
		NewZ.Normalize();

		myEigen::Matrixf4x4 m(NewX, NewY, NewZ, NewZero);
		return Transform(m, myEigen::Matrix4x4Inverse(m));
	}
```



### 三角形类

写了这么多变换矩阵，不要忘了我们最终需要变换的是一个个的三角面，我们现在还没有涉及到纹理的读取，所以三角形类只需要存储三个顶点及其对应的法向量和颜色值。

```C++
namespace rst {
	class Triangle {
	public:
		myEigen::Vector4f vertex[3];
		myEigen::Vector4f normal[3];
		TGAColor vertexColor[3];

		Triangle();
		Triangle(myEigen::Vector3f v[3]);

		void setVertex(int index, const myEigen::Vector4f& newVert);
		void setVertex(const myEigen::Vector4f newVert[3]);
		void setNormal(int index, const myEigen::Vector4f& newNorm);
		void setNormal(const myEigen::Vector4f newNorm[3]);
		void setColor(int index, const TGAColor& newColor);
		void setColor(const TGAColor newColor[3]);
	};
}
```

我们后续到了光栅化和着色阶段都还会继续补充这个类，但现在这个阶段就这样其实足够了。

## MVP变换

掌握了上述基本变换，我们终于可以将这些变换落到实处了。

一个模型从一个三维模型变成我们屏幕上的二维图像的过程，简单来说需要两步骤：第一步就是MVP变换，第二步则是光栅化。

MVP变换是一系列矩阵变换，在这一系列矩阵变换前，我们只有一个个孤立的模型，我们先通过模型变换将这些模型像积木一样一个个搭在世界坐标下各自应该在的位置，对模型所有的仿射变换就在这一阶段；然后用一个摄像机将所有的模型“拍”进相机里，相机所有的变换都发生在这个阶段；然后经过投影进入一个正则正方体内，有可能的话能做出绘画中透视的效果；最后将3维场景映射到屏幕空间里变成2维图像；到这里MVP变换就已经结束了，之后需要考虑的就是光栅化的内容了。

MVP变换这个简写我个人不是很喜欢，实际上的MVP变换应当指的是模型变换(modeling transformation)和观测变换(viewing transformation)，而观测变换中又分为三步，分别是视图/相机变换(view/camera transformation)、投影变换(projection transformation)和视口变换(viewport transformation)。我们常说的MVP变换应该是model， view和projection的缩写，并没有把视口变换或者说屏幕映射给涵盖进来，以至于在我刚开始学的时候给我产生了很大的迷惑作用……

可以参考虎书的这张图，用图解的方式展现了一下光栅化之前模型的所有变换过程：

![MVP变换过程](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202209231208088.png)

### 模型变换(Modeling Transform)

模型变换是最简单的变换，我们需要做的仅仅是将模型从模型空间(object space)转换到世界空间(world space)下我们希望这个模型在的位置。也就是说如果我们需要对模型进行任何变换，例如旋转位移缩放等，都是在这个阶段进行。

这里需要注意的是变换的顺序，需要进行线性变换，再进行非线性变换，原理大家细想一下就能明白，如果先位移后旋转，物体位移了旋转轴没有位移，那这样出来的结果肯定跟我们预想的不太一样，犯了刻舟求剑的错误。

```C++
	Transform Modeling(const myEigen::Vector3f& translate,
		const myEigen::Vector3f& scale,
		const myEigen::Vector3f& axis, float theta)
	{
		return Translate(translate) * Rotate(axis, theta) * Scale(scale);
	}
```

### 视图/相机变换(View/Camera Transformation)

在这个变换中，我们需要将模型从世界空间(world space)转换到相机空间(camera space)，同时所有跟相机有关的变换都将在这个阶段运算。

我们先想象一个相机。在约定速成的情况下我们将相机所在的位置e作为相机空间的原点，相机看向的方向gaze direction为相机空间的-Z方向，相机本身在固定位置看向同一个方向时也可能会旋转，摆出不同的pose，我们规定一个view-up vector来表示相机“头部”指向的位置，这个view-up向量就是相机空间的Y轴方向，Y轴左乘Z轴我们就可以得到一个新的右手系。我们用$\vec{X^{'}}$$\vec{Y^{'}}$$\vec{Z^{'}}$来表示相机空间的三个坐标轴：
$$
\vec{Z^{'}}=-\frac{\vec{gaze}}{||\vec{gaze}||}\\
\vec{Y^{'}}=-\frac{\vec{viewup}}{||\vec{viewup}||}\\
\vec{X^{'}}=\vec{Y^{'}}×\vec{Z^{'}}
$$
然后我们需要把世界空间内用$\vec{X}$$\vec{Y}$$\vec{Z}$表示的物体全部转移到相机空间的新坐标系内。

正好我们之前刚讨论过坐标系变换，为了方便我故意连符号都取的一样，这里给出公式：
$$
P_{\vec{X^{'}}\vec{Y^{'}}\vec{Z^{'}}}=\begin{bmatrix}\vec{X^{'}}\quad\vec{Y^{'}}\quad\vec{Z^{'}}\quad\vec{e}\\
0\quad0\quad0\quad1\end{bmatrix}^{-1}
P_{\vec{X}\vec{Y}\vec{Z}}\\
\begin{bmatrix}\vec{X^{'}}\quad\vec{Y^{'}}\quad\vec{Z^{'}}\quad\vec{e}\\
0\quad0\quad0\quad1\end{bmatrix}=
\begin{bmatrix}1\quad0\quad0\quad x_{e}\\
0\quad1\quad0\quad y_{e}\\
0\quad0\quad1\quad z_{e}\\
0\quad0\quad0\quad1\end{bmatrix}
\begin{bmatrix} x_\vec{X^{'}}\quad x_\vec{Y^{'}}\quad x_\vec{Z^{'}}\quad 0\\
 y_\vec{X^{'}}\quad y_\vec{Y^{'}}\quad y_\vec{Z^{'}}\quad 0\\
 z_\vec{X^{'}}\quad z_\vec{Y^{'}}\quad z_\vec{Z^{'}}\quad 0\\
0\quad\quad0\quad\quad0\quad\quad1\end{bmatrix}
$$
注意这里的等式左右跟之前写的不一样，所以变换矩阵变成了逆矩阵，我们可以写出代码：

```c++
	Transform Camera(const myEigen::Vector3f& eye_pos,const myEigen::Vector3f& gaze_dir,const myEigen::Vector3f& view_up)
	{
		myEigen::Vector3f newx = myEigen::crossProduct(view_up, -gaze_dir);
		myEigen::Vector3f newy = view_up;
		myEigen::Vector3f newz = -gaze_dir;
		myEigen::Vector3f newzero = eye_pos;
		return WolrdToNewCoordinate(newx, newy, newz, newzero);
	}
```

### 投影变换(Projection Transformation)

在这个阶段，我们要将相机空间中的模型变换到一个xyz都是(-1, 1)范围内的一个正则视体(canonical view volume)中。

为什么要变换到正则视体中？因为我们需要确定相机空间中哪些模型是应该被渲染的，规定那么一个区域。那些在投影变换后没有被包含在正则视体里面的点线面就会被我们剔除。

#### 正交投影(Orthographic Projection)

最容易想到的确定相机空间中渲染的区域的方法就是直接指定一个长方体，只有在长方体内的物体会被渲染，我们可以将这个长方体映射到正则视体中，这个变换就是所谓的正交投影，指定的长方体为正交视体(orthographic view volume)

![orthographicViewVolume](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202209231208140.png)

可以参考虎书中的这幅插图，我们用l, b, n, r, t, f来表示这个正交视体，因为我们目标得到的正则视体的xyz坐标都是[-1, 1]范围内的，所以实际上只需要平移加缩放就可以将正交视体转换为正则视体。

我们使用的l, b, n, r, t, f分别是left, bottom, near, right, top, far的缩写。符号

需要注意，我们规定r>l，t>b, 不过因为我们看向的是-z坐标轴的方向，所以f的绝对值比n大，但是因为都是负数，所以n>f。
$$
M_{orth}=\begin{bmatrix}
\frac{2}{r-l}\quad0\quad0\quad-\frac{r+l}{r-l}\\
0\quad\frac{2}{t-b}\quad0\quad-\frac{t+b}{t-b}\\
0\quad0\quad\frac{2}{n-f}\quad-\frac{n+f}{n-f}\\
0\quad0\quad0\quad1\\
\end{bmatrix}
$$
根据公式我们可以写出函数：

```c++
	Transform Orthographic(float left, float bottom, float near, float right, float top, float far)
	{
		myEigen::Matrixf4x4 m(2.0f / abs(right - left), 0.0f, 0.0f, -(right + left) / abs(right - left),
			0.0f, 2.0f / abs(top - bottom), 0.0f, -(top + bottom) / abs(top - bottom),
			0.0f, 0.0f, 2.0f / abs(near - far), -(near + far) / abs(near - far),
			0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m, myEigen::Matrix4x4Inverse(m));
	}

	Transform Orthographic(const myEigen::Vector3f& lbn, const myEigen::Vector3f& rtf)
	{
		float left = lbn.x;
		float bottom = lbn.y;
		float near = lbn.z;
		float right = rtf.x;
		float top = rtf.y;
		float far = rtf.z;
		myEigen::Matrixf4x4 m(2.0f / abs(right - left), 0.0f, 0.0f, -(right + left) / abs(right - left),
			0.0f, 2.0f / abs(top - bottom), 0.0f, -(top + bottom) / abs(top - bottom),
			0.0f, 0.0f, 2.0f / abs(near - far), -(near + far) / abs(near - far),
			0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m, myEigen::Matrix4x4Inverse(m));
	}
```

但是正交投影其实是不符合现实生活规律的，我们真实的眼睛实际上做的是透视投影的过程。

#### 透视投影(Perspective Projection)

如果有过绘画经验或者摄影经验的人都会明白一个道理，所谓的平行线在我们的眼睛里是不存在的。

我们可以想象一个铁轨，铁轨的两侧百分之一百是平行的，但是当我们望向地平线时，铁轨的两侧总会交汇于一个看起来似乎无穷远的地方。或者说仔细想想近大远小的概念，正交投影会对视体内所有的物体一视同仁地缩放和平移，显然是没有哪个步骤能做出近大远小的效果的。

这是因为我们的眼睛接收光线的区域并不是像正交视体那样是一个长方体，实际上我们能看到的区域是一个锥形区域，被称为视椎体，这么一个锥形区域将会首先变换为一个正交视体，然后再变换为一个正则视体。可想而知，视椎体内的一切这个过程中都会发生一定程度的“变形”，因为是椎体变换为长方体，自然远处的物体会被“压缩”地更厉害，平行线也会在越远的地方偏离的越夸张，最后两条平行线会在一个比较远的地方汇为一点，这个点在绘画上称之为灭点。

当然这种“视椎体”其实也是一种近似，人眼非常复杂，其中人的眼角余光并不是很遵循这个规则。在绘画上有一种称之为曲线透视的概念，又被叫做鱼眼镜头，就是模拟再现人的眼角余光而产生的。不过在这一节中，我们只复现视椎体和直线透视的效果。

![MVP变换过程](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202209231208088.png)

我们再回来看一下这张图，看其中camera space的图就可以大概感受到视椎体的概念。

这个视椎体显然不会是一成不变的，我们可以改变视椎体的形状。首先就是从侧面看过去其实这个所谓的视椎体并不是一个锥体，从侧面的投影可以看出来这其实是一个四棱台，这看起来就像是视椎体被人从中间截断了，同时视椎体显然也不是无限延伸的，到一定距离后就不会继续延伸。也就是说，我们实际上给了一个限制。这就引入了近剔除平面n和远剔除平面f的概念，只有z坐标在n和f之间的物体才会被渲染，太远或者太近都不会被渲染。

同时由于这个椎体原点就是相机空间的原点，所以我们可以调节我们的垂直可视角度来同时调整近远平面的大小，这个垂直可视角度被我们称作fov-Y，也就是Field Of View的缩写，有些时候可能会选择平行可视角度来决定视野的大小，但是一般情况下我们都使用垂直可视角度。有了垂直可视角度，我们还需要一个近平面的长宽比aspect，这样我们就可以完全地确定一个椎体的形状了。

现在我们需要考虑如何将视椎体变换为正交视体。

我们将近剔除平面直接当成是正交视体的其中一个面，剩下的工作就是将后面的区域给“挤压”到近平面的大小，这样我们才能得到一个长方体。

这个变换矩阵的推导有很多案例，也比较复杂，我简单说下闫令琪老师说的办法：

因为是“挤压”，我们可以直观地想象，离近平面越远的坐标被挤压的越厉害，也就是说，他们坐标的变换肯定会受到z值的影响，我们可以做几个横截面，利用普通的相似三角形原理就可以做出以下公式：
$$
P^{'}=\begin{bmatrix}nx/z\\ny/z\\?\\1\end{bmatrix}=\begin{bmatrix}nx\\ny\\?\\z\end{bmatrix}
$$
这样我们可以一窥透视矩阵除了第三行外的全貌，再然后我们在近平面z值不变和远平面z值不变的前提下列出一个方程组即可求出第三行的值，这里直接给出结果：
$$
M_{perspective}=\begin{bmatrix}
n\quad\quad0\quad\quad0\quad\quad0\\
0\quad\quad n\quad\quad0\quad\quad0\\
0\quad0\quad n+f\quad-fn\\
0\quad\quad0\quad\quad1\quad\quad0
\end{bmatrix}
$$
可以看到这个变换矩阵不属于线性变换，注意这个矩阵的第四行是0 0 1 0而不是我们熟知的0 0 0 1。

这意味着每个做了这个变换后的点的坐标的w值都为z而不是1，所以我们得出的结果坐标并不能直接用，还需要将四个点坐标都除以w的值(w=z)将w归一化后才能继续下一步。

同时我们来看一个正常坐标乘以这个矩阵后的结果：
$$
P\begin{bmatrix}x\\y\\z\\1\end{bmatrix}=
\begin{bmatrix}nx\\ny\\(n+f)z-fn\\z\end{bmatrix}=>
\begin{bmatrix}\frac{nx}{z}\\\frac{ny}{z}\\n+f-\frac{fn}{z}\\1\end{bmatrix}
$$
我们会发现z坐标会有些微的偏移，但是我们后续的变换中其实不会再用到z做什么重要工作，只需要z坐标的大小顺序不要乱，让它在计算遮挡的时候再最后发挥一次作用就行，所以就像虎书里说的，我们这里带z坐标仅仅是"along to the ride"。

接下来，经过了透视矩阵，我们的视椎体已经被我们压缩成了一个正交视体，我们只需要对这个正交视体做一次正交投影即可。

这里需要算一算变换后的正交视体的参数(l, b, n, r, t, f)，因为我们的视椎体的顶点在相机空间原点，所以我们看向的视椎体一定是相对-z轴轴对称的，所以：
$$
r=-l=aspect*t\\
t=-b=tan\frac{fovY}{2}|n|\\
$$
而n和f都是已知量，可以写出代码：

```C++
	Transform Perspective(float zneardis, float zfardis, float fovY, float aspect)
	{
		float n = -zneardis;
		float f = -zfardis;
		float ffovY = fovY / 180.0 * MY_PI;
		
		myEigen::Matrixf4x4 m(n, 0.0f, 0.0f, 0.0f,
			0.0f, n, 0.0f, 0.0f,
			0.0f, 0.0f, n + f, -f * n,
			0.0f, 0.0f, 1.0f, 0.0f);
		Transform per(m);

		float t = std::tan(ffovY / 2) * abs(n);
		float b = -t;
		float r = aspect * t;
		float l = -r;
		return Orthographic(l, b, -zneardis, r, t, -zfardis) * per;
	}
```

### 视口变换(Viewport Transformation)

终于到了最后一步了，我们需要将正则视体里的所有可渲染的物体都变换到以左下角为(0, 0)点的屏幕空间内。

在这次变换中，由于我们是从三维空间映射到二维空间，所以z值与这次变换无关，该有的近大远小效果都在透视投影中解决了。

我们需要知道屏幕空间的信息，也就是我们输出的屏幕的高度和宽度是多少，知道这些就够了，我们会将正则视体内的坐标都经过缩放和平移成比例地映射到屏幕空间。

假设屏幕空间高度为h，宽度为w：
$$
M_{viewport}=\begin{bmatrix}\frac{w}{2}\quad0\quad0\quad\frac{w}{2}\\
0\quad\frac{h}{2}\quad0\quad\frac{h}{2}\\
0\quad0\quad1\quad0\\
0\quad0\quad0\quad1
\end{bmatrix}
$$
实际上就是每个坐标在xy平面上先缩放到屏幕空间的大小，再将左下角平移到(0, 0)点。

简单写出代码：

```c++
	Transform Viewport(float width, float height)
	{
		myEigen::Matrixf4x4 m(width / 2.0f, 0.0f, 0.0f, width / 2.0f,
			0.0f, height / 2.0f, 0.0f, height / 2.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m);
	}
```

## 简单光栅化

到此MVP变换的部分已经全部结束了，接下来要将视口变换的结果给显示到屏幕上，我们需要光栅化三角形。

在下一节中我们会讨论光栅化的基本步骤，但是在这节中为了能够看到我们MVP变换的成功，我们快速写一份简单的三角形光栅化的代码：

```c++
void rst::rasterizer::draw()
{
	Transform mvp = Perspective(zneardis, zfardis, fovY, aspect)
		* Camera(eye_pos, gaze_dir, view_up) * Modeling(myEigen::Vector3f(0),
			myEigen::Vector3f(1),
			myEigen::Vector3f(rotateAxis), theta);
	Transform viewport = Viewport(width, height);

	for (size_t i = 0; i < 3; i++)
	{
		m.t.vertex[i] = mvp(m.t.vertex[i]);
		m.t.vertex[i] /= m.t.vertex[i].w;
		m.t.vertex[i] = viewport(m.t.vertex[i]);
	}

	m.t.setColor(0, TGAColor(255, 0, 0, 0));
	m.t.setColor(1, TGAColor(0, 255, 0, 0));
	m.t.setColor(2, TGAColor(0, 0, 255, 0));

	rasterize_wireframe(m);
}

void rst::rasterizer::rasterize_wireframe(const model& m)
{
	myEigen::Vector2i a(m.t.vertex[0].x, m.t.vertex[0].y);
	myEigen::Vector2i b(m.t.vertex[1].x, m.t.vertex[1].y);
	myEigen::Vector2i c(m.t.vertex[2].x, m.t.vertex[2].y);

	draw_line(a, b);
	draw_line(b, c);
	draw_line(c, a);
}
```

## Hello, triangles!

我顺带修改了一下main函数，现在可一次性输出多张图，并计算渲染时长了：

```c++
int main(int argc, char** argv) {

    uint16_t frame = 0;
    auto begin = std::chrono::steady_clock::now();
    float angle = 0;

    while (true)
    {
        std::string filename = "result\\output" + std::to_string(frame);
        
        TGAImage image(700, 700, TGAImage::RGB);
        rst::rasterizer rst(filename, image);
        rst.SetCamera(myEigen::Vector3f(0, 0, 5));
        rst.SetTheta(angle);
        rst.SetRotateAxis(myEigen::Vector3f(1, 1, 0));
        rst.draw();
        rst.output();
        cout << "frame:" << frame << endl;
        frame++;
        angle += 5;
        if (frame > 100) {
            auto end = std::chrono::steady_clock::now();
            cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
            return 0;
        }
    }
    auto end = std::chrono::steady_clock::now();
    cout << "用时" << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin) << endl;
    return 0;
}
```

渲染100张图，我测试了很多次，电脑不摸鱼的时候平均花费5秒左右。

让我们试试把debug改成release：

![image-20220923010834780](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202209231208190.png)

我超！0.58秒，为什么说编译器是神……大概算算每秒172帧，俗话说一帧能玩两帧电竞，这已经够把咱们送上csgo职业赛场86次了。

让我们打开输出的文件夹，来看看这个三角形绕(1,1,0)旋转的结果：

（注：输出的是tga图片序列，这个动图是我开了adobe家软件自己做的……目前我们的光栅器还不能输出动图，估计以后也不会有这个功能。）

![](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202209231208242.gif)

！成功了

看起来旋转地很对，我们经历了千难万险，终于成功画出了我们的三角形。

不得不感慨一局，CG方向画个三角形都要上千行代码，太扯淡了。不过不管怎么样，我们终于可以喊出一句：

Hello, triangles!

下一节我们将讲述一下三角形光栅化的内容，包括Z-buffer计算遮挡，抗锯齿，线性插值等，我们在这一节里实现的只是一个能跑就谢天谢地的光栅化，显然是不堪大用的。

