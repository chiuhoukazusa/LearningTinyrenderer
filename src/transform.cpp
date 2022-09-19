#include "transform.h"
#include "myEigen.hpp"

namespace rst {
	constexpr float MY_PI = 3.14159265359f;
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

	Transform Rotate(const myEigen::Vector3f& axis, float theta) {
		float rad = theta / 180.0f * MY_PI;
		float cos = std::cos(rad);
		float sin = std::sin(rad);

		myEigen::Vector4f a(axis.x, axis.y, axis.z, 0);
		a.Normalize();
		myEigen::Matrixf4x4 L(a.x * a.x, a.x * a.y, a.x * a.z, 0.0f,
								a.y * a.x, a.y * a.y, a.y * a.z, 0.0f,
								a.x * a.z, a.z * a.y, a.z * a.z, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);
		myEigen::Matrixf4x4 N(0.0f, -a.z, -a.y, 0.0f,
								a.z, 0.0f, -a.x, 0.0f,
								-a.y, a.x, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);
		myEigen::Matrixf4x4 m;

		m = myEigen::Matrix4x4Identity() * cos + L * (1 - cos) + N * sin;
		m.m[3][3] = 1.0f;
		return Transform(m, myEigen::Matrix4x4Transpose(m));
	}
}