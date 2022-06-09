#include<renderer/renderer.h>

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

void Renderer::present() {
	ctx.swap_chain->Present(0, 0);
}