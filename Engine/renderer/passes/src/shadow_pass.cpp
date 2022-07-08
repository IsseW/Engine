#include<renderer/renderer.h>

void ShadowPass::clean_up() {
	directional_shadows.clean_up();
}

Vec2<u16> get_size(Vec2<u16> size) {
	// TODO
	return Vec2<u16>(100, 100);
}

void ShadowPass::resize(ID3D11Device* device, Vec2<u16> size) {
	// directional_shadows.resize(device, get_size(size));
}

void ShadowPass::draw(const RendererCtx& ctx, const World& world, const AssetHandler& assets) {
	ctx.context->ClearDepthStencilView(directional_shadows.view, D3D11_CLEAR_DEPTH, 1.0f, 0);
	ctx.context->OMSetRenderTargets(0, nullptr, directional_shadows.view);



}

Result<ShadowPass, RenderCreateError> ShadowPass::create(ID3D11Device* device, Vec2<u16> size) {
	DepthTextures directional_shadows;
	TRY(directional_shadows, DepthTextures::create(device, get_size(size).with_z(5)));

	SBuffer<DirectionalLight> directional_lights;
	TRY(directional_lights, SBuffer<DirectionalLight>::create(device));

	return ok<ShadowPass, RenderCreateError>(ShadowPass{
			nullptr,
			directional_shadows,
			directional_lights,
		});
}
