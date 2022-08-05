#include<renderer/renderer.h>
#include<renderer/window.h>

void RendererCtx::clean_up() {
	screen.clean_up();
	screen_depth.clean_up();
	screen_gbuffer.clean_up();

	reflection_depth.clean_up();
	screen_gbuffer.clean_up();

	swap_chain->Release();
	context->Release();
	device->Release();
}

Vec2<u16> Viewpoint::size() const {
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
}

DrawingContext DrawingContext::create(Renderer& renderer, const World& world, const AssetHandler& assets) {
	DrawingContext context{};

	Vec<Id<Object>> objects_in_camera{};

	world.objects.values([&](const Object& obj) {
		// TODO: collect info from scene.
	});


	world.reflective.iter([&](Id<Reflective> id, const Reflective& reflective) {
		constexpr Mat4<f32> LOOKDIRS[6] = {
			Mat4<f32> {Vec4<Vec4<f32>> {
				Vec4<f32> {0.0, 0.0, 1.0, 0.0},
				Vec4<f32> {0.0, 1.0, 0.0, 0.0},
				Vec4<f32> {-1.0, 0.0, 0.0, 0.0},
				Vec4<f32> {0.0, 0.0, 0.0, 1.0},
			}},
			Mat4<f32> {Vec4<Vec4<f32>> {
				Vec4<f32> {0.0, 0.0, -1.0, 0.0},
				Vec4<f32> {0.0, 1.0, 0.0, 0.0},
				Vec4<f32> {1.0, 0.0, 0.0, 0.0},
				Vec4<f32> {0.0, 0.0, 0.0, 1.0},
			}},
			Mat4<f32> {Vec4<Vec4<f32>> {
				Vec4<f32> {1.0, 0.0, 0.0, 0.0},
				Vec4<f32> {0.0, 0.0, 1.0, 0.0},
				Vec4<f32> {0.0, -1.0, 0.0, 0.0},
				Vec4<f32> {0.0, 0.0, 0.0, 1.0},
			}},
			Mat4<f32> {Vec4<Vec4<f32>> {
				Vec4<f32> {1.0, 0.0, 0.0, 0.0},
				Vec4<f32> {0.0, 0.0, -1.0, 0.0},
				Vec4<f32> {0.0, 1.0, 0.0, 0.0},
				Vec4<f32> {0.0, 0.0, 0.0, 1.0},
			}},
			Mat4<f32> {Vec4<Vec4<f32>> {
				Vec4<f32> {1.0, 0.0, 0.0, 0.0},
				Vec4<f32> {0.0, 1.0, 0.0, 0.0},
				Vec4<f32> {0.0, 0.0, 1.0, 0.0},
				Vec4<f32> {0.0, 0.0, 0.0, 1.0},
			}},
			Mat4<f32> {Vec4<Vec4<f32>> {
				Vec4<f32> {-1.0, 0.0, 0.0, 0.0},
				Vec4<f32> {0.0, 1.0, 0.0, 0.0},
				Vec4<f32> {0.0, 0.0, -1.0, 0.0},
				Vec4<f32> {0.0, 0.0, 0.0, 1.0},
			}},
		};

		auto mat = reflective.transform.get_mat();

		f32 n = 0.01f;
		// tanf(PI / 2 / 2) = 1
		f32 top = 1.0f * n;
		f32 bottom = -top;
		f32 right = top;
		f32 left = -right;
		auto proj = math::create_persp_proj(left, right, bottom, top, n, 100.0f).transposed();
		for (usize i = 0; i < 6; ++i) {
			auto view = (mat * LOOKDIRS[i]).invert().transposed();
			context.viewpoints.push(Viewpoint{
				objects_in_camera,
				view,
				proj,
				reflective.transform.translation,
				reflective.cube_texture.uav[i],
				renderer.ctx.reflection_depth,
				renderer.ctx.reflection_gbuffer,
				renderer.ctx.reflection_viewport,
				some(id),
			});
		}
	});

	context.viewpoints.push(Viewpoint{
		objects_in_camera,
		world.camera.get_view().transposed(),
		world.camera.get_proj(renderer.ctx.ratio()).transposed(),
		world.camera.transform.translation,
		renderer.ctx.screen.uav,
		renderer.ctx.screen_depth,
		renderer.ctx.screen_gbuffer,
		renderer.ctx.viewport,
		none<Id<Reflective>>(),
	});

	return context;
}

void Renderer::draw(const World& world, AssetHandler& assets, f32 delta) {
	first_pass.particle_renderer.run(*this, world, delta);

	assets.default_asset<Image>()->bind(ctx.device);
	world.objects.values([&](const Object& obj) {
		assets.get_or_default(obj.mesh)->bind(ctx.device, assets);
	});
	world.reflective.values([&](const Reflective& obj) {
		assets.get_or_default(obj.mesh)->bind(ctx.device, assets);
	});

	DrawingContext context = DrawingContext::create(*this, world, assets);

	shadow_pass.draw(*this, world, assets);

	for (const Viewpoint& v : context.viewpoints) {
		ctx.context->RSSetViewports(1, &v.viewport);
		first_pass.draw(*this, world, assets, v);

		second_pass.draw(*this, world, v);
	}
}

void Renderer::present() {
	ctx.swap_chain->Present(1, 0);
}