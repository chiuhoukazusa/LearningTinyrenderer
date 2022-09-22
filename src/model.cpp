#include "model.h"
#include "triangle.h"

namespace rst {
	model::model() {
		myEigen::Vector3f v[3];
		v[0] = myEigen::Vector3f(2, 0, -2);
		v[1] = myEigen::Vector3f(0, 2, -2);
		v[2] = myEigen::Vector3f(-2, 0, -2);
		t = Triangle(v);
	}
}