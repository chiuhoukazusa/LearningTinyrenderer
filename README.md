# 02.简单3D数学库|封装向量类和矩阵类

## 本项目代码已托管至github，将会随着博客实时更新进度

每一节的工程我都会创建一个新的分支，分支名由这一节的数字决定。

https://github.com/chiuhoukazusa/LearningTinyrenderer/tree/02

## 前言

在上一节的工程中，给定两个点，我们已经做到成功画出一条唯一确定的线段了。但是为了我们以后方便，我们显然需要封装向量类和矩阵类。当然我们可以直接使用Eigen库来完成这个问题，Eigen库非常的好用，但是我更想自己写一个试试。我们要做的其实完全不复杂，仅仅是封装几个类，并且没有什么性能要求，不会涉及到使用一些指令集来进行加速。所有的一切都将只用一些比较简单的c++代码进行实现。

本节代码参考了一部分pbrt-v3的配套代码中的geometry.h文件中的实现方式和DirectXMath库的设计方式，来封装一些简单的涉及3d数学的类。以下是pbrt-v3的源码：

https://github.com/mmp/pbrt-v3 

## 我们需要做的

pbrt-v3中的数学库实现相当细致，不仅仅只是封装了向量和矩阵，而是将点、法线等等也跟向量区分了开来。

这也算是必要的工作，因为这样就从根源上排除了一些非法操作，例如将点与点相乘等。同时pbrt-v3还封装了Transform类，里面会大量使用到点、向量等数学运算，因此pbrt-v3这样做是非常合适的。

而我们暂时只封装向量和矩阵类，为了简单而言。因为我们正式的一些渲染流程都还没写，在这种情况下死磕数学库我认为不是一个理智的决定，而且我们也没有使用到SIMD指令集来加速。因此我的构思是先用着，等后续有机会了会再来一节3d数学库的完善和重构，届时我会尽量细致地学习并编写一个好用的高性能的3d数学库，当前的目标则是“能用就行”。

## 向量（以Vector3为例）

相比起矩阵来说，向量无疑算是比较简单的。以三维向量为例，我们先定义一个模板，顺带写几个构造函数：

```c++
namespace myEigen {
	template<typename T>
	class Vector3 {
	public:
		T x, y, z;
		Vector3():x(0), y(0), z(0){}
		Vector3(T x):x(x), y(x), z(x){}
		Vector3(T x, T y, T z) :x(x), y(y), z(z) {}
	};
}
```

### 向量的运算符重载与函数

我们的下一步就是重载运算符，将原本用于标量的加减乘除重载为适用于向量的形式。

同样也很简单，就不用复习一遍公式了，直接写，先从加减开始：

```c++
		Vector3<T>* operator=(const Vector3<T>& v) {
			x = v.x, y = v.y, z = v.z;
			return *this;
		}
		Vector3<T> operator+(const Vector3<T>& v) const {
			return Vector3(x + v.x, y + v.y, z + v.z);
		}
		Vector3<T>& operator+=(const Vector3<T>& v) {
			x += v.x, y += v.y, z += v.z;
			return *this;
		}
		Vector3<T> operator-(const Vector3<T>& v) const {
			return Vector3(x - v.x, y - v.y, z - v.z);
		}
		Vector3<T>& operator-=(const Vector3<T>& v) {
			x -= v.x, y -= v.y, z -= v.z;
			return *this;
		}
		Vector3<T> operator-() const {
			return Vector3<T>(-x, -y, -z);
		}
```

也比较简单，不过从乘除开始情况就稍微有点不一样了。

向量乘法有点积和叉积，那么我们要重载的是哪一种乘法？

哪种都不是，我们将直接把两个向量的各个分量一一对应分别相乘为一个新的向量的分量。这听起来很奇怪，但是这样的运算在后续中将会用到很多次，最常见的就是我们将颜色进行混合时，两个颜色以这种方式相乘的结果就是ps里的正片叠底。

而且不仅仅只有这一种情况，向量不一定会跟向量相乘，如果向量跟一个标量相乘，那么结果也不一样。记住这几点就可以写出代码了，顺便为了方便再把平方、取长度和归一化也一起写了：

