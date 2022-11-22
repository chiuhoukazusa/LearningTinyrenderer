#pragma once

#include "myEigen.hpp"

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

		Transform operator*(const Transform& t) {
			Transform ans(this->m * t.m, myEigen::Matrix4x4Inverse(this->m * t.m));
			return ans;
		}

		template<typename T>
		myEigen::Vector3<T> operator()(const myEigen::Vector3<T>& v) const {
			myEigen::Vector4<T> v1(v.x, v.y, v.z, 1);
			v1 = m * v1;
			myEigen::Vector3<T> v2(v1.x, v1.y, v1.z);
			return v2;
		}
		template<typename T>
		myEigen::Vector4<T> operator()(const myEigen::Vector4<T>& v) const { return m * v; }
		void toNormal();
		void undo();

	private:
		myEigen::Matrixf4x4 m, mInv;
	};


	Transform Translate(const myEigen::Vector3f& delta);
	Transform Scale(float x, float y, float z);
	Transform Scale(const myEigen::Vector3f& scale);
	Transform RotateX(float theta);
	Transform RotateY(float theta);
	Transform RotateZ(float theta);
	Transform Rotate(const myEigen::Vector3f& axis, float theta);
	Transform WolrdToNewCoordinate(const myEigen::Vector3f& newX, const myEigen::Vector3f& newY, const myEigen::Vector3f& newZ,
		const myEigen::Vector3f& newZero);
	Transform NewToWorldCoordinate(const myEigen::Vector3f& newX, const myEigen::Vector3f& newY, const myEigen::Vector3f& newZ,
		const myEigen::Vector3f& newZero);

	Transform Modeling(const myEigen::Vector3f& translate = (0),
		const myEigen::Vector3f& scale = (1),
		const myEigen::Vector3f& axis = (0), float theta = 0);
	Transform Camera(const myEigen::Vector3f& eye_pos,const myEigen::Vector3f& gaze_dir,const myEigen::Vector3f& view_up);
	Transform Orthographic(float left, float bottom, float near, float right, float top, float far);
	Transform Orthographic(const myEigen::Vector3f& lbn, const myEigen::Vector3f& rtf);
	Transform Perspective(float zneardis, float zfardis, float fovY, float aspect);
	Transform Viewport(float width, float height);
}