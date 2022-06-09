#include<renderer/renderer.h>
#include<renderer/rtv.h>

const float clear_color_with_alpha[4] = { 0,0,0,0 };

void clean_up_ctx(RendererCtx& ctx) {
	ctx.ds_view->Release();
	ctx.ds_texture->Release();
	ctx.rtv->Release();
	ctx.swap_chain->Release();
	ctx.context->Release();
	ctx.device->Release();
}

void Renderer::clean_up() {
	clean_up_ctx(ctx);
}

void Renderer::resize(u32 width, u32 height)  {
	if (ctx.rtv) { ctx.rtv->Release();  }
	std::cout << width << " " << height << std::endl;
	ctx.swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	ctx.rtv = create_render_target_view(ctx.device, ctx.swap_chain).unwrap();
}

void Renderer::begin_draw() {
	this->ctx.context->OMSetRenderTargets(1, &ctx.rtv, NULL);
	this->ctx.context->ClearRenderTargetView(ctx.rtv, clear_color_with_alpha);
}

void Renderer::present() {
	ctx.swap_chain->Present(1, 0);
}