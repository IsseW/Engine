#include<renderer/passes/draw_objects.h>

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

ShadowPass::Locals ShadowPass::Locals::from_object(const Object& obj) {
	return Locals{
		obj.transform.get_mat().transposed()
	};
}

void ShadowPass::draw(const RendererCtx& ctx, const World& world, const AssetHandler& assets) {
	if (world.dir_lights.len() > directional_shadows.size.z) {
		directional_shadows.resize(ctx.device, directional_shadows.size.with_z((u16)world.dir_lights.len() + 5));
	}
	if (world.spot_lights.len() > spot_shadows.size.z) {
		spot_shadows.resize(ctx.device, spot_shadows.size.with_z((u16)world.spot_lights.len() + 5));
	}

	ctx.context->VSSetShader(vs, nullptr, 0);
	ctx.context->IASetInputLayout(il);
	
	ctx.context->ClearDepthStencilView(directional_shadows.view, D3D11_CLEAR_DEPTH, 1.0f, 0);
	ctx.context->OMSetRenderTargets(0, nullptr, directional_shadows.view);

	ID3D11Buffer* uniforms[2] = { globals.buffer, locals.buffer };
	ctx.context->VSSetConstantBuffers(0, 2, uniforms);

	auto draw = [&](const auto& lights) {
		lights.values([&](const auto& light) {
			Globals g = Globals::from_light(light);
			globals.update(ctx.context, &g);
			draw_objects(ctx, world, assets, [&](const Object& obj) {
				Locals l = Locals::from_object(obj);
				locals.update(ctx.context, &l);
				return false;
			});
		});
	};
	draw(world.dir_lights);

	ctx.context->ClearDepthStencilView(spot_shadows.view, D3D11_CLEAR_DEPTH, 1.0f, 0);
	ctx.context->OMSetRenderTargets(0, nullptr, spot_shadows.view);

	draw(world.spot_lights);
}

Result<ShadowPass, RenderCreateError> ShadowPass::create(ID3D11Device* device, Vec2<u16> size) {

	VSIL vsil;
	TRY(vsil, load_vertex(device, "ShadowVertex.cso", VERTEX_LAYOUT));

	Uniform<Globals> globals;
	TRY(globals, Uniform<Globals>::create(device));

	Uniform<Locals> locals;
	TRY(locals, Uniform<Locals>::create(device));

	DepthTextures directional_shadows;
	TRY(directional_shadows, DepthTextures::create(device, get_size(size).with_z(5)));

	DepthTextures spot_shadows;
	TRY(spot_shadows, DepthTextures::create(device, get_size(size).with_z(5)));

	return ok<ShadowPass, RenderCreateError>(ShadowPass{
			vsil.vs,
			vsil.il,
			globals,
			locals,
			directional_shadows,
			spot_shadows,
		});
}

ShadowPass::Globals ShadowPass::Globals::from_light(const DirLight& light)
{
	auto mat = light.transform.get_mat().invert() * math::create_orth_proj(-100.0f, 100.0f, -100.0f, 100.0f, 0.0f, 100.0f);

	return Globals{
		mat.transposed(),
	};
}

ShadowPass::Globals ShadowPass::Globals::from_light(const SpotLight& light)
{
	auto mat = light.transform.get_mat().invert() * math::create_persp_proj(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 100.0f);

	return Globals{
		mat.transposed(),
	};
}
