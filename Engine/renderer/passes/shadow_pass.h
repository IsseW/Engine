#pragma once
#include<renderer/util.h>
#include<renderer/world.h>

struct ShadowPass {
	DepthTextures shadows;

	static Result<ShadowPass, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void draw(Renderer& rend, const World& world, const AssetHandler& assets);
	 
	void clean_up();
};
