#pragma once
#include"transform.h"
#include<assets/assets.h>
#include"util.h"

struct ParticleSystemData {
	Vec4<f32> min_spawn_point;
	Vec4<f32> max_spawn_point;
	Vec4<f32> start_dir;
	Vec4<f32> acceleration;

	f32 start_angle_random;

	f32 vel_magnitude_min;
	f32 vel_magnitude_max;

	f32 min_life_time;
	f32 max_life_time;


	f32 delta_time;
	f32 time;

	u32 particle_count;
};

struct ParticleData {
	Vec3<f32> velocity;
	f32 expected_life_time;
};

struct ObjectData;

struct ParticleSystem {
	Transform transform;

	Vec3<f32> min_spawn_point {-1.0f};
	Vec3<f32> max_spawn_point {1.0f};

	Vec3<f32> start_dir {0.0f, 1.0f, 0.0f};
	f32 start_angle_random { 0.2f };

	f32 vel_magnitude_min {1.0f};
	f32 vel_magnitude_max {5.0f};
	f32 min_life_time {4.0f};
	f32 max_life_time {8.0f};

	Vec3<f32> acceleration {0.0f, -0.4f, 0.0f};

	f32 start_size {0.1f};

	bool paused{ false };

	UAVSBuffer<ParticleData> particle_data;
	Buffer particles;

	Option<AId<Material>> material;

	static ParticleSystem create(ID3D11Device* device, Transform transform, usize num_particles);
	void set_material(AId<Material> material);
	ParticleSystemData get_data(f32 time, f32 delta_time) const;
	ObjectData get_object_data() const;
	usize num_particles() const;
	void clean_up();
};
