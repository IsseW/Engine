#include<renderer/transform.h>
#include <directxmath.h>

Transform Transform::from_translation(Vec3<f32> trans) {
	Transform t{};
	t.translation = trans;
	return t;
}
Transform Transform::from_scale(Vec3<f32> scale) {
	Transform t{};
	t.scale = scale;
	return t;
}
Transform Transform::from_rotation(Quat<f32> rot) {
	Transform t{};
	t.rotation = rot;
	return t;
}
Transform&& Transform::with_translation(Vec3<f32> trans) {
	translation = trans;
	return std::move(*this);
}
Transform&& Transform::with_scale(Vec3<f32> scale) {
	this->scale = scale;
	return std::move(*this);
}
Transform&& Transform::with_rotation(Quat<f32> rot) {
	rotation = rot;
	return std::move(*this);
}
Transform&& Transform::looking_at(Vec3<f32> point) {
	Vec3<f32> dir = (point - translation).normalized();
	return this->with_rotation(Quat<f32>::looking_dir(dir, Vec3<f32>::unit_z(), Vec3<f32>::unit_y()));
}

Mat4<f32> Transform::get_mat() const {
	return math::transformation(translation, scale, rotation);
}

Camera Camera::perspective(Transform transform, f32 fov) {
	Camera c;
	c.transform = transform;
	c.cam_near = 0.01f;
	c.cam_far = 100.0f;
	c.is_perspective = true;
	c.size = 10.0f;
	c.fov = fov;
	return c;
}
Camera Camera::orthographic(Transform transform, f32 size) {
	Camera c;
	c.transform = transform;
	c.cam_near = 0.01f;
	c.cam_far = 100.0f;
	c.is_perspective = false;
	c.size = size;
	c.fov = 60.0f;
	return c;
}

Mat4<f32> Camera::get_view() const {
	return transform.get_mat().invert();
}

Mat4<f32> create_orth_proj(f32 left, f32 right, f32 bottom, f32 top, f32 cam_near, f32 cam_far) {
	return Mat4<f32> {
		2.0f / (right - left),				0.0f,								0.0f,							0.0f,
		0.0f,								2.0f / (top - bottom),				0.0f,							0.0f,
		0.0f,								0.0f,								1.0f / (cam_far - cam_near),			0.0f,
		(right + left) / (left - right),	(top + bottom) / (bottom - top),	0.5f + 0.5f * (cam_far + cam_near) / (cam_near - cam_far),	1.0f
	};
}
Mat4<f32> create_persp_proj(f32 left, f32 right, f32 bottom, f32 top, f32 cam_near, f32 cam_far) {
	return Mat4<f32> {
		2.0f * cam_near / (right - left),		0.0f,								0.0f,								0.0f,
		0.0f,								2.0f * cam_near / (top - bottom),		0.0f,								0.0f,
		(right + left) / (right - left),	(top + bottom) / (top - bottom),	0.5f + 0.5f * (cam_far + cam_near) / (cam_far - cam_near), 1.0f,
		0.0f,								0.0f,								cam_far * cam_near / (cam_near - cam_far),	0.0f,
	};
}

Mat4<f32> from_direct(DirectX::XMFLOAT4X4 mat) {
	return Mat4<f32> {
		mat._11, mat._12, mat._13, mat._14,
		mat._21, mat._22, mat._23, mat._24,
		mat._31, mat._32, mat._33, mat._34,
		mat._41, mat._42, mat._43, mat._44
	};
}

Mat4<f32> Camera::get_proj(u32 width, u32 height) const {
	f32 ratio = (f32)width / (f32)height;
	if (is_perspective) {
		f32 top = tanf(fov / 2.0f) * cam_near;
		f32 bottom = -top;
		f32 right = top * ratio;
		f32 left = -right;

		return create_persp_proj(left, right, bottom, top, cam_near, cam_far);
	}
	else {
		f32 top = size;
		f32 bottom = -top;
		f32 right = top * ratio;
		f32 left = -right;
		return create_orth_proj(left, right, bottom, top, cam_near, cam_far);
	}
}