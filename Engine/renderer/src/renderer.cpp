#include<renderer/renderer.h>
#include<renderer/window.h>
#include<DirectXCollision.h>
#include <unordered_set>

void RendererCtx::clean_up() {
	screen.clean_up();
	screen_depth.clean_up();
	screen_gbuffer.clean_up();

	reflection_depth.clean_up();
	reflection_gbuffer.clean_up();

	swap_chain->Release();
	context->Release();
	device->Release();
#if defined(_DEBUG)
	Sleep(1000);
	debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	debug->Release();
#endif
}

Vec2<u16> Viewpoint::size() const {
	return Vec2<u16>(viewport.Width, viewport.Height);
}

f32 RendererCtx::ratio() const {
	return viewport.Width / viewport.Height;
}

void Renderer::clean_up() {
	shadow_pass.clean_up();
	first_pass.clean_up();
	second_pass.clean_up();
	debug_pass.clean_up();
	ctx.clean_up();
}

void Renderer::resize(const Window& window, Vec2<u16> size)  {
	if (size.x <= 0 || size.y <= 0) {
		return;
	}

	ctx.resize(size);
}

void collect_to_render(const World& world, const Mat4<f32>& world_mat, const Mat4<f32>& proj_mat, bool orthographic, Vec<Id<Object>>& objects, Vec<Id<Reflective>>& reflective, Option<Id<Reflective>> skip, Option<Vec<Line>*> debug_lines) {
	auto draw_shape = [&](const auto& shape, f32 c) {
		Option<Vec<Line>*> maybe_debug_lines = debug_lines;
		maybe_debug_lines.then_do([&](Vec<Line>* debug_lines) {
			Vec3<f32> corners[8];
			shape.GetCorners((DirectX::XMFLOAT3*)corners);
			debug_lines->push(Line{ corners[0] - world.camera.transform.translation, corners[1] - world.camera.transform.translation , c });
			debug_lines->push(Line{ corners[1] - world.camera.transform.translation, corners[2] - world.camera.transform.translation , c });
			debug_lines->push(Line{ corners[2] - world.camera.transform.translation, corners[3] - world.camera.transform.translation , c });
			debug_lines->push(Line{ corners[3] - world.camera.transform.translation, corners[0] - world.camera.transform.translation , c });

			debug_lines->push(Line{ corners[0] - world.camera.transform.translation, corners[4] - world.camera.transform.translation , c });
			debug_lines->push(Line{ corners[1] - world.camera.transform.translation, corners[5] - world.camera.transform.translation , c });
			debug_lines->push(Line{ corners[2] - world.camera.transform.translation, corners[6] - world.camera.transform.translation , c });
			debug_lines->push(Line{ corners[3] - world.camera.transform.translation, corners[7] - world.camera.transform.translation , c });

			debug_lines->push(Line{ corners[4] - world.camera.transform.translation, corners[5] - world.camera.transform.translation , c });
			debug_lines->push(Line{ corners[5] - world.camera.transform.translation, corners[6] - world.camera.transform.translation , c });
			debug_lines->push(Line{ corners[6] - world.camera.transform.translation, corners[7] - world.camera.transform.translation , c });
			debug_lines->push(Line{ corners[7] - world.camera.transform.translation, corners[4] - world.camera.transform.translation , c });
		});
	};

	Vec<const Vec<Id<Object>>*> intersected_objects;
	Vec<const Vec<Id<Reflective>>*> intersected_reflective;

	auto intersect_shape = [&](const auto& shape) {
		draw_shape(shape, 1.0);

		auto collect = [&](Vec3<f32> origin, f32 len) {
			auto bounding_box = DirectX::BoundingBox{};
			auto xm_vec = to_direct(origin);
			DirectX::XMFLOAT3 center;
			DirectX::XMStoreFloat3(&center, xm_vec);
			bounding_box.Center = center;
			bounding_box.Extents = DirectX::XMFLOAT3{ len, len, len };

			bool i = shape.Intersects(bounding_box);

			draw_shape(bounding_box, i ? 0.5f : 0.0f);

			return i;
		};

		intersected_objects = world.octree_obj.collect(collect);
		debug_lines.take();
		intersected_reflective = world.octree_reflective.collect(collect);
		
		std::unordered_set<Id<Object>> objs;
		for (auto* obj_id : intersected_objects) {
			for (auto& id : *obj_id) {
				if (!objs.contains(id)) {
					objs.insert(id);
					auto object = world.objects.get(id).unwrap();
					auto& bounds = object->bounds;
					auto center = bounds.center();
					auto extents = bounds.extends();
					DirectX::BoundingOrientedBox axis_bounds{};
					axis_bounds.Center = *(DirectX::XMFLOAT3*)&center;
					axis_bounds.Extents = *(DirectX::XMFLOAT3*)&extents;
					if (shape.Intersects(axis_bounds)) {
						objects.push(id);
					}
				}
			}
		}

		std::unordered_set<Id<Reflective>> refls;
		skip.then_do([&](auto id) { refls.insert(id); });
		for (auto* obj_id : intersected_reflective) {
			for (auto& id : *obj_id) {
				if (!refls.contains(id)) {
					refls.insert(id);
					auto refl = world.reflective.get(id).unwrap();
					auto& bounds = refl->bounds;
					auto center = bounds.center();
					auto extents = bounds.extends();
					DirectX::BoundingOrientedBox axis_bounds{};
					axis_bounds.Center = *(DirectX::XMFLOAT3*)&center;
					axis_bounds.Extents = *(DirectX::XMFLOAT3*)&extents;
					if (shape.Intersects(axis_bounds)) {
						reflective.push(id);
					}
				}
			}
		}
	};

	if (orthographic) {
		f32 n = -(1.0f - proj_mat[2][3]) / proj_mat[2][2];
		f32 f = (1.0f + proj_mat[2][3]) / proj_mat[2][2];
		f32 b = -(1.0f + proj_mat[1][3]) / proj_mat[1][1];
		f32 t = (1.0f - proj_mat[1][3]) / proj_mat[1][1];
		f32 l = -(1.0f + proj_mat[0][3]) / proj_mat[0][0];
		f32 r = (1.0f - proj_mat[0][3]) / proj_mat[0][0];
		Aabb<f32> aabb { { l, b, n }, { r, t, f } };
		auto mat = *(DirectX::XMFLOAT4X4*)&world_mat;
		auto center = aabb.center();
		auto extents = aabb.extends();
		DirectX::BoundingOrientedBox axis_bounds{};
		axis_bounds.Center = *(DirectX::XMFLOAT3*)&center;
		axis_bounds.Extents = *(DirectX::XMFLOAT3*)&extents;

		DirectX::BoundingOrientedBox bounds{};
		axis_bounds.Transform(bounds, to_direct(world_mat));
		intersect_shape(bounds);
	}
	else {
		auto frustrum_planes = DirectX::BoundingFrustum{ to_direct(proj_mat) };
		DirectX::BoundingFrustum bounding_frustrum {};
		frustrum_planes.Transform(bounding_frustrum, to_direct(world_mat));
		intersect_shape(bounding_frustrum);
	}
}


