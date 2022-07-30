#include<renderer/renderer.h>
#include<windows.h>

const char* COMPUTE_FILE = "Deferred.cso";

Result<SecondPass, RenderCreateError> SecondPass::create(ID3D11Device* device) {
	ID3D11ComputeShader* deferred;
	TRY(deferred, load_compute(device, COMPUTE_FILE));

	Uniform<Locals> locals;
	TRY(locals, Uniform<Locals>::create(device));

	SBuffer<Directional> dir_lights;
	TRY(dir_lights, SBuffer<Directional>::create(device, 10));

	SBuffer<Spot> spot_lights;
	TRY(spot_lights, SBuffer<Spot>::create(device, 10));


	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.MaxAnisotropy = 4;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	sampler_desc.BorderColor[0] = 1.0;

	ID3D11SamplerState* shadow_sampler_state;
	if (FAILED(device->CreateSamplerState(&sampler_desc, &shadow_sampler_state))) {
		return FailedSamplerStateCreation;
	}

	return ok<SecondPass, RenderCreateError>(SecondPass{
			deferred,
			locals,
			dir_lights,
			spot_lights,
			shadow_sampler_state,
		});
}

void SecondPass::clean_up() {
	if (deferred) deferred->Release();
}

void SecondPass::draw(Renderer& rend, const World& world) {
	// Unbind render targets. 
	rend.ctx.context->OMSetRenderTargets(0, NULL, NULL);

	rend.ctx.context->CSSetShader(deferred, NULL, 0);

	// Bind lights.
	Vec<Directional> directional {};
	world.dir_lights.values([&](const DirLight& light) {
		directional.push(Directional::from_light(light, world.camera));
	});
	dir_lights.update(rend.ctx.context, directional.raw(), directional.len());

	Vec<Spot> spot{};
	world.spot_lights.values([&](const SpotLight& light) {
		spot.push(Spot::from_light(light, world.camera));
	});
	spot_lights.update(rend.ctx.context, spot.raw(), spot.len());

	Locals locals = {
		world.camera.transform.translation,
		mode,
		directional.len(),
		spot.len(),
	};
	this->locals.update(rend.ctx.context, &locals);
	rend.ctx.context->CSSetConstantBuffers(0, 1, &this->locals.buffer);


	const usize SRV_COUNT = 9;
	ID3D11ShaderResourceView* srv[SRV_COUNT] = {
		rend.first_pass.gbuffer.albedo.srv,
		rend.first_pass.gbuffer.normal.srv,
		rend.first_pass.gbuffer.position.srv,
		rend.first_pass.gbuffer.light_info.srv,
		rend.first_pass.depth.srv,
		rend.shadow_pass.directional_shadows.srv,
		rend.shadow_pass.spot_shadows.srv,
		dir_lights.srv,
		spot_lights.srv,
	};
	rend.ctx.context->CSSetShaderResources(0, SRV_COUNT, srv);
	rend.ctx.context->CSSetUnorderedAccessViews(0, 1, &rend.ctx.screen.uav, nullptr);
	rend.ctx.context->CSSetSamplers(0, 1, &shadow_sampler);


	auto size = rend.ctx.size();
	rend.ctx.context->Dispatch(size.x, size.y, 1);

	// Unbind resources
	ID3D11ShaderResourceView* empty_srv[SRV_COUNT] = { nullptr };
	rend.ctx.context->CSSetShaderResources(0, SRV_COUNT, empty_srv);
	ID3D11UnorderedAccessView* empty_uav = nullptr;
	rend.ctx.context->CSSetUnorderedAccessViews(0, 1, &empty_uav, nullptr);
}

SecondPass::Directional SecondPass::Directional::from_light(const DirLight& light, const Camera& camera)
{
	auto mat = light.get_texture_mat(camera);
	return Directional{
		mat.transposed(),
		light.transform.translation,
		light.transform.forward(),
		light.light.color,
		light.light.strength,
	};
}

SecondPass::Spot SecondPass::Spot::from_light(const SpotLight& light, const Camera& camera)
{
	auto mat = light.get_texture_mat(camera);
	return Spot{
		mat.transposed(),
		light.transform.translation,
		light.transform.forward(),
		light.light.color,
		light.light.strength,
	};
}
