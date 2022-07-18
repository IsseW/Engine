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

	return ok<SecondPass, RenderCreateError>(SecondPass{
			deferred,
			locals,
			dir_lights,
			spot_lights,
		});
}

void SecondPass::clean_up() {
	if (deferred) deferred->Release();
}

void SecondPass::draw(Renderer& rend, const World& world) {

	rend.ctx.context->OMSetRenderTargets(0, NULL, NULL);

	rend.ctx.context->CSSetShader(deferred, NULL, 0);

	Locals locals = { mode };
	this->locals.update(rend.ctx.context, &locals);
	rend.ctx.context->CSSetConstantBuffers(0, 1, &this->locals.buffer);


	const usize SRV_COUNT = 6;
	ID3D11ShaderResourceView* srv[SRV_COUNT] = {
		rend.first_pass.gbuffer.albedo.srv,
		rend.first_pass.gbuffer.normal.srv,
		rend.first_pass.gbuffer.position.srv,
		rend.first_pass.depth.srv,
		rend.shadow_pass.directional_shadows.srv,
		rend.shadow_pass.spot_shadows.srv,
	};
	rend.ctx.context->CSSetShaderResources(0, SRV_COUNT, srv);
	rend.ctx.context->CSSetUnorderedAccessViews(0, 1, &rend.ctx.screen.uav, nullptr);


	auto size = rend.ctx.size();
	rend.ctx.context->Dispatch(size.x, size.y, 1);

	// Unbind resources
	ID3D11ShaderResourceView* empty_srv[SRV_COUNT] = { nullptr };
	rend.ctx.context->CSSetShaderResources(0, SRV_COUNT, empty_srv);
	ID3D11UnorderedAccessView* empty_uav = nullptr;
	rend.ctx.context->CSSetUnorderedAccessViews(0, 1, &empty_uav, nullptr);
}