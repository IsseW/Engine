
struct ParticleData {
	float3 velocity;
	float expected_life_time;
};

RWBuffer<float4> particle_pos : register(u0);
RWStructuredBuffer<ParticleData> particle_data : register(u1);

cbuffer SystemData : register(b0) {
	float4 min_spawn_point;
	float4 max_spawn_point;
	float4 start_dir;
	float4 acceleration;

	float start_angle_random;

	float vel_magnitude_min;
	float vel_magnitude_max;

	float min_life_time;
	float max_life_time;


	float delta_time;
	float time;

	uint particle_count;
	uint particles_per_thread;
}

float4 hash(float3 p, float t) {
	float4 p4 = float4(p, t);
	p4 = frac(p4 * float4(.1031, .1030, .0973, .1099));
	p4 += dot(p4, p4.wzxy + 33.33);
	return frac((p4.xxyz + p4.yzzw) * p4.zywx);
}

float3 rotate_around_axis(float3 v, float3 x, float a) {
	float ca = cos(a);
	return v * ca + cross(v, x) * sin(a) + x * dot(v, x) * (1.0 - ca);
}

[numthreads(8, 1, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID) {
	uint start = dispatch_id.x * particles_per_thread;
	uint end = min(start + particles_per_thread, particle_count);

	for (uint id = start; id < end; id++) {
		float3 pos = particle_pos[id].xyz;

		float expected_life_time = particle_data[id].expected_life_time;

		float life_time = particle_pos[id].w * expected_life_time - delta_time;

		if (life_time < 0.0) {
			float seed = time + (float)id;
			float4 r1 = hash(pos, seed);
			float4 r2 = hash(pos.xzy, -seed);

			// Select starting position
			particle_pos[id] = float4(min_spawn_point.xyz + (max_spawn_point.xyz - min_spawn_point.xyz) * r1.yzw, 1.0);

			// Pick a new lifetime
			particle_data[id].expected_life_time = min_life_time + (max_life_time - min_life_time) * r1.x;

			// Pick new velocity
			float vel_magnitude = vel_magnitude_min + (vel_magnitude_max - vel_magnitude_min) * r2.y;

			float a = start_angle_random * r2.z;
			float b = 2.0 * 3.141592643 * r2.w;
			float3 orth;
			float3 right = float3(1.0, 0.0, 0.0);
			if (dot(start_dir.xyz, right) == 0.0) {
				orth = float3(0.0, 1.0, 0.0);
			}
			else {
				orth = normalize(cross(start_dir.xyz, right));
			}
			float3 axis = rotate_around_axis(orth, start_dir.xyz, b);
			float3 start_d = rotate_around_axis(start_dir.xyz, axis, a);
			particle_data[id].velocity = vel_magnitude * start_d;
		}
		else {
			pos += particle_data[id].velocity * delta_time;
			particle_data[id].velocity += acceleration.xyz * delta_time;
			particle_pos[id] = float4(pos, life_time / expected_life_time);
		}
	}
}