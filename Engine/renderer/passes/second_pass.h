#pragma once
#include"first_pass.h"

enum class RenderMode {
	Deferred,
	Ambient,
	Diffuse,
	Specular,
	Shininess,
	Depth,
	Normal,
	Position,
};

constexpr usize NUM_MODES = 8;
constexpr const char* MODES[NUM_MODES] = { "Deferred", "Ambient", "Diffuse", "Specular", "Shininess", "Depth", "Normal", "Position"};

struct SecondPass {
	struct ObjectData {
		Vec3<f32> cam_pos;
		RenderMode mode;
		u32 num_lights;
		Vec3<f32> dummy;
	};

	struct LightData {
		Mat4<f32> view_mat;
		Mat4<f32> proj_mat;
		Vec3<f32> pos;
		Vec3<f32> direction;
		Vec3<f32> color;

		u32 light_type;
		f32 cutoff;
		
		static LightData from_light(const Light& light, const Camera& camera);
	};

	ID3D11ComputeShader* deferred;

	ID3D11ComputeShader* ffxa;

	Uniform<ObjectData> object;

	SBuffer<LightData> lights;
	ID3D11SamplerState* shadow_sampler;

	RenderMode mode = RenderMode::Deferred;
	bool do_ffxa = true;

	void draw(Renderer& rend, const World& world, const Viewpoint& viewpoint);

	static Result<SecondPass, RenderCreateError> create(ID3D11Device* device);

	void clean_up();
};