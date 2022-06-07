#pragma once
#include<math/mat.h>

struct Transform {
	Vec3<f32> translation;
	Vec3<f32> scale;
	Quat<f32> rotation;
};

