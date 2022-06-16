#pragma once
#include<math/mat.h>

struct Transform {
	static Transform from_translation(Vec3<f32> trans);
	static Transform from_scale(Vec3<f32> scale);
	static Transform from_rotation(Quat<f32> rot);
	Transform&& with_translation(Vec3<f32> trans);
	Transform&& with_scale(Vec3<f32> scale);
	Transform&& with_rotation(Quat<f32> rot);
	Transform&& looking_at(Vec3<f32> point, Vec3<f32> up);

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
