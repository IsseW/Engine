#include<renderer/passes/draw_objects.h>

void ShadowPass::clean_up() {
	shadows.clean_up();
}

constexpr Vec2<u16> TEX_SIZE = { 1024, 1024 };

void ShadowPass::resize(ID3D11Device* device, Vec2<u16> size) {
	// directional_shadows.resize(device, get_size(size));
}

void ShadowPass::draw(Renderer& rend, const World& world, const AssetHandler& assets) {
	if (world.lights.len() > shadows.size.z) {
		shadows.resize(rend.ctx.device, shadows.size.with_z((u16)world.lights.len() + 5));
	}
	D3D11_VIEWPORT view = {
		0,
		0,
		(f32)TEX_SIZE.x,
		(f32)TEX_SIZE.y,
		0,
		1,
	};
	rend.ctx.context->RSSetViewports(1, &view);

	ID3D11RenderTargetView* rtv[1] = { 0 };

	shadows.clear(rend.ctx.context);
	usize i = 0;
	world.lights.values([&](const Light& light) {
		rend.ctx.context->OMSetRenderTargets(0, rtv, shadows.dsvs[i++]);
		auto g = FirstPass::Globals::from_light(light, world.camera);
		draw_objects(rend, world, assets, g, false, Vec<Id<Object>>{}, Vec<Id<Reflective>>{});
	});
}

Result<ShadowPass, RenderCreateError> ShadowPass::create(ID3D11Device* device, Vec2<u16> size) {

	DepthTextures shadows;
	TRY(shadows, DepthTextures::create(device, TEX_SIZE.with_z(5)));

	return ok<ShadowPass, RenderCreateError>(ShadowPass{
		shadows,
	});
}
