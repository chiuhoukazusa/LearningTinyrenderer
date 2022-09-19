#pragma once
#include <iostream>
#include <math.h>


namespace myEigen {
	template<typename T>
	class Vector4 {
	public:
		T x, y, z, w;
		Vector4() :x(0), y(0), z(0), w(0) {}
		Vector4(T x) :x(x), y(x), z(x), w(x) {}
		Vector4(T x, T y, T z, T w) :x(x), y(y), z(z), w(w) {}
		Vector4<T>* operator=(const Vector4<T>& v) {
			x = v.x, y = v.y, z = v.z, w = v.w;
			return *this;
		}
		Vector4<T> operator+(const Vector4<T>& v) const {
			return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
		}
		Vector4<T>& operator+=(const Vector4<T>& v) {
			x += v.x, y += v.y, z += v.z, w += v.w;
			return *this;
		}
		Vector4<T> operator-(const Vector4<T>& v) const {
			return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
		}
		Vector4<T>& operator-=(const Vector4<T>& v) {
			x -= v.x, y -= v.y, z -= v.z, w -= v.w;
			return *this;
		}
		Vector4<T> operator-() const {
			return Vector4<T>(-x, -y, -z, -w);
		}
		template<typename U>
		Vector4<T> operator*(U s) const {
			return Vector4<T>(x * s, y * s, z * s, w * s);
		}
		template<>
		Vector4<T> operator*(const Vector4<T>& v) const {
			return Vector3<T>(x * v.x, y * v.y, z * v.z, w * v.w);
		}
		template<typename U>
		Vector4<T>& operator*=(U s) {
			x *= s, y *= s, z *= s, w *= s;
			return *this;
		}
		template<typename U>
		Vector4<T> operator/(U s) const {
			return Vector4<T>(x / s, y / s, z / s, w / s);
		}
		template<>
		Vector4<T> operator/(Vector4<T> v) const {
			return Vector4<T>(x / v.x, y / v.y, z / v.z, w / v.w);
		}
		template<typename U>
		Vector4<T>& operator/=(U s) {
			x /= s, y /= s, z /= s, w /= s;
			return *this;
		}
		float Norm() const {
			return x * x + y * y + z * z + w * w;
		}
		float Length() const {
			return std::sqrt(Norm());
		}
		Vector4<T>& Normalize() {
			operator/=(Norm());
			return *this;
		}
		friend std::ostream& operator<<(std::ostream& os, const Vector4<T>& v) {
			return os << v.x << "," << v.y << "," << v.z << "," << v.w;
		}
		friend Vector4<T> operator*(const T& r, const Vector4<T>& v) {
			return Vector4<T> * r;
		}
	};

	template<typename T>
	inline T dotProduct(const Vector4<T>& a, const Vector4<T>& b) {
		return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
	}

	template<typename T>
	inline Vector4<T> lerp(const Vector4<T>& a, const Vector4<T>& b, const float& t) {
		return a * (1 - t) + b * t;
	}

	typedef Vector4<float> Vector4f;
	typedef Vector4<int> Vector4i;

	template<typename T>
	class Vector3 {
	public:
		T x, y, z;
		Vector3():x(0), y(0), z(0){}
		Vector3(T x):x(x), y(x), z(x){}
		Vector3(T x, T y, T z) :x(x), y(y), z(z) {}
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
		friend std::ostream& operator<<(std::ostream& os, const Vector3<T>& v) {
			return os << v.x << "," << v.y << "," << v.z;
		}
		friend Vector3<T> operator*(const T& r, const Vector3<T>& v) {
			return Vector3<T> * r;
		}
	};
	
	template<typename T>
	inline T dotProduct(const Vector3<T>& a, const Vector3<T>& b) {
		return (a.x * b.x + a.y * b.y + a.z * b.z);
	}

	template<typename T>
	inline Vector3<T> crossProduct(const Vector3<T>& a, const Vector3<T>& b) {
		return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
	}

	template<typename T>
	inline Vector3<T> lerp(const Vector3<T>& a, const Vector3<T>& b, const float& t) {
		return a * (1 - t) + b * t;
	}

