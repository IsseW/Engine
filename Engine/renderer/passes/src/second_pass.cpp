#include<renderer/renderer.h>
#include<windows.h>

const char* COMPUTE_FILE = "Deferred.cso";

Result<SecondPass, RenderCreateError> SecondPass::create(ID3D11Device* device) {
	ID3D11ComputeShader* deferred;
	TRY(deferred, load_compute(device, COMPUTE_FILE));

	Uniform<Locals> locals;
	TRY(locals, Uniform<Locals>::create(device));

	return ok<SecondPass, RenderCreateError>(SecondPass{
			deferred,
			locals,
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


	ID3D11ShaderResourceView* views[4] = {
		rend.first_pass.gbuffer.albedo.srv,
		rend.first_pass.gbuffer.normal.srv,
		rend.first_pass.gbuffer.position.srv,
		rend.first_pass.depth.rsv,
	};
	rend.ctx.context->CSSetShaderResources(0, 4, views);

	rend.ctx.context->CSSetUnorderedAccessViews(0, 1, &rend.ctx.screen.uav, nullptr);


	auto size = rend.ctx.size();
	rend.ctx.context->Dispatch(size.x, size.y, 1);

	// ID3D11UnorderedAccessView* null_uav = nullptr;
	// rend.ctx.context->CSSetUnorderedAccessViews(0, 1, &null_uav, nullptr);
	// rend.ctx.context->CopyResource(rend.ctx.screen.texture, rend.first_pass.gbuffer.albedo.texture);
}