#pragma once
#include"first_pass.h"

enum class RenderMode {
	Deferred,
	Ambient,
	Diffuse,
	Specular,
	Depth,
	Normal,
	Position,
};

constexpr const char* MODES[7] = { "Deferred", "Ambient", "Diffuse", "Specular", "Depth", "Normal", "Position"};

struct SecondPass {
	struct ObjectData {
		Vec3<f32> cam_pos;
		RenderMode mode;
		u32 num_dir;
		u32 num_spot;
		Vec2<f32> dummy;
	};

	struct Directional {
		Mat4<f32> view_mat;
		Mat4<f32> proj_mat;
		Vec3<f32> pos;
		Vec3<f32> direction;
		Vec3<f32> color;
		f32 strength;
		
		static Directional from_light(const DirLight& light, const Camera& camera);
	};

	struct Spot {
		Mat4<f32> view_mat;
		Mat4<f32> proj_mat;
		Vec3<f32> pos;
		Vec3<f32> direction;
		Vec3<f32> color;
		f32 strength;

		static Spot from_light(const SpotLight& light, const Camera& camera);
	};

	ID3D11ComputeShader* deferred;

	Uniform<ObjectData> object;

	SBuffer<Directional> dir_lights;
	SBuffer<Spot> spot_lights;
	ID3D11SamplerState* shadow_sampler;

	RenderMode mode = RenderMode::Deferred;

	void draw(Renderer& rend, const World& world);

	static Result<SecondPass, RenderCreateError> create(ID3D11Device* device);

	void clean_up();
};