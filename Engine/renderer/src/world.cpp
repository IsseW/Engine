#include<renderer/world.h>
#include<renderer/window.h>
#include<math/consts.h>
#include<algorithm>

Object::Object(Transform transform) : transform(transform) {}

Object&& Object::with_mesh(AId<Mesh> mesh) {
	this->mesh = some(mesh);

	return std::move(*this);
}

ObjectData Object::get_data() const {
	return ObjectData{
		transform.get_mat().transposed(),
	};
}

World::World(Camera camera) : camera(camera) { }

Id<Object> World::add(Object&& object) {
	return objects.insert(std::move(object));
}
Id<SpotLight> World::add(SpotLight&& object) {
	return spot_lights.insert(std::move(object));
}

Id<DirLight> World::add(DirLight&& object)
{
	return dir_lights.insert(std::move(object));
}

void World::remove(Id<Object> id) {
	objects.remove(id);
}
void World::remove(Id<SpotLight> id) {
	spot_lights.remove(id);
}
void World::remove(Id<DirLight> id) {
	dir_lights.remove(id);
}

void update_camera(Camera& cam, f32 dt, const Window& window) {

	if (window.input().mouse_locked()) {
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
		if (move.length_sqr() > 0.0f) {
			move.normalize();
			move *= cam.speed * dt * (window.input().pressed(Key::Ctrl) ? cam.speed_boost : 1.0f);
			cam.transform.translation += move;
		}
		// Rotation
		Vec2<f32> mouse_delta = window.input().mouse_delta();
		if (mouse_delta.length_sqr() > 0.0) {
			f32 sensitivity = cam.sensitivity / 25.0f;
			cam.pitch -= sensitivity * mouse_delta.y * F32::TO_RAD;
			cam.pitch = std::clamp(cam.pitch, -F32::PI / 2.0f, F32::PI / 2.0f);
			cam.yaw += sensitivity * mouse_delta.x * F32::TO_RAD;

			cam.transform.rotation = Quat<f32>::angle_axis(Vec3<f32>::unit_y(), cam.yaw) * Quat<f32>::angle_axis(Vec3<f32>::unit_x(), cam.pitch);
		}
	}
	cam.pitch = std::clamp(cam.pitch, -F32::PI / 2.0f, F32::PI / 2.0f);
	cam.yaw = std::fmod(cam.yaw, 2.0f * F32::PI);
	cam.transform.rotation = Quat<f32>::angle_axis(Vec3<f32>::unit_y(), cam.yaw) * Quat<f32>::angle_axis(Vec3<f32>::unit_x(), cam.pitch);
}

void World::update(f32 dt, const Window& window) {
	update_camera(camera, dt, window);
}

Mat4<f32> SpotLight::get_texture_mat(const Camera& viewpoint) const {
	return transform.get_mat().invert() * math::create_persp_proj(-5.0f, 5.0f, -5.0f, 5.0f, 0.01f, 100.0f);
}

Mat4<f32> DirLight::get_texture_mat(const Camera& viewpoint) const {
	
	float radius = 10.0f;
	Vec3<f32> dir = transform.forward();
	Vec3<f32> target_pos = viewpoint.transform.translation;
	Vec3<f32> light_pos = target_pos + dir * radius;

	Mat4<f32> view = Transform::from_translation(light_pos).with_rotation(Quat<f32>::looking_dir(dir, Vec3<f32>::unit_z(), Vec3<f32>::unit_y())).get_mat().invert();
	
	Vec4<f32> target = view * target_pos.with_w(1.0);

	auto proj = math::create_orth_proj(
		target.x - radius,
		target.x + radius,
		target.y - radius,
		target.y + radius,
		target.z - 100.0f,
		target.z + 100.0f
	);

	return view;
}
