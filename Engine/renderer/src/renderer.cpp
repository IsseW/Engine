#include<renderer/renderer.h>
#include<renderer/window.h>
#include<DirectXCollision.h>

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



DrawingContext DrawingContext::create(Renderer& renderer, const World& world, const AssetHandler& assets, Vec<Line>& debug_lines) {
	DrawingContext context{};

	static Transform transform{};
	static float rotation = 0.0;
	transform.rotation = Quat<f32>::angle_axis(Vec3<f32>{0,0,1.0f}, rotation);
	rotation += 0.01;

	auto camera_transform = world.camera.transform.get_mat();
	auto camera_proj = world.camera.get_proj(renderer.ctx.ratio());
	auto frustrum_planes = DirectX::BoundingFrustum{ to_direct(camera_proj) };

	DirectX::BoundingFrustum bounding_frustrum;
	auto transform_mat = to_direct(camera_transform);
	DirectX::FXMMATRIX mat{ transform_mat };
	frustrum_planes.Transform(bounding_frustrum, mat);

	auto intersected_objects_camera = world.octree_obj.collect([&](Vec3<f32> origin, f32 len) {
		auto bounding_box = DirectX::BoundingOrientedBox{};
		auto xm_vec = to_direct(origin);
		DirectX::XMFLOAT3 center;
		DirectX::XMStoreFloat3(&center, xm_vec);
		bounding_box.Center = center;
		bounding_box.Extents = DirectX::XMFLOAT3{ len, len, len };
		return bounding_frustrum.Intersects(bounding_box);
	});

	Vec<Id<Object>> objects_in_camera{};

	for (auto* obj_id : intersected_objects_camera) {
		for (auto& id : *obj_id) {
			objects_in_camera.push(id);
		}
	}

	//world.objects.values([&](const Object& obj) {
	//
	//});

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
			Vec<Id<Object>> objects_in_reflection;
			context.viewpoints.push(Viewpoint{
				objects_in_reflection,
				(mat * LOOKDIRS[i]).invert().transposed(),
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
		camera_proj.transposed(),
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

	Vec<Line> debug_lines;
	DrawingContext context = DrawingContext::create(*this, world, assets, debug_lines);

	shadow_pass.draw(*this, world, assets);

	for (const Viewpoint& v : context.viewpoints) {
		ctx.context->RSSetViewports(1, &v.viewport);
		first_pass.draw(*this, world, assets, v);

		second_pass.draw(*this, world, v);
	}

	debug_pass.draw(*this, world, debug_lines);
}

void Renderer::present() {
	ctx.swap_chain->Present(1, 0);
}