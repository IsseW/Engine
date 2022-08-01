#include<renderer/passes/draw_objects.h>

void ShadowPass::clean_up() {
	vs->Release();
	il->Release();

	globals.clean_up();
	object.clean_up();
	directional_shadows.clean_up();
	spot_shadows.clean_up();
}

Vec2<u16> get_size(Vec2<u16> size) {
	// TODO
	return Vec2<u16>(100, 100);
}

void ShadowPass::resize(ID3D11Device* device, Vec2<u16> size) {
	// directional_shadows.resize(device, get_size(size));
}

ShadowPass::ObjectData ShadowPass::ObjectData::from_object(const Object& obj) {
	return ObjectData{
		obj.transform.get_mat().transposed()
	};
}

void ShadowPass::draw(Renderer& rend, const World& world, const AssetHandler& assets) {
	if (world.dir_lights.len() > directional_shadows.size.z) {
		directional_shadows.resize(rend.ctx.device, directional_shadows.size.with_z((u16)world.dir_lights.len() + 5));
	}
	if (world.spot_lights.len() > spot_shadows.size.z) {
		spot_shadows.resize(rend.ctx.device, spot_shadows.size.with_z((u16)world.spot_lights.len() + 5));
	}

	rend.ctx.context->VSSetShader(vs, nullptr, 0);
	rend.ctx.context->PSSetShader(nullptr, nullptr, 0);
	rend.ctx.context->IASetInputLayout(il);

	ID3D11RenderTargetView* rtv[1] = { 0 };

	ID3D11Buffer* uniforms[2] = { globals.buffer, object.buffer };
	rend.ctx.context->VSSetConstantBuffers(0, 2, uniforms);

	// TODO: Do occlusion for lights.
	auto draw = [&](const auto& lights, DepthTextures& target) {
		usize i = 0;
		lights.values([&](const auto& light) {
			rend.ctx.context->OMSetRenderTargets(0, rtv, target.dsvs[i++]);
			Globals g = Globals::from_light(light, world.camera);
			globals.update(rend.ctx.context, &g);
			draw_objects(rend, world, assets, false);
		});
	};

	directional_shadows.clear(rend.ctx.context);
	draw(world.dir_lights, directional_shadows);

	spot_shadows.clear(rend.ctx.context);
	draw(world.spot_lights, spot_shadows);
}

Result<ShadowPass, RenderCreateError> ShadowPass::create(ID3D11Device* device, Vec2<u16> size) {

	VSIL vsil;
	TRY(vsil, load_vertex(device, "ShadowVertex.cso", VERTEX_LAYOUT));

	Uniform<Globals> globals;
	TRY(globals, Uniform<Globals>::create(device));

	Uniform<ObjectData> object;
	TRY(object, Uniform<ObjectData>::create(device));

	DepthTextures directional_shadows;
	TRY(directional_shadows, DepthTextures::create(device, get_size(size).with_z(5)));

	DepthTextures spot_shadows;
	TRY(spot_shadows, DepthTextures::create(device, get_size(size).with_z(5)));

	return ok<ShadowPass, RenderCreateError>(ShadowPass{
			vsil.vs,
			vsil.il,
			globals,
			object,
			directional_shadows,
			spot_shadows,
		});
}

ShadowPass::Globals ShadowPass::Globals::from_light(const DirLight& light, const Camera& camera)
{
	auto mat = light.get_texture_mat(camera);

	return Globals{
		mat.transposed(),
	};
}

ShadowPass::Globals ShadowPass::Globals::from_light(const SpotLight& light, const Camera& camera)
{
	auto mat = light.get_texture_mat(camera);

	return Globals{
		mat.transposed(),
	};
}