```c++
		template<typename U>
		Vector3<T> operator*(U s) const {
			return Vector3<T>(x * s, y * s, z * s);
		}
		template<>
		Vector3<T> operator*(const Vector3<T>& v) const {
			return Vector3<T>(x * v.x, y * v.y, z * v.z);
		}
		template<typename U>
		Vector3<T>& operator*=(U s) {
			x *= s, y *= s, z *= s;
			return *this;
		}
		template<typename U>
		Vector3<T> operator/(U s) const {
			return Vector3<T>(x / s, y / s, z / s);
		}
		template<>
		Vector3<T> operator/(Vector3<T> v) const {
			return Vector3<T>(x / v.x, y / v.y, z / v.z);
		}
		template<typename U>
		Vector3<T>& operator/=(U s) {
			x /= s, y /= s, z /= s;
			return *this;
		}
		float Norm() const {
			return x * x + y * y + z * z;
		}
		float Length() const {
			return std::sqrt(Norm());
		}
		Vector3<T>& Normalize() {
			operator/=(Norm());
			return *this;
		}
```

但是这还没有完成，仔细思考一下会发现我们重载的乘法运算符只能用于向量乘以标量的情况。但如果只是将二者调换一下，让标量乘以向量，这在数学上完全说得通，但是在我们的代码里就不行，所以我们需要定义一个友元。以及<<运算符的重载也需要在友元中进行，所以我们一起写了：

```c++
		friend std::ostream& operator<<(std::ostream& os, const Vector3<T>& v) {
			return os << v.x << "," << v.y << "," << v.z;
		}
		friend Vector3<T> operator*(const T& r, const Vector3<T>& v) {
			return Vector3<T> * r;
		}
```

我们依旧还没有写点积和叉积，这些我们将在类的外部定义，同时我们还需要定义一下插值算法，这个我们后期也会用到很多次，简单写一下这三个函数：

```c++
	template<typename T>
	inline T dotProduct(const Vector2<T>& a, const Vector2<T>& b) {
		return (a.x * b.x + a.y * b.y);
	}

	template<typename T>
	inline Vector3<T> crossProduct(const Vector3<T>& a, const Vector3<T>& b) {
		return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
	}

	template<typename T>
	inline Vector3<T> lerp(const Vector3<T>& a, const Vector3<T>& b, const float& t) {
		return a * (1 - t) + b * t;
	}
```

### 收尾工作

最后为了方便我们日后使用，我们使用typedef定义两种不同的向量形式：

```C++
	typedef Vector3<float> Vector3f;
	typedef Vector3<int> Vector3i;
```

至此，我们已经写了一个比较完整的三维向量类。我们完全可以将同样的思路放在二维向量和四维向量的封装之中，在此由于篇幅原因不表，记得写一个toVector4(Vector3 v)的函数，给三维到四维函数一个转换。

写完后可以直接修改上一节的drawline()方法，然后进行测试，看看结果是否和预期一致。

## 矩阵

矩阵其实非常复杂，我翻了翻PBRTv3中的transform.h，发现他好像只实现了4x4的矩阵。

如果把什么二阶方阵和三阶方阵全都实现一遍的话可能有点多余，我暂时想到会用到矩阵的地方就是仿射变换的时候，所以我们暂时先只实现Martrix4x4就够了。而且也用不上模板，我们只使用martrixf4x4就够了（事实证明打脸了，继续往下看下去就知道了）。

还是跟之前向量一样的思路：

```c++
	class Matrixf4x4 {
	public:
		float m[4][4];
				Matrixf4x4(){
			m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.f;
			m[0][1] = m[0][2] = m[0][3] = 
			m[1][0] = m[1][2] = m[1][3] = 
			m[2][0] = m[2][1] = m[2][3] =
			m[3][0] = m[3][1] = m[3][2] = 0.f;
		}
		Matrixf4x4(float mat[4][4]) { memcpy(m, mat, 16 * sizeof(float)); }
		Matrixf4x4(float t00, float t01, float t02, float t03,
			float t10, float t11, float t12, float t13,
			float t20, float t21, float t22, float t23,
			float t30, float t31, float t32, float t33)
		{
			m[0][0] = t00, m[0][1] = t01, m[0][2] = t02, m[0][3] = t03;
			m[1][0] = t10, m[1][1] = t11, m[1][2] = t12, m[1][3] = t13;
			m[2][0] = t20, m[2][1] = t21, m[2][2] = t22, m[2][3] = t23;
			m[3][0] = t30, m[3][1] = t31, m[3][2] = t32, m[3][3] = t33;
		}
		template<typename T>
		Matrixf4x4(Vector4<T> v0, Vector4<T> v1, Vector4<T> v2, Vector4<T> v3) {
			m[0][0] = v0.x, m[0][1] = v1.x, m[0][2] = v2.x, m[0][3] = v3.x;
			m[1][0] = v0.y, m[1][1] = v1.y, m[1][2] = v2.y, m[1][3] = v3.y;
			m[2][0] = v0.z, m[2][1] = v1.z, m[2][2] = v2.z, m[2][3] = v3.z;
			m[3][0] = v0.w, m[3][1] = v1.w, m[3][2] = v2.w, m[3][3] = v3.w;
		}
	};
```

