#pragma once
#include"first_pass.h"

enum class RenderMode {
	Deferred,
	Albedo,
	Depth,
	Normal,
	Position,
};

constexpr const char* MODES[5] = { "Deferred", "Albedo", "Depth", "Normal", "Position"};

struct SecondPass {
	struct Locals {
		RenderMode mode;
		Vec3<f32> padding;
	};

	struct Directional {
		Vec3<f32> direction;
		Vec3<f32> color;
		f32 strength;
		
		static Directional from_light(const DirLight& light);
	};

	struct Spot {
		Vec3<f32> direction;
		Vec3<f32> color;
		f32 strength;

		static Spot from_light(const SpotLight& light);
	};

	ID3D11ComputeShader* deferred;

	Uniform<Locals> locals;

	SBuffer<Directional> dir_lights;
	SBuffer<Spot> spot_lights;

	RenderMode mode = RenderMode::Deferred;

	void draw(Renderer& rend, const World& world);

	static Result<SecondPass, RenderCreateError> create(ID3D11Device* device);

	void clean_up();
};