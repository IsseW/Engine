#pragma once
#include<renderer/util.h>
#include<renderer/world.h>



struct ShadowPass {

	struct DirectionalLight {
		Vec3<f32> dir;
		Vec3<f32> color;
		f32 strength;
	};
	struct SpotLight {

	};

	DepthTextures directional_shadows;

	SBuffer<DirectionalLight> directional_lights;

	static Result<ShadowPass, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void draw(const RendererCtx& ctx, const World& world, const AssetHandler& assets);

	void clean_up();
};
