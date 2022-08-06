#include<renderer/passes/debug_pass.h>
#include<renderer/renderer.h>

constexpr usize LEN = 80;

Result<ID3D11Buffer*, RenderCreateError> create_buf(ID3D11Device* device) {
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(Vec4<f32>) * LEN;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	ID3D11Buffer* buffer;
	if (FAILED(device->CreateBuffer(&desc, nullptr, &buffer))) {
		return FailedBufferCreation;
	}

	return ok<ID3D11Buffer*, RenderCreateError>(buffer);
}

void DebugPass::draw(Renderer& rend, const World& world, Vec<Line> lines) {
	auto* ctx = rend.ctx.context;

	auto globals = FirstPass::Globals::from_world(world, rend.ctx.ratio());
	rend.first_pass.globals.update(ctx, &globals);

	ctx->PSSetShader(this->ps, nullptr, 0);
	ctx->VSSetShader(this->vs, nullptr, 0);
	ctx->IASetInputLayout(this->il);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	ctx->VSSetConstantBuffers(0, 1, &rend.first_pass.globals.buffer);
	u32 stride = sizeof(Vec4<f32>);
	u32 offset = 0;
	ctx->IASetVertexBuffers(0, 1, &this->vtx_buf, &stride, &offset);
	
	ctx->OMSetRenderTargets(1, &rend.ctx.screen.rtv, nullptr);

	for (usize i = 0; i < lines.len(); i += LEN/2) {
		D3D11_MAPPED_SUBRESOURCE res;
		ctx->Map(this->vtx_buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		usize count = min(lines.len() - i, LEN / 2);
		Line* data = &lines[i];
		memcpy(res.pData, data, count*sizeof(Line));
		ctx->Unmap(this->vtx_buf, 0);
		ctx->Draw(count*2, 0);
	}
	
	ctx->PSSetShader(nullptr, nullptr, 0);
}

Result<DebugPass, RenderCreateError> DebugPass::create(ID3D11Device* device) {
	VSIL vsil;
	TRY(vsil, load_vertex(device, "shaders/debug_vertex.cso", { { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } }));
	
	ID3D11PixelShader* ps;
	TRY(ps, load_pixel(device, "shaders/debug_pixel.cso"));
	
	ID3D11Buffer* vtx_buf;
	TRY(vtx_buf, create_buf(device));

	return ok<DebugPass, RenderCreateError>(
		DebugPass {
			vsil.vs, vsil.il, ps, vtx_buf,
		}
	);
}

void DebugPass::clean_up() {
	this->il->Release();
	this->ps->Release();
	this->vs->Release();
	this->vtx_buf->Release();
}