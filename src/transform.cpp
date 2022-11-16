#include "transform.h"

namespace rst {
	constexpr float MY_PI = 3.14159265359f;

	void Transform::toNormal() {
		m = myEigen::Matrix4x4Transpose(myEigen::Matrix4x4Inverse(m));
		mInv = myEigen::Matrix4x4Inverse(m);
	}

	void Transform::undo() {
		std::swap(m, mInv);
	}

	Transform Translate(const myEigen::Vector3f& delta) {
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

	Transform Scale(const myEigen::Vector3f& scale) {
		return Scale(scale.x, scale.y, scale.z);
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

	Transform WolrdToNewCoordinate(const myEigen::Vector3f& newX, const myEigen::Vector3f& newY, const myEigen::Vector3f& newZ,
		const myEigen::Vector3f& newZero)
	{
		myEigen::Vector4f NewX(newX.x, newX.y, newX.z, 0);
		myEigen::Vector4f NewY(newY.x, newY.y, newY.z, 0);
		myEigen::Vector4f NewZ(newZ.x, newZ.y, newZ.z, 0);
		myEigen::Vector4f NewZero(newZero.x, newZero.y, newZero.z, 1);
		NewX.Normalize();
		NewY.Normalize();
		NewZ.Normalize();

		myEigen::Matrixf4x4 mInv(NewX, NewY, NewZ, NewZero);
		myEigen::Matrixf4x4 m = myEigen::Matrix4x4Inverse(mInv);
		return Transform(m, mInv);
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

	Transform Modeling(const myEigen::Vector3f& translate,
		const myEigen::Vector3f& scale,
		const myEigen::Vector3f& axis, float theta)
	{
		return Translate(translate) * Rotate(axis, theta) * Scale(scale);
	}

	Transform Camera(const myEigen::Vector3f& eye_pos,const myEigen::Vector3f& gaze_dir,const myEigen::Vector3f& view_up)
	{
		myEigen::Vector3f newx = myEigen::crossProduct(view_up, -gaze_dir);
		myEigen::Vector3f newy = view_up;
		myEigen::Vector3f newz = -gaze_dir;
		myEigen::Vector3f newzero = eye_pos;
		return WolrdToNewCoordinate(newx, newy, newz, newzero);
	}

	Transform Orthographic(float left, float bottom, float near, float right, float top, float far)
	{
		myEigen::Matrixf4x4 m(2.0f / fabs(right - left), 0.0f, 0.0f, -(right + left) / fabs(right - left),
			0.0f, 2.0f / fabs(top - bottom), 0.0f, -(top + bottom) / fabs(top - bottom),
			0.0f, 0.0f, 2.0f / fabs(near - far), -(near + far) / fabs(near - far),
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
		myEigen::Matrixf4x4 m(2.0f / fabs(right - left), 0.0f, 0.0f, -(right + left) / fabs(right - left),
			0.0f, 2.0f / fabs(top - bottom), 0.0f, -(top + bottom) / fabs(top - bottom),
			0.0f, 0.0f, 2.0f / fabs(near - far), -(near + far) / fabs(near - far),
			0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m, myEigen::Matrix4x4Inverse(m));
	}

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

		float t = std::tan(ffovY / 2) * fabs(n);
		float b = -t;
		float r = aspect * t;
		float l = -r;
		return Orthographic(l, b, -zneardis, r, t, -zfardis) * per;
	}

	Transform Viewport(float width, float height)
	{
		myEigen::Matrixf4x4 m(width / 2.0f, 0.0f, 0.0f, width / 2.0f,
			0.0f, height / 2.0f, 0.0f, height / 2.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		return Transform(m);
	}
}