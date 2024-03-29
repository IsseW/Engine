#include<renderer/passes/draw_objects.h>

void ShadowPass::clean_up() {
	shadows.clean_up();
}

constexpr Vec2<u16> TEX_SIZE = { 2048, 2048 };

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

		Vec<Id<Object>> objects{};
		Vec<Id<Reflective>> reflective{};

		collect_to_render(world, light.transform.get_mat(), light.get_proj_mat(world.camera), light.light_type == LightType::Directional, objects, reflective, {});

		draw_objects(rend, world, assets, g, false, objects, reflective);
	});
}

Result<ShadowPass, RenderCreateError> ShadowPass::create(ID3D11Device* device, Vec2<u16> size) {

	DepthTextures shadows;
	TRY(shadows, DepthTextures::create(device, TEX_SIZE.with_z(5)));

	return ok<ShadowPass, RenderCreateError>(ShadowPass{
		shadows,
	});
}
