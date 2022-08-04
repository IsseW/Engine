#include<renderer/world.h>
#include<renderer/window.h>
#include<math/consts.h>
#include<algorithm>

Object::Object(Transform transform) : transform(transform) {}

Object&& Object::with_mesh(AId<Mesh> mesh) {
	this->mesh = some(mesh);

	return std::move(*this);
}

ObjectData Transform::get_data() const {
	return ObjectData{
		get_mat().transposed(),
	};
}

World::World(Camera camera) : camera(camera) { }

Id<Object> World::add(Object&& object) {
	return objects.insert(std::move(object));
}
Id<Light> World::add(Light&& object) {
	return lights.insert(std::move(object));
}
Id<ParticleSystem> World::add(ParticleSystem&& object) {
	return particle_systems.insert(std::move(object));
}
Id<Reflective> World::add(Reflective&& object) {
	return reflective.insert(std::move(object));
}

void World::remove(Id<Object> id) {
	objects.remove(id);
}
void World::remove(Id<Light> id) {
	lights.remove(id);
}
void World::remove(Id<ParticleSystem> id) {
	particle_systems.remove(id).then_do([](ParticleSystem system) {
		system.clean_up();
	});
}
void World::remove(Id<Reflective> id) {
	reflective.remove(id).then_do([](Reflective reflective) {
		reflective.clean_up();
	});
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
	time += dt;
	update_camera(camera, dt, window);
}

void World::clean_up() {
	particle_systems.values([](ParticleSystem& system) {
		system.clean_up();
	});
}

Mat4<f32> Light::get_view_mat(const Camera& viewpoint) const {
	switch (light_type) {
	case LightType::Directional: {
		Vec3<f32> dir = transform.forward();
		Vec3<f32> target_pos = viewpoint.transform.translation;
		return Transform::from_translation(target_pos).with_rotation(Quat<f32>::looking_dir(dir, Vec3<f32>::unit_z(), Vec3<f32>::unit_y())).get_mat().invert();
	}
	case LightType::Spot:
		return transform.get_mat().invert();
	}
	return Mat4<f32>::identity();
}

Mat4<f32> Light::get_proj_mat(const Camera& viewpoint) const {
	switch (light_type) {
	case LightType::Directional: {
		float radius = 25.0f;
		return math::create_orth_proj(
			-radius,
			radius,
			-radius,
			radius,
			-100.0f,
			100.0f
		);
	}
	case LightType::Spot: {
		f32 n = 0.01f;
		f32 top = tanf(angle) * n;
		f32 bottom = -top;
		f32 right = top;
		f32 left = -right;
		return math::create_persp_proj(left, right, bottom, top, n, 100.0f);
	}
	}
	return Mat4<f32>::identity();
}

Light Light::directional(Transform trans, Vec3<f32> col)
{
	return Light{
		trans,
		col,
		LightType::Directional,
		0.0,
	};
}

Light Light::spot(Transform trans, Vec3<f32> col, f32 angle)
{
	return Light{
		trans,
		col,
		LightType::Spot,
		angle,
	};
}

Reflective::Reflective(Transform transform) : transform(transform), mesh() { }
Reflective::Reflective(ID3D11Device* device, Transform transform) : cube_texture(CubeTexture::create(device, { 500, 500 }).unwrap()), transform(transform), mesh() { }

Reflective&& Reflective::with_mesh(AId<Mesh> mesh) {
	this->mesh.insert(mesh);
	return std::move(*this);
}

void Reflective::create_texture(ID3D11Device* device) {
	cube_texture = CubeTexture::create(device, { 500, 500 }).unwrap();
}

void Reflective::clean_up() {
	cube_texture.clean_up();
}