	template<typename T>
	inline Vector4<T> toVector4(const Vector3<T>& v) {
		return Vector4<T>(v.x, v.y, v.z, 1.0f);
	}

	typedef Vector3<float> Vector3f;
	typedef Vector3<int> Vector3i;

	template<typename T>
	class Vector2 {
	public:
		T x, y;
		Vector2() :x(0), y(0) {}
		Vector2(T x) :x(x), y(x) {}
		Vector2(T x, T y) :x(x), y(y) {}
		Vector2<T>* operator=(const Vector2<T>& v) {
			x = v.x, y = v.y;
			return *this;
		}
		Vector2<T> operator+(const Vector2<T>& v) const {
			return Vector2(x + v.x, y + v.y);
		}
		Vector2<T>& operator+=(const Vector2<T>& v) {
			x += v.x, y += v.y;
			return *this;
		}
		Vector2<T> operator-(const Vector2<T>& v) const {
			return Vector2(x - v.x, y - v.y);
		}
		Vector2<T>& operator-=(const Vector2<T>& v) {
			x -= v.x, y -= v.y;
			return *this;
		}
		Vector2<T> operator-() const {
			return Vector2<T>(-x, -y);
		}
		template<typename U>
		Vector2<T> operator*(U s) const {
			return Vector2<T>(x * s, y * s);
		}
		template<>
		Vector2<T> operator*(const Vector2<T>& v) const {
			return Vector3<T>(x * v.x, y * v.y);
		}
		template<typename U>
		Vector2<T>& operator*=(U s) {
			x *= s, y *= s;
			return *this;
		}
		template<typename U>
		Vector2<T> operator/(U s) const {
			return Vector2<T>(x / s, y / s);
		}
		template<>
		Vector2<T> operator/(Vector2<T> v) const {
			return Vector2<T>(x / v.x, y / v.y);
		}
		template<typename U>
		Vector2<T>& operator/=(U s) {
			x /= s, y /= s;
			return *this;
		}
		float Norm() const {
			return x * x + y * y;
		}
		float Length() const {
			return std::sqrt(Norm());
		}
		Vector2<T>& Normalize() {
			operator/=(Norm());
			return *this;
		}
		friend std::ostream& operator<<(std::ostream& os, const Vector2<T>& v) {
			return os << v.x << "," << v.y;
		}
		friend Vector2<T> operator*(const T& r, const Vector2<T>& v) {
			return Vector2<T> *r;
		}
	};

	template<typename T>
	inline T dotProduct(const Vector2<T>& a, const Vector2<T>& b) {
		return (a.x * b.x + a.y * b.y);
	}

	template<typename T>
	inline Vector2<T> lerp(const Vector2<T>& a, const Vector2<T>& b, const float& t) {
		return a * (1 - t) + b * t;
	}

	typedef Vector2<float> Vector2f;
	typedef Vector2<int> Vector2i;

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

		Matrixf3x3 operator+(const Matrixf3x3& m) const {
			Matrixf3x3 ans;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					ans.m[i][j] = this->m[i][j] + m.m[i][j];
				}
			}
			return ans;
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

		Matrixf4x4 operator+(const Matrixf4x4& m) const {
			Matrixf4x4 ans;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					ans.m[i][j] = this->m[i][j] + m.m[i][j];
				}
			}
			return ans;
		}

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
	};

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

	inline Matrixf4x4 Matrix4x4Transpose(const Matrixf4x4& m) {
		Matrixf4x4 mat(m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
			m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
			m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
			m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]);
		return mat;
	}
	
	inline float Matrix4x4Determinant(const Matrixf4x4& m) {
		return m.m[0][0] * Matrix3x3Determinant(Matrix4x4CMinor(m, 0, 0)) +
			m.m[0][1] * Matrix3x3Determinant(Matrix4x4CMinor(m, 0, 1)) +
			m.m[0][2] * Matrix3x3Determinant(Matrix4x4CMinor(m, 0, 2)) +
			m.m[0][3] * Matrix3x3Determinant(Matrix4x4CMinor(m, 0, 3));
	}

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

	inline Matrixf4x4 Matrix4x4Identity() {
		Matrixf4x4 m;
		return m;
	}
}