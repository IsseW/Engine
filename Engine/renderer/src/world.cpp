#include<renderer/world.h>
#include<renderer/window.h>
#include<math/consts.h>
#include<algorithm>

Object::Object(Transform transform, AId<Mesh> mesh, Rgb color) : transform(transform), mesh(mesh), color(color) {}

Object&& Object::with_image(AId<Image> image) {
	this->image = some(image);

	return std::move(*this);
}

World::World(Camera camera, DirLight light) : camera(camera), dir_light(light), point_lights(), objects() { }

Id<Object> World::add(Object&& object) {
	return objects.insert(std::move(object));
}
Id<PointLight> World::add(PointLight&& object) {
	return point_lights.insert(std::move(object));
}

void update_camera(Camera& cam, f32 dt, const Window& window) {
	// Translation
	Vec3<f32> move;
	if (window.input().pressed(Key::Space)) {
		move.y += 1.0f;
	}
	if (window.input().pressed(Key::Shift)) {
		move.y -= 1.0f;
	}
	Vec3<f32> forward = cam.transform.forward();
	Vec3<f32> right = cam.transform.right();

	Vec3<f32> move_forward = forward.x_z().normalized();
	Vec3<f32> move_right = right.x_z().normalized();
	if (window.input().pressed(Key::W)) {
		move += move_forward;
	}
	if (window.input().pressed(Key::S)) {
		move -= move_forward;
	}
	if (window.input().pressed(Key::D)) {
		move += move_right;
	}
	if (window.input().pressed(Key::A)) {
		move -= move_right;
	}
	const f32 MOVE_SPEED = 1.0;
	if (move.length_sqr() > 0.0) {
		move.normalize();
		cam.transform.translation += (move * MOVE_SPEED * dt);
	}
	// Rotation
	const f32 SENSITIVITY = 1.0f;

	static f32 pitch = 0.0f;
	static f32 yaw = 0.0f;

	Vec2<f32> mouse_delta = window.input().mouse_delta();
	if (mouse_delta.length_sqr() > 0.0) {
		pitch += SENSITIVITY * mouse_delta.y * F32::TO_RAD;
		pitch = std::clamp(pitch, -F32::PI / 2.0f, F32::PI / 2.0f);
		yaw += SENSITIVITY * mouse_delta.x * F32::TO_RAD;

		cam.transform.rotation = Quat<f32>::angle_axis(Vec3<f32>::unit_y(), yaw) * Quat<f32>::angle_axis(Vec3<f32>::unit_x(), pitch);
	}
}

void World::update(f32 dt, const Window& window) {
	update_camera(camera, dt, window);
}