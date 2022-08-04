#pragma once
#include<math/mat.h>

struct ObjectData;
struct Transform {
	static Transform from_translation(Vec3<f32> trans);
	static Transform from_scale(Vec3<f32> scale);
	static Transform from_rotation(Quat<f32> rot);
	Transform&& with_translation(Vec3<f32> trans);
	Transform&& with_scale(Vec3<f32> scale);
	Transform&& with_rotation(Quat<f32> rot);
	Transform&& looking_at(Vec3<f32> point);

	Vec3<f32> translation { 0.0, 0.0, 0.0 };
	Vec3<f32> scale { 1.0, 1.0, 1.0 };
	Quat<f32> rotation { 0.0, 0.0, 0.0, 1.0 };

	Vec3<f32> forward() const;
	Vec3<f32> back() const;
	Vec3<f32> right() const;
	Vec3<f32> left() const;
	Vec3<f32> up() const;
	Vec3<f32> down() const;

	Mat4<f32> get_mat() const;
	ObjectData get_data() const;
};

struct Camera {
	static Camera perspective(Transform transform, f32 fov);
	static Camera orthographic(Transform transform, f32 size);

	Mat4<f32> get_view() const;
	Mat4<f32> get_proj(f32 ratio) const;

	Transform transform;
	f32 pitch = 0.0f;
	f32 yaw = 0.0f;

	f32 speed = 4.0f;
	f32 speed_boost = 5.0f;
	f32 sensitivity = 1.0f;

	bool is_perspective;
	f32 cam_near;
	f32 cam_far;
	// Only used when the camera is orthographic
	f32 size;
	// Only used when the camera is perspective
	f32 fov;
};