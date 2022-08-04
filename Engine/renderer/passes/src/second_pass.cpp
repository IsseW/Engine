#include<renderer/renderer.h>
#include<windows.h>

Result<SecondPass, RenderCreateError> SecondPass::create(ID3D11Device* device) {
	ID3D11ComputeShader* deferred;
	TRY(deferred, load_compute(device, "deferred.cso"));

	Uniform<ObjectData> object;
	TRY(object, Uniform<ObjectData>::create(device));

	SBuffer<LightData> lights;
	TRY(lights, SBuffer<LightData>::create(device, 10));


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
			object,
			lights,
			shadow_sampler_state,
	});
}

void SecondPass::clean_up() {
	if (deferred) deferred->Release();
}

void SecondPass::draw(Renderer& rend, const World& world, const Viewpoint& viewpoint) {
	// Unbind render targets. 
	rend.ctx.context->OMSetRenderTargets(0, NULL, NULL);

	rend.ctx.context->CSSetShader(deferred, NULL, 0);

	// Bind lights.
	Vec<LightData> lights {};
	world.lights.values([&](const Light& light) {
		lights.push(LightData::from_light(light, world.camera));
	});
	this->lights.update(rend.ctx.context, lights.raw(), lights.len());

	ObjectData object = {
		viewpoint.pos,
		mode,
		lights.len(),
	};
	this->object.update(rend.ctx.context, &object);
	rend.ctx.context->CSSetConstantBuffers(0, 1, &this->object.buffer);

	const usize SRV_COUNT = 8;
	ID3D11ShaderResourceView* srv[SRV_COUNT] = {
		rend.first_pass.gbuffer.ambient.srv,
		rend.first_pass.gbuffer.diffuse.srv,
		rend.first_pass.gbuffer.specular.srv,
		rend.first_pass.gbuffer.normal.srv,
		rend.first_pass.gbuffer.position.srv,
		rend.first_pass.depth.srv,
		rend.shadow_pass.shadows.srv,
		this->lights.srv,
	};
	rend.ctx.context->CSSetShaderResources(0, SRV_COUNT, srv);
	rend.ctx.context->CSSetUnorderedAccessViews(0, 1, &viewpoint.render_target, nullptr);
	rend.ctx.context->CSSetSamplers(0, 1, &shadow_sampler);

	auto size = rend.ctx.size();
	rend.ctx.context->Dispatch(size.x, size.y, 1);

	// Unbind resources
	ID3D11ShaderResourceView* empty_srv[SRV_COUNT] = { nullptr };
	rend.ctx.context->CSSetShaderResources(0, SRV_COUNT, empty_srv);
	ID3D11UnorderedAccessView* empty_uav = nullptr;
	rend.ctx.context->CSSetUnorderedAccessViews(0, 1, &empty_uav, nullptr);
}

SecondPass::LightData SecondPass::LightData::from_light(const Light& light, const Camera& camera) {
	return LightData {
		light.get_view_mat(camera).transposed(),
		light.get_proj_mat(camera).transposed(),
		light.transform.translation,
		light.transform.forward(),
		light.color,
		(u32)light.light_type,
		std::cos(light.angle),
	};
}