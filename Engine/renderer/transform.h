#pragma once
#include<math/mat.h>

struct Transform {
	Vec3<f32> translation;
	Vec3<f32> scale;
	Quat<f32> rotation;

	Mat4<f32> get_mat() const;
};

struct Camera {
	static Camera perspective(Transform transform, f32 fov);
	static Camera orthographic(Transform transform, f32 size);

	Mat4<f32> get_view() const;
	Mat4<f32> get_proj(u32 width, u32 height) const;
private:
	Transform _transform;
	f32 _near;
	f32 _far;
	bool _perspective;
	// Only used when the camera is orthographic
	f32 _size;
	// Only used when the camera is perspective
	f32 _fov;
};
