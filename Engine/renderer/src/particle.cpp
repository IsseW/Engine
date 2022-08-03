#include<renderer/world.h>


ParticleSystem ParticleSystem::create(ID3D11Device* device, Transform transform, usize num_particles) {
	ParticleSystem system {};
	system.transform = transform;

	ParticleData* data = new ParticleData[num_particles];
	memset(data, 0, sizeof(ParticleData) * num_particles);
	system.particle_data = UAVSBuffer<ParticleData>::create(device, data, num_particles).unwrap();

	Vec4<f32>* p = new Vec4<f32>[num_particles * 2];
	memset(p, 0, sizeof(Vec4<f32>) * num_particles * 2);
	system.particles = Buffer::create(device, p, num_particles).unwrap();

	delete[] data;
	delete[] p;
	return system;
}

void ParticleSystem::set_material(AId<Material> material) {
	this->material.insert(material);
}

ParticleSystemData ParticleSystem::get_data(f32 time, f32 delta_time) const {
	return ParticleSystemData{
		min_spawn_point.with_w(0.0),
		max_spawn_point.with_w(0.0),
		start_dir.with_w(0.0),
		acceleration.with_w(0.0),
		start_angle_random,
		vel_magnitude_min,
		vel_magnitude_max,
		min_life_time,
		max_life_time,
		delta_time,
		time,
		num_particles(),
	};
}

ObjectData ParticleSystem::get_object_data() const {
	return ObjectData{
		transform.get_mat().transposed(),
	};
}

usize ParticleSystem::num_particles() const {
	return particles.len;
}

void ParticleSystem::clean_up() {
	particle_data.clean_up();
	particles.clean_up();
}