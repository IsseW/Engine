#pragma once
#include<renderer/util.h>
#include<renderer/world.h>

struct ShadowPass {
	struct Globals {
		static Globals from_light(const DirLight& light);
		static Globals from_light(const SpotLight& light);

		Mat4<f32> texture_matrix;
	};
	struct Locals {
		Mat4<f32> world_matrix;

		static Locals from_object(const Object& obj);
	};

	ID3D11VertexShader* vs;
	ID3D11InputLayout* il;

	Uniform<Globals> globals;
	Uniform<Locals> locals;

	DepthTextures directional_shadows;

	DepthTextures spot_shadows;

	static Result<ShadowPass, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void draw(const RendererCtx& ctx, const World& world, const AssetHandler& assets);
	 
	void clean_up();
};