值得注意的是用四个四维向量构造矩阵的思路，这里我们默认我们的向量是列向量。以后的文章里，如无特殊说明，我们说的向量也都是列向量。

### 矩阵的运算符重载

矩阵的运算比较复杂，大部分需要写成函数形式，需要重载的运算符只有*和<<。

我们先回顾一下矩阵与向量，矩阵与矩阵的乘法公式：

假设现在有两个4阶方阵A和B，假设结果为T，则
$$
T_{nm}=a_{n1}*b_{1m}+a_{n2}*b_{2m}+a_{n3}*b_{3m}+a_{n4}*b_{4m}
$$
对于4阶方阵A左乘4维列向量B，有
$$
AB =\begin{bmatrix}A_{11}\quad A_{12}\quad A_{13}\quad A_{14}\\
A_{21\quad }A_{22}\quad A_{23}\quad A_{24}\\
A_{31\quad }A_{32}\quad A_{33}\quad A_{34}\\
A_{41\quad }A_{42}\quad A_{43}\quad A_{44}
\end{bmatrix}\begin{bmatrix}x\\
y\\
z\\
w
\end{bmatrix}
=
\begin{bmatrix}A_{11}x+A_{12}y+A_{13}z+A_{14}w\\
A_{21}x+A_{22}y+A_{23}z+A_{24}w\\
A_{31}x+A_{32}y+A_{33}z+A_{34}w\\
A_{41}x+A_{42}y+A_{43}z+A_{44}w\end{bmatrix}
$$
那么我们可以写出重载：

```c++
		template<typename T>
		Matrixf4x4 operator*(const Vector4<T>& v) const {
			return Vector4<T>(m[0][0] * v.x, m[0][1] * v.y, m[0][2] * v.z, m[0][3] * v.w,
				m[1][0] * v.x, m[1][1] * v.y, m[1][2] * v.z, m[1][3] * v.w,
				m[2][0] * v.x, m[2][1] * v.y, m[2][2] * v.z, m[2][3] * v.w,
				m[3][0] * v.x, m[3][1] * v.y, m[3][2] * v.z, m[3][3] * v.w);
		}

		Matrixf4x4 operator*(const Matrixf4x4& m) const {
			Matrixf4x4 ans;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					ans.m[i][j] = this->m[i][0] * m.m[0][j] +
						this->m[i][1] * m.m[1][j] +
						this->m[i][2] * m.m[2][j] +
						this->m[i][3] * m.m[3][j];
				}
			}
			return ans;
		}

		Matrixf4x4 operator*(float s) const {
			Matrixf4x4 mat;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					mat.m[i][j] = m[i][j] * s;
				}
			}
			return mat;
		}

		friend std::ostream& operator<<(std::ostream& os, const Matrixf4x4& v) {
			return os << v.m[0][0] << " " << v.m[0][1] << " " << v.m[0][2] << " " << v.m[0][3] << std::endl
				<< v.m[1][0] << " " << v.m[1][1] << " " << v.m[1][2] << " " << v.m[1][3] << std::endl
				<< v.m[2][0] << " " << v.m[2][1] << " " << v.m[2][2] << " " << v.m[2][3] << std::endl
				<< v.m[3][0] << " " << v.m[3][1] << " " << v.m[3][2] << " " << v.m[3][3] << std::endl;
		}
```

### 矩阵的函数

我们需要设计几种矩阵的函数以供我们之后的工作使用。不过我们不可能把矩阵所有的运算都写出来，这就要求我们必须有所取舍。

我这里参考了DirectXMath库的方案，主要将会定义矩阵的转置、逆、行列式这几个用于计算的函数以及返回单位矩阵的函数。

### 转置和返回单位矩阵

我们先从最简单的开始，转置无非就是把矩阵里的元素的行列下标对换：

```c++
	inline Matrixf4x4 Matrix4x4Transpose(const Matrixf4x4& m) {
		Matrixf4x4 mat(m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
			m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
			m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
			m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]);
		return mat;
	}
```