DrawingContext DrawingContext::create(Renderer& renderer, const World& world, const AssetHandler& assets, const Input& input, Vec<Line>& debug_lines) {
	DrawingContext context{};
	static Mat4<f32> camera_world_mat;
	static Mat4<f32> camera_proj_mat;
	static bool is_orth;
	static bool update_frustrum = true;
	if (input.just_pressed(Key::G)) {
		update_frustrum = !update_frustrum;
	}
	auto camera_transform = world.camera.transform.get_mat();
	auto camera_proj = world.camera.get_proj(renderer.ctx.ratio());
	if (update_frustrum) {
		camera_world_mat = camera_transform;
		camera_proj_mat = camera_proj;
		is_orth = !world.camera.is_perspective;
	}

	Vec<Id<Object>> objects_in_camera{};
	Vec<Id<Reflective>> reflective_in_camera{};

	collect_to_render(world, camera_world_mat, camera_proj_mat, is_orth, objects_in_camera, reflective_in_camera, {}, update_frustrum ? none<Vec<Line>*>() : some(&debug_lines));

	std::unordered_set<Id<Reflective>> rendered;
	Vec<Id<Reflective>> to_render = reflective_in_camera;

	while (to_render.len() > 0) {
		Id<Reflective> id = to_render.pop().unwrap_unchecked();
		if (rendered.contains(id)) {
			continue;
		}
		rendered.insert(id);

		const Reflective& reflective = *world.reflective.get(id).unwrap();

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
		auto proj = math::create_persp_proj(left, right, bottom, top, n, 30.0f);
		for (usize i = 0; i < 6; ++i) {
			Vec<Id<Object>> objects_in_reflection;
			Vec<Id<Reflective>> reflective_in_reflection;
			auto world_mat = (mat * LOOKDIRS[i]);

			collect_to_render(world, world_mat, proj, false, objects_in_reflection, reflective_in_reflection, some(id));

			to_render.extend(reflective_in_reflection);
			auto vp = Viewpoint{
				objects_in_reflection,
				reflective_in_reflection,
				world_mat.invert().transposed(),
				proj.transposed(),
				reflective.transform.translation,
				reflective.cube_texture.uav[i],
				renderer.ctx.reflection_depth,
				renderer.ctx.reflection_gbuffer,
				renderer.ctx.reflection_viewport,
			};
			context.viewpoints.push(vp);
		}
	}

	context.viewpoints.push(Viewpoint{
		objects_in_camera,
		reflective_in_camera,
		world.camera.get_view().transposed(),
		camera_proj.transposed(),
		world.camera.transform.translation,
		renderer.ctx.screen.uav,
		renderer.ctx.screen_depth,
		renderer.ctx.screen_gbuffer,
		renderer.ctx.viewport,
	});

	return context;
}

void Renderer::draw(const World& world, AssetHandler& assets, const Input& input, f32 delta) {
	first_pass.particle_renderer.run(*this, world, delta);

	assets.default_asset<Image>()->bind(ctx.device);
	world.objects.values([&](const Object& obj) {
		assets.get_or_default(obj.mesh)->bind(ctx.device, assets);
	});
	world.reflective.values([&](const Reflective& obj) {
		assets.get_or_default(obj.mesh)->bind(ctx.device, assets);
	});

	Vec<Line> debug_lines;
	DrawingContext context = DrawingContext::create(*this, world, assets, input, debug_lines);

	shadow_pass.draw(*this, world, assets);

	for (const Viewpoint& v : context.viewpoints) {
		ctx.context->RSSetViewports(1, &v.viewport);
		first_pass.draw(*this, world, assets, v);

		second_pass.draw(*this, world, v);
	}

	if (draw_debug) {
		debug_pass.draw(*this, world, debug_lines);
	}

	ctx.context->ClearState();
}

void Renderer::present() {
	ctx.swap_chain->Present(1, 0);
}