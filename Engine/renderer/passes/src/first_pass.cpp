#include<renderer/passes/draw_objects.h>
#include<fstream>

Result<ObjectRenderer, RenderCreateError> ObjectRenderer::create(ID3D11Device* device)
{
	VSIL vsil;
	TRY(vsil, load_vertex(device, "shaders/vertex.cso", VERTEX_LAYOUT));

	ID3D11PixelShader* ps;
	TRY(ps, load_pixel(device, "shaders/pixel.cso"));

	ID3D11VertexShader* tess_vs;
	TRY(tess_vs, load_vertex_without_layout(device, "shaders/tesselation_vertex.cso"));

	ID3D11HullShader* hs;
	TRY(hs, load_hull(device, "shaders/hull.cso"));

	ID3D11DomainShader* ds;
	TRY(ds, load_domain(device, "shaders/domain.cso"));

	ID3D11PixelShader* refl_ps;
	TRY(refl_ps, load_pixel(device, "shaders/reflective.cso"));

	Uniform<ObjectData> object;
	TRY(object, Uniform<ObjectData>::create(device));

	Uniform<MaterialData> material;
	TRY(material, Uniform<MaterialData>::create(device));

	Uniform<DetailData> detail_data;
	TRY(detail_data, Uniform<DetailData>::create(device));

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.MaxAnisotropy = 4;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* sampler_state;
	if (FAILED(device->CreateSamplerState(&sampler_desc, &sampler_state))) {
		return FailedSamplerStateCreation;
	}

	return ok<ObjectRenderer, RenderCreateError>(ObjectRenderer{
			vsil.vs,
			ps,
			tess_vs,
			hs,
			ds,
			refl_ps,
			vsil.il,
			object,
			material,
			detail_data,
			sampler_state,
		});
}

void ObjectRenderer::clean_up() {
	vs->Release();
	ps->Release();
	tess_vs->Release();
	hs->Release();
	ds->Release();
	refl_ps->Release();

	layout->Release();

	object.clean_up();

	detail_data.clean_up();

	sampler_state->Release();
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

	D3D11_RASTERIZER_DESC rs_desc;
	ZeroMemory(&rs_desc, sizeof(D3D11_RASTERIZER_DESC));
	rs_desc.FillMode = D3D11_FILL_SOLID;
	rs_desc.CullMode = D3D11_CULL_BACK;
	rs_desc.AntialiasedLineEnable = false;
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
		rs_default,
		rs_wireframe,
		rs_cull_none,
	});
}

void FirstPass::clean_up() {
	object_renderer.clean_up();
	particle_renderer.clean_up();
	globals.clean_up();

	rs_default->Release();
	rs_wireframe->Release();
	rs_cull_none->Release();
}


void FirstPass::draw(Renderer& rend, const World& world, const AssetHandler& assets, const Viewpoint& viewpoint) {
	rend.ctx.context->ClearDepthStencilView(viewpoint.depth.dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	viewpoint.gbuffer.clear(rend.ctx.context);

	auto targets = viewpoint.gbuffer.targets();
	rend.ctx.context->OMSetRenderTargets(targets.size(), targets.data(), viewpoint.depth.dsv);

	// First update the globals buffer
	auto g = FirstPass::Globals{
		viewpoint.view,
		viewpoint.proj,
		viewpoint.pos,
	};

	draw_objects(rend, world, assets, g, true, viewpoint.objects_in_camera, Vec<Id<Reflective>>{}, viewpoint.skip_reflective);
}


FirstPass::Globals FirstPass::Globals::from_world(const World& world, f32 ratio) {
	return Globals{
		world.camera.get_view().transposed(),
		world.camera.get_proj(ratio).transposed(),
		world.camera.transform.translation,
	};
}

FirstPass::Globals FirstPass::Globals::from_light(const Light& light, const Camera& camera) {
	return Globals{
		light.get_view_mat(camera).transposed(),
		light.get_proj_mat(camera).transposed(),
		camera.transform.translation,
	};
}