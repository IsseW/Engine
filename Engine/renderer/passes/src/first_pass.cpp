#include<renderer/passes/draw_objects.h>
#include<fstream>

Result<ObjectRenderer, RenderCreateError> ObjectRenderer::create(ID3D11Device* device)
{
	VSIL vsil;
	TRY(vsil, load_vertex(device, "VertexShader.cso", VERTEX_LAYOUT));

	ID3D11PixelShader* ps;
	TRY(ps, load_pixel(device, "PixelShader.cso"));

	Uniform<ObjectRenderer::Locals> locals;
	TRY(locals, Uniform<Locals>::create(device));

	return ok<ObjectRenderer, RenderCreateError>(ObjectRenderer{
			vsil.vs,
			ps,
			vsil.il,
			locals
		});
}

void ObjectRenderer::clean_up() {
	vs->Release();
	ps->Release();
	layout->Release();

	locals.clean_up();
}

Result<GBuffer, RenderCreateError> GBuffer::create(ID3D11Device* device, Vec2<u16> size) {
	RenderTexture albedo;
	TRY(albedo, RenderTexture::create(device, size, DXGI_FORMAT_R8G8B8A8_UNORM));
	RenderTexture normal;
	TRY(normal, RenderTexture::create(device, size, DXGI_FORMAT_R32G32B32A32_FLOAT));
	RenderTexture position;
	TRY(position, RenderTexture::create(device, size, DXGI_FORMAT_R32G32B32A32_FLOAT));

	return ok<GBuffer, RenderCreateError>(GBuffer{
			albedo,
			normal,
			position,
		});
}

void GBuffer::resize(ID3D11Device* device, Vec2<u16> size) {
	albedo.resize(device, size);
	normal.resize(device, size);
	position.resize(device, size);
}

void GBuffer::clean_up() {
	albedo.clean_up();
	normal.clean_up();
	position.clean_up();
}

void GBuffer::clear(ID3D11DeviceContext* ctx) {
	const float clear_color[4] = { 0.2f, 0.2f, 0.2f, 0.0f };
	ctx->ClearRenderTargetView(albedo.rtv, clear_color);
	const float clear_empty[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ctx->ClearRenderTargetView(normal.rtv, clear_empty);
	ctx->ClearRenderTargetView(position.rtv, clear_empty);
}

std::array<ID3D11RenderTargetView*, 3> GBuffer::targets() const {
	return { albedo.rtv, normal.rtv, position.rtv };
}


Result<FirstPass, RenderCreateError> FirstPass::create(ID3D11Device* device, Vec2<u16> size) {
	ObjectRenderer object_renderer;
	TRY(object_renderer, ObjectRenderer::create(device));

	Uniform<Globals> globals;
	TRY(globals, Uniform<Globals>::create(device));

	DepthTexture depth;
	TRY(depth, DepthTexture::create(device, size));

	GBuffer gbuffer;
	TRY(gbuffer, GBuffer::create(device, size));

	return ok<FirstPass, RenderCreateError>(FirstPass{
			object_renderer,
			globals,
			depth,
			gbuffer,
		});
}

void FirstPass::resize(ID3D11Device* device, Vec2<u16> size) {
	depth.resize(device, size);
	gbuffer.resize(device, size);
}

void FirstPass::clean_up() {
	object_renderer.clean_up();
	globals.clean_up();

	depth.clean_up();
	gbuffer.clean_up();
}

FirstPass::Globals FirstPass::Globals::from_world(const World& world, f32 ratio) {
	return Globals{
		world.camera.get_view().transposed(),
		world.camera.get_proj(ratio).transposed(),
	};
}

ObjectRenderer::Locals ObjectRenderer::Locals::from_object(const Object& obj) {
	return Locals{
		obj.transform.get_mat().transposed(),
		obj.color.with_w(1.0)
	};
}

void FirstPass::draw(const RendererCtx& ctx, const World& world, const AssetHandler& assets) {
	ctx.context->ClearDepthStencilView(depth.dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	gbuffer.clear(ctx.context);

	auto targets = gbuffer.targets();
	ctx.context->OMSetRenderTargets(targets.size(), targets.data(), depth.dsv);

	// First update the globals buffer
	auto g = FirstPass::Globals::from_world(world, ctx.ratio());
	globals.update(ctx.context, &g);

	ctx.context->VSSetShader(object_renderer.vs, nullptr, 0);
	ctx.context->PSSetShader(object_renderer.ps, nullptr, 0);

	ID3D11Buffer* uniforms[2] = { globals.buffer, object_renderer.locals.buffer };
	ctx.context->VSSetConstantBuffers(0, 2, uniforms);
	ctx.context->PSSetConstantBuffers(0, 2, uniforms);
	ctx.context->IASetInputLayout(object_renderer.layout);

	draw_objects(ctx, world, assets, [&](const Object& obj) {
		auto locals = ObjectRenderer::Locals::from_object(obj);
		object_renderer.locals.update(ctx.context, &locals);

		const Image* image = assets.get_or_default(obj.image);

		if (image->binded.is_none()) {
			return true;
		}
		auto binded = image->binded.as_ptr().unwrap_unchecked();

		ctx.context->PSSetShaderResources(0, 1, &binded->rsv);
		ctx.context->PSSetSamplers(0, 1, &binded->sampler_state);
		return false;
	});
}