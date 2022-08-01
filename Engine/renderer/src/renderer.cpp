#include<renderer/renderer.h>
#include<renderer/window.h>

void RendererCtx::clean_up() {
	screen.clean_up();

	swap_chain->Release();
	context->Release();
	device->Release();
}

Vec2<u16> RendererCtx::size() const {
	return Vec2<u16>(viewport.Width, viewport.Height);
}

f32 RendererCtx::ratio() const {
	return viewport.Width / viewport.Height;
}

void Renderer::clean_up() {
	ctx.clean_up();
	shadow_pass.clean_up();
	first_pass.clean_up();
	second_pass.clean_up();
}

void Renderer::resize(const Window& window, Vec2<u16> size)  {
	if (size.x <= 0 || size.y <= 0) {
		return;
	}

	ctx.resize(size);
	// Do we want to do this? Might be unrelated to screen size. Otherwise will probably need more thinking of how we choose a size for the shadow texture
	// shadow_pass.resize(ctx.device, size);

	first_pass.resize(ctx.device, size);
}

DrawingContext DrawingContext::create(const World& world, const AssetHandler& assets) {
	Vec<Id<Object>> objects_in_camera{};

	world.objects.values([&](const Object& obj) {
		// TODO: collect info from scene.
	});

	TODO;
}

void Renderer::draw(const World& world, AssetHandler& assets) {
	assets.default_asset<Image>()->bind(ctx.device);
	world.objects.values([&](const Object& obj) {
		assets.get_or_default(obj.mesh)->bind(ctx.device, assets);
	});

	// DrawingContext context = DrawingContext::create(world, assets);

	ctx.context->RSSetViewports(1, &ctx.viewport);

	shadow_pass.draw(*this, world, assets);

	first_pass.draw(*this, world, assets);

	second_pass.draw(*this, world);
}

void Renderer::present() {
	ctx.swap_chain->Present(1, 0);
}