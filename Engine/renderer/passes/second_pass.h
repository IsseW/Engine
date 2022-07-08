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

	ID3D11ComputeShader* deferred;

	Uniform<Locals> locals;

	RenderMode mode = RenderMode::Deferred;

	void draw(Renderer& rend, const World& world);

	static Result<SecondPass, RenderCreateError> create(ID3D11Device* device);

	void clean_up();
};