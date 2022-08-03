#include<renderer/passes/draw_objects.h>
#include<fstream>

Result<ObjectRenderer, RenderCreateError> ObjectRenderer::create(ID3D11Device* device)
{
	VSIL vsil;
	TRY(vsil, load_vertex(device, "vertex.cso", VERTEX_LAYOUT));

	ID3D11PixelShader* ps;
	TRY(ps, load_pixel(device, "pixel.cso"));

	Uniform<ObjectData> object;
	TRY(object, Uniform<ObjectData>::create(device));

	Uniform<MaterialData> material;
	TRY(material, Uniform<MaterialData>::create(device));

	return ok<ObjectRenderer, RenderCreateError>(ObjectRenderer{
			vsil.vs,
			ps,
			vsil.il,
			object,
			material,
		});
}

void ObjectRenderer::clean_up() {
	vs->Release();
	ps->Release();
	layout->Release();

	object.clean_up();
}

Result<GBuffer, RenderCreateError> GBuffer::create(ID3D11Device* device, Vec2<u16> size) {
	RenderTexture ambient;
	TRY(ambient, RenderTexture::create(device, size, DXGI_FORMAT_R8G8B8A8_UNORM));
	RenderTexture diffuse;
	TRY(diffuse, RenderTexture::create(device, size, DXGI_FORMAT_R8G8B8A8_UNORM));
	RenderTexture specular;
	TRY(specular, RenderTexture::create(device, size, DXGI_FORMAT_R8G8B8A8_UNORM));
	RenderTexture normal;
	TRY(normal, RenderTexture::create(device, size, DXGI_FORMAT_R32G32B32A32_FLOAT));
	RenderTexture position;
	TRY(position, RenderTexture::create(device, size, DXGI_FORMAT_R32G32B32A32_FLOAT));

	return ok<GBuffer, RenderCreateError>(GBuffer{
			ambient,
			diffuse,
			specular,
			normal,
			position,
		});
}

void GBuffer::resize(ID3D11Device* device, Vec2<u16> size) {
	ambient.resize(device, size);
	diffuse.resize(device, size);
	specular.resize(device, size);
	normal.resize(device, size);
	position.resize(device, size);
}

void GBuffer::clean_up() {
	ambient.clean_up();
	diffuse.clean_up();
	specular.clean_up();
	normal.clean_up();
	position.clean_up();
}

void GBuffer::clear(ID3D11DeviceContext* ctx) {
	const float clear_empty[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ctx->ClearRenderTargetView(ambient.rtv, clear_empty);
	ctx->ClearRenderTargetView(diffuse.rtv, clear_empty);
	ctx->ClearRenderTargetView(specular.rtv, clear_empty);
	ctx->ClearRenderTargetView(normal.rtv, clear_empty);
	ctx->ClearRenderTargetView(position.rtv, clear_empty);
}

std::array<ID3D11RenderTargetView*, 5> GBuffer::targets() const {
	return { ambient.rtv, diffuse.rtv, specular.rtv, normal.rtv, position.rtv, };
}


Result<FirstPass, RenderCreateError> FirstPass::create(ID3D11Device* device, Vec2<u16> size) {
	ObjectRenderer object_renderer;
	TRY(object_renderer, ObjectRenderer::create(device));

	ParticleRenderer particle_renderer;
	TRY(particle_renderer, ParticleRenderer::create(device));

	Uniform<Globals> globals;
	TRY(globals, Uniform<Globals>::create(device));

	DepthTexture depth;
	TRY(depth, DepthTexture::create(device, size));

	GBuffer gbuffer;
	TRY(gbuffer, GBuffer::create(device, size));

	D3D11_RASTERIZER_DESC rs_desc;
	ZeroMemory(&rs_desc, sizeof(D3D11_RASTERIZER_DESC));
	rs_desc.FillMode = D3D11_FILL_SOLID;
	rs_desc.CullMode = D3D11_CULL_BACK;
	ID3D11RasterizerState* rs_default;
	if (FAILED(device->CreateRasterizerState(&rs_desc, &rs_default))) {
		return FailedRSCreation;
	}

	rs_desc.FillMode = D3D11_FILL_WIREFRAME;
	ID3D11RasterizerState* rs_wireframe;
	if (FAILED(device->CreateRasterizerState(&rs_desc, &rs_wireframe))) {
		return FailedRSCreation;
	}

	rs_desc.FillMode = D3D11_FILL_SOLID;
	rs_desc.CullMode = D3D11_CULL_NONE;
	ID3D11RasterizerState* rs_cull_none;
	if (FAILED(device->CreateRasterizerState(&rs_desc, &rs_cull_none))) {
		return FailedRSCreation;
	}

	return ok<FirstPass, RenderCreateError>(FirstPass{
		object_renderer,
		particle_renderer,
		globals,
		depth,
		gbuffer,
		rs_default,
		rs_wireframe,
		rs_cull_none,
	});
}

void FirstPass::resize(ID3D11Device* device, Vec2<u16> size) {
	depth.resize(device, size);
	gbuffer.resize(device, size);
}

void FirstPass::clean_up() {
	object_renderer.clean_up();
	particle_renderer.clean_up();
	globals.clean_up();

	depth.clean_up();
	gbuffer.clean_up();

	rs_default->Release();
	rs_wireframe->Release();
	rs_cull_none->Release();
}


void FirstPass::draw(Renderer& rend, const World& world, const AssetHandler& assets) {
	rend.ctx.context->ClearDepthStencilView(depth.dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	gbuffer.clear(rend.ctx.context);

	auto targets = gbuffer.targets();
	rend.ctx.context->OMSetRenderTargets(targets.size(), targets.data(), depth.dsv);

	// First update the globals buffer
	auto g = FirstPass::Globals::from_world(world, rend.ctx.ratio());

	draw_objects(rend, world, assets, g, true);
}


FirstPass::Globals FirstPass::Globals::from_world(const World& world, f32 ratio) {
	return Globals{
		world.camera.get_view().transposed(),
		world.camera.get_proj(ratio).transposed(),
		world.camera.transform.translation,
	};
}

FirstPass::Globals FirstPass::Globals::from_light(const DirLight& light, const Camera& camera) {
	return Globals{
		light.get_view_mat(camera).transposed(),
		light.get_proj_mat(camera).transposed(),
		camera.transform.translation,
	};
}

FirstPass::Globals FirstPass::Globals::from_light(const SpotLight& light, const Camera& camera) {
	return Globals{
		light.get_view_mat(camera).transposed(),
		light.get_proj_mat(camera).transposed(),
		camera.transform.translation,
	};
}