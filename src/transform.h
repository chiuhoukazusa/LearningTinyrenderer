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
			Transform ans(this->m * t.m);
			return ans;
		}

	private:
		myEigen::Matrixf4x4 m, mInv;
	};

	Transform Translate(const myEigen::Vector3f& delta);
	Transform Scale(float x, float y, float z);
	Transform RotateX(float theta);
	Transform RotateY(float theta);
	Transform RotateZ(float theta);
	Transform Rotate(const myEigen::Vector3f& axis, float theta);

}