似乎毫无难度。。。那买一送一再附赠一个吧

```
	inline Matrixf4x4 Matrix4x4Identity() {
		Matrixf4x4 m;
		return m;
	}
```

### 行列式

四阶矩阵的行列式比较复杂，如果是二阶或者三阶方阵，那我们很快就能给出结果，可惜我们现在需要硬写四阶方阵的行列式。
$$
det\begin{bmatrix}A_{11}\quad A_{12}\quad A_{13}\quad A_{14}\\
A_{21\quad }A_{22}\quad A_{23}\quad A_{24}\\
A_{31\quad }A_{32}\quad A_{33}\quad A_{34}\\
A_{41\quad }A_{42}\quad A_{43}\quad A_{44}
\end{bmatrix}\ = A_{11}det\begin{bmatrix}
A_{22}\quad A_{23}\quad A_{24}\\
A_{32}\quad A_{33}\quad A_{34}\\
A_{42}\quad A_{43}\quad A_{44}
\end{bmatrix}\ \\+ A_{12}det\begin{bmatrix}
A_{21}\quad A_{23}\quad A_{24}\\
A_{31}\quad A_{33}\quad A_{34}\\
A_{41}\quad A_{43}\quad A_{44}
\end{bmatrix}\ + A_{13}det\begin{bmatrix}
A_{21}\quad A_{22}\quad A_{24}\\
A_{31}\quad A_{32}\quad A_{34}\\
A_{41}\quad A_{42}\quad A_{44}
\end{bmatrix}\ + A_{14}det\begin{bmatrix}
A_{21}\quad A_{22}\quad A_{23}\\
A_{31}\quad A_{32}\quad A_{33}\\
A_{41}\quad A_{42}\quad A_{43}
\end{bmatrix}\
$$
而三阶方阵可以直接使用公式：
$$
det\begin{bmatrix}A_{11}\quad A_{12}\quad A_{13}\\
A_{21\quad }A_{22}\quad A_{23}\\
A_{31\quad }A_{32}\quad A_{33}\\
\end{bmatrix}\ = A_{11}A_{22}A_{33} + A_{12}A_{23}A_{31} + A_{13}A_{21}A_{32}- \\A_{13}A_{22}A_{31} - A_{12}A_{21}A_{33} - A_{11}A_{23}A_{32}
$$
然而我们并没有定义三阶方阵，为了拯救我们的键盘，只能临时赶紧定义一个：

```C++
	class Matrixf3x3 {
	public:
		float m[3][3];
		Matrixf3x3() {
			m[0][0] = m[1][1] = m[2][2] = 1.f;
			m[0][1] = m[0][2] =
				m[1][0] = m[1][2] =
				m[2][0] = m[2][1] = 0.f;
		}
		Matrixf3x3(float mat[3][3]) { memcpy(m, mat, 9 * sizeof(float)); }
		Matrixf3x3(float t00, float t01, float t02,
			float t10, float t11, float t12,
			float t20, float t21, float t22)
		{
			m[0][0] = t00, m[0][1] = t01, m[0][2] = t02;
			m[1][0] = t10, m[1][1] = t11, m[1][2] = t12;
			m[2][0] = t20, m[2][1] = t21, m[2][2] = t22;
		}
		template<typename T>
		Matrixf3x3(Vector3<T> v0, Vector3<T> v1, Vector3<T> v2, Vector3<T> v3) {
			m[0][0] = v0.x, m[0][1] = v1.x, m[0][2] = v2.x;
			m[1][0] = v0.y, m[1][1] = v1.y, m[1][2] = v2.y;
			m[2][0] = v0.z, m[2][1] = v1.z, m[2][2] = v2.z;
		}

		template<typename T>
		Matrixf3x3 operator*(const Vector3<T>& v) const {
			return Vector3<T>(m[0][0] * v.x, m[0][1] * v.y, m[0][2] * v.z,
				m[1][0] * v.x, m[1][1] * v.y, m[1][2] * v.z,
				m[2][0] * v.x, m[2][1] * v.y, m[2][2] * v.z);
		}

		Matrixf3x3 operator*(const Matrixf3x3& m) const {
			Matrixf3x3 ans;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					ans.m[i][j] = this->m[i][0] * m.m[0][j] +
						this->m[i][1] * m.m[1][j] +
						this->m[i][2] * m.m[2][j];
				}
			}
			return ans;
		}

		Matrixf3x3 operator*(float s) const {
			Matrixf3x3 mat;
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					mat.m[i][j] = m[i][j] * s;
				}
			}
			return mat;
		}

		friend std::ostream& operator<<(std::ostream& os, const Matrixf3x3& v) {
			return os << v.m[0][0] << " " << v.m[0][1] << " " << v.m[0][2] << std::endl
				<< v.m[1][0] << " " << v.m[1][1] << " " << v.m[1][2] << std::endl
				<< v.m[2][0] << " " << v.m[2][1] << " " << v.m[2][2] << std::endl;
		}
	};

	inline float Matrix3x3Determinant(const Matrixf3x3& m) {
		return m.m[0][0] * m.m[1][1] * m.m[2][2] +
			m.m[0][1] * m.m[1][2] * m.m[2][0] +
			m.m[0][2] * m.m[1][0] * m.m[2][1] -
			m.m[0][2] * m.m[1][1] * m.m[2][1] -
			m.m[0][1] * m.m[1][0] * m.m[2][2] -
			m.m[0][0] * m.m[1][2] * m.m[2][1];

	}
```

基本上都是复制粘贴，重点是最后那个行列式函数，有了这个三阶方阵的行列式函数，我们就可以轻松写出四阶的行列式函数。

但是先别急，我们再写一个函数用以返回4阶方阵的余子阵：

```C++
	inline Matrixf3x3 Matrix4x4CMinor(const Matrixf4x4& mat, uint16_t row, uint16_t col) {
		float m[3][3];
		int m_row = 0;
		for (int i = 0; i < 4; i++) {
			if (i == row) { continue; }
			int m_col = 0;
			for (int j = 0; j < 4; j++) {
				if (j == col) { continue; }
				m[m_row][m_col] = mat.m[i][j];
				m_col++;
			}
			m_row++;
		}
		return Matrixf3x3(m);
	}
```

这里千万注意，别把行跟列搞反了，我们传进来的是这个余子阵的元素所在的行row和列col，但删去的却是第col行第row列的元素，这里有个小坑别填反了。

然后我们就可以求矩阵的行列式了：

```C++
	inline float Matrix4x4Determinant(const Matrixf4x4& m) {
		return m.m[0][0] * Matrix3x3Determinant(Matrix4x4CMinor(m, 0, 0)) +
			m.m[0][1] * Matrix3x3Determinant(Matrix4x4CMinor(m, 0, 1)) +
			m.m[0][2] * Matrix3x3Determinant(Matrix4x4CMinor(m, 0, 2)) +
			m.m[0][3] * Matrix3x3Determinant(Matrix4x4CMinor(m, 0, 3));
	}
```

### 逆

终于到了最后一关，求矩阵的逆。

矩阵的逆有很多种求法，最基础的就是伴随矩阵除以行列式的求法。值得一提的是，pbrt-v3使用的是全主元消去法求矩阵逆，同时代码写的很好，我看了几遍下来没有发现它的算法有使用到额外空间来存储单位矩阵。

不过我们还是从最简单的伴随矩阵除以行列式的算法来做。

行列式已经解决了，让我们来看看伴随矩阵如何计算：
$$
A^*=C^T_A=\begin{bmatrix}C_{11}\quad C_{12}\quad C_{13}\quad C_{14}\\
C_{21\quad }C_{22}\quad C_{23}\quad C_{24}\\
C_{31\quad }C_{32}\quad C_{33}\quad C_{34}\\
C_{41\quad }C_{42}\quad C_{43}\quad C_{44}
\end{bmatrix}^T
$$

$$
C_{ij}=(-1)^{i+j}det\overline{A}_{ij}
$$

这些公式所需要的转置函数和余子式函数都已经被我们写好了。可以说我们之前封装了那么多函数，就是为了这一刻：

```c++
	inline Matrixf4x4 Matrix4x4Inverse(const Matrixf4x4& m) {
		float DetM = Matrix4x4Determinant(m);
		float InverseDetM = 1.0f / DetM;
		Matrixf4x4 CofMat;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				CofMat.m[i][j] = pow(-1, i + j) * Matrix3x3Determinant(Matrix4x4CMinor(m, i, j));
			}
		}
		return Matrix4x4Transpose(CofMat * InverseDetM);
	}
```

好，到此为止，我自己测试了几个用例，结果都是正确的，那么我们这个数学库也就基本完成了，也就是说起码到了“可以用”的级别。

下一节我们将会开始MVP变换，届时将真正考验我们的数学库的正确性。
