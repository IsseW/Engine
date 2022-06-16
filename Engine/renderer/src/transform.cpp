#include<renderer/transform.h>


Mat4<f32> Transform::get_mat() const {
	return math::transformation(translation, scale, rotation);
}

Camera Camera::perspective(Transform transform, f32 fov) {
	Camera c;
	c._transform = transform;
	c._near = 0.01f;
	c._far = 100.0f;
	c._perspective = true;
	c._size = 10.0f;
	c._fov = fov;
	return c;
}
Camera Camera::orthographic(Transform transform, f32 size) {
	Camera c;
	c._transform = transform;
	c._near = 0.01f;
	c._far = 100.0f;
	c._perspective = false;
	c._size = size;
	c._fov = 60.0f;
	return c;
}

Mat4<f32> Camera::get_view() const {
	return _transform.get_mat();
}

Mat4<f32> create_orth_proj(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
	return Mat4<f32> {
		2.0f / (right - left),				0.0f,								0.0f,							0.0f,
		0.0f,								2.0f / (top - bottom),				0.0f,							0.0f,
		0.0f,								0.0f,								2.0f / (far - near),			0.0f,
		(right + left) / (left - right),	(top + bottom) / (bottom - top),	(far + near) / (near - far),	1.0f
	};
}
Mat4<f32> create_persp_proj(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
	return Mat4<f32> {
		2.0f * near / (right - left),		0.0f,								0.0f,								0.0f,
		0.0f,								2.0f * near / (top - bottom),		0.0f,								0.0f,
		(right + left) / (right - left),	(top + bottom) / (top - bottom),	(far + near) / (near - far),		-1.0f,
		0.0f,								0.0f,								2.0f * far * near / (near - far),	0.0f,
	};
}

Mat4<f32> Camera::get_proj(u32 width, u32 height) const {
	f32 ratio = (f32)width / (f32)height;
	if (_perspective) {
		f32 top = tanf(_fov / 2.0f) * _near;
		f32 bottom = -top;
		f32 right = top * ratio;
		f32 left = -right;
	}
	else {
		f32 top = _size;
		f32 bottom = -top;
		f32 right = top * ratio;
		f32 left = -right;
		return create_orth_proj(left, right, bottom, top, _near, _far);
	}
}