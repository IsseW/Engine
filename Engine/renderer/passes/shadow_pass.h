#pragma once
#include<renderer/util.h>
#include<renderer/world.h>

struct ShadowPass {
	ID3D11VertexShader* vs;
	ID3D11InputLayout* il;

	DepthTextures directional_shadows;

	DepthTextures spot_shadows;

	static Result<ShadowPass, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void draw(Renderer& rend, const World& world, const AssetHandler& assets);
	 
	void clean_up();
};
