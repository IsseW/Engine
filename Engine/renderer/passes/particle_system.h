#pragma once
#include<renderer/util.h>
#include<renderer/world.h>

struct ParticleRenderer {
	struct Globals {
		Vec2<f32> start_size;

		Vec2<f32> dummy;
	};
	ID3D11VertexShader* vs;
	ID3D11InputLayout* il;

	ID3D11ComputeShader* cs;
	ID3D11GeometryShader* gs;

	Uniform<ParticleSystemData> system_data;
	Uniform<Globals> gs_globals;

	
	void run(Renderer& rend, const World& world, f32 delta_time);
	static Result<ParticleRenderer, RenderCreateError> create(ID3D11Device* device);
	void clean_up();
};