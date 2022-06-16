#include<renderer/renderer.h>
#include<renderer/rtv.h>
#include<renderer/window.h>

const float clear_color_with_alpha[4] = { 0,0,0,0 };

void clean_up_ctx(RendererCtx& ctx) {
	ctx.ds_view->Release();
	ctx.ds_texture->Release();
	ctx.rtv->Release();
	ctx.swap_chain->Release();
	ctx.context->Release();
	ctx.device->Release();
}

void clean_up_object_renderer(ObjectRenderer& object_renderer) {
	object_renderer.layout->Release();
	object_renderer.ps->Release();
	object_renderer.vs->Release();
}

void clean_up_first_pass(FirstPass& first_pass) {
	first_pass.globals.buffer->Release();
	clean_up_object_renderer(first_pass.object_renderer);
}


void Renderer::clean_up() {
	clean_up_ctx(ctx);
	clean_up_first_pass(first_pass);
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

Globals Globals::from_world(const World& world, u32 width, u32 height) {
	return Globals{
		world.camera.get_proj(width, height),
		world.camera.get_view()
	};
}

void Renderer::draw_first_pass(const Window* window, const World& world, const AssetHandler& assets) {
	// First update the globals buffer
	auto globals = Globals::from_world(world, window->width(), window->height());
	first_pass.globals.update(ctx.context, &globals);

	for (const Object& obj : world.objects) {
		auto mesh = assets.get(obj.mesh).unwrap();
	}
}

void Renderer::present() {
	ctx.swap_chain->Present(1, 0);
}