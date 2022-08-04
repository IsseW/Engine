#include"imgui/imgui.h"
#include"imgui/imgui_impl_win32.h"
#include"imgui/imgui_impl_dx11.h"
#include"ui.h"
#include<sstream>
#include<unordered_set>
#include<math/consts.h>
#include<renderer/window.h>

bool capturing_keyboard() {
	return ImGui::GetIO().WantCaptureKeyboard;
}
bool capturing_mouse() {
	return ImGui::GetIO().WantCaptureMouse;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void setup_ui(const Window* window, Renderer& renderer) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO();
	

	// Prevents Dear ImGui from creating an .ini file.
	ImGui::SaveIniSettingsToDisk(NULL);

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window->window());
	ImGui_ImplWin32_EnableDpiAwareness();
	ImGui_ImplDX11_Init(renderer.ctx.device, renderer.ctx.context);
}

void start() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void end() {
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool edit(const char* label, Vec2<f32>& v) {
	return ImGui::DragFloat2(label, v.data());
}

bool edit(const char* label, Vec3<f32>& v) {
	return ImGui::DragFloat3(label, v.data());
}

bool edit(const char* label, Vec4<f32>& v) {
	return ImGui::DragFloat4(label, v.data());
}

bool edit(const char* label, Mat3<f32>& v) {
	if (ImGui::CollapsingHeader(label)) {
		bool edited = false;
		ImGui::Indent();
		std::string s = label;
		s += " row 0";
		edited |= edit(s.data(), v[0]);
		s = label;
		s += " row 1";
		edited |= edit(s.data(), v[1]);
		s = label;
		s += " row 2";
		edited |= edit(s.data(), v[2]);
		ImGui::Unindent();
		return edited;
	}
	return false;
}

bool edit(const char* label, Mat4<f32>& v) {
	if (ImGui::CollapsingHeader(label)) {
		bool edited = false;
		ImGui::Indent();
		std::string s = label;
		s += " row 0";
		edited |= edit(s.data(), v[0]);
		s = label;
		s += " row 1";
		edited |= edit(s.data(), v[1]);
		s = label;
		s += " row 2";
		edited |= edit(s.data(), v[2]);
		s += " row 3";
		edited |= edit(s.data(), v[3]);
		ImGui::Unindent();
		return edited;
	}
	return false;
}

bool edit(const char* label, Quat<f32>& quat) {
	auto euler = quat.to_euler() * F32::TO_DEG;

	if (edit("Rotation", euler)) {
		quat = Quat<f32>::from_euler(euler * F32::TO_RAD);
		return true;
	}
	return false;
}

bool edit(Transform& transform) {
	bool e = edit("Trans", transform.translation);
	e |= edit("Scale", transform.scale);
	e |= edit("Rotation", transform.rotation);

	return e;
}

bool edit_vertices(Option<Mesh*> mesh, Mat4<f32> mat) {
	bool edited = false;
	ImGui::Indent();
	mesh.then_do([&](Mesh* mesh) {
		auto& vertices = mesh->vertices;
		for (usize j = 0; j < vertices.len(); ++j) {
			ImGui::PushID(j * mesh->submeshes.len());
			auto t = mat.transform_point(vertices[j].v);
			edited |= edit("vertex", t);
			ImGui::PopID();
		}
	});
	ImGui::Unindent();
	return edited;
}


template<typename T>
void select_asset(const char* name, Option<AId<T>>& asset_id, AssetHandler& assets) {
	if (ImGui::BeginListBox(name)) {
		if (ImGui::Selectable("None", asset_id.is_none())) {
			asset_id = none<AId<T>>();
		}
		assets.assets<T>().iter([&](AId<T> id, const Asset<T>& asset) {
			std::string s = id.to_string();
			s += " ";
			s += asset.path.as_ptr().map<const char*>([](const std::string* s) { return s->data(); }).unwrap_or("Unnamed");

			if (ImGui::Selectable(s.data(), asset_id.is_some() && *asset_id.as_ptr().unwrap_unchecked() == id)) {
				asset_id = some(id);
			}
		});
		ImGui::EndListBox();
	}
}


void light_ui(Light& light) {
	edit(light.transform);
	ImGui::ColorEdit3("Color", light.color.data());

	if (ImGui::Selectable("Directional", light.light_type == LightType::Directional)) {
		light.light_type = LightType::Directional;
	}

	if (ImGui::Selectable("Spot", light.light_type == LightType::Spot)) {
		light.light_type = LightType::Spot;
	}

	if (light.light_type == LightType::Spot) {
		ImGui::SliderAngle("Angle", &light.angle, 0.0f, 80.0f);
	}
}

template<typename T>
void object_ui(T& obj, AssetHandler& assets) {
	edit(obj.transform);

	select_asset("Mesh", obj.mesh, assets);

	ImGui::Checkbox("Tesselate", &obj.tesselate);
}

void material_ui(Material& mat, Renderer& renderer, AssetHandler& assets) {
	mat.bind(renderer.ctx.device, assets);

	auto size = ImVec2(100, 100);

	auto col = mat.ambient.get_color();
	auto image = assets.get_or_default(mat.ambient.tex)->binded.as_ptr().unwrap_unchecked()->srv;
	ImGui::Text("Ambient: ");
	select_asset("Ambient", mat.ambient.tex, assets);
	ImGui::SameLine();
	ImGui::Image(image, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(col.x, col.y, col.z, 1.0f));
	if (mat.ambient.tex.is_none()) {
		ImGui::ColorEdit3("Ambient", mat.ambient.color.data());
	}

	col = mat.diffuse.get_color();
	image = assets.get_or_default(mat.diffuse.tex)->binded.as_ptr().unwrap_unchecked()->srv;
	ImGui::Text("Diffuse: ");
	select_asset("Diffuse", mat.diffuse.tex, assets);
	ImGui::SameLine();
	ImGui::Image(image, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(col.x, col.y, col.z, 1.0f));
	if (mat.diffuse.tex.is_none()) {
		ImGui::ColorEdit3("Diffuse", mat.diffuse.color.data());
	}

	col = mat.specular.get_color();
	image = assets.get_or_default(mat.specular.tex)->binded.as_ptr().unwrap_unchecked()->srv;
	ImGui::Text("Specular: ");
	select_asset("Specular", mat.specular.tex, assets);
	ImGui::SameLine();
	ImGui::Image(image, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(col.x, col.y, col.z, 1.0f));
	if (mat.specular.tex.is_none()) {
		ImGui::ColorEdit3("Specular", mat.specular.color.data());
	}

	ImGui::DragFloat("Shininess", &mat.shininess);
}

void editor_ui(const Window& window, Renderer& renderer, World& world, AssetHandler& assets) {
	static bool world_active = true;
	if (ImGui::Begin("World Editor", &world_active)) {
		auto view_mat = world.camera.get_view();
		auto view_mat_inv = view_mat.invert();
		auto proj_mat = world.camera.get_proj(window.ratio());
		auto proj_mat_inv = proj_mat.invert();
		if (ImGui::CollapsingHeader("Camera")) {
			ImGui::PushID(0);
			ImGui::Indent();
			edit("Position", world.camera.transform.translation);
			Vec2<f32> rot = Vec2<f32>(world.camera.pitch, world.camera.yaw) * F32::TO_DEG;
			edit("Rotation", rot);
			world.camera.pitch = rot.x * F32::TO_RAD;
			world.camera.yaw = rot.y * F32::TO_RAD;

			// UB?
			ImGui::DragFloat2("near, far", &world.camera.cam_near);

			ImGui::Checkbox("Perspective", &world.camera.is_perspective);

			if (world.camera.is_perspective) {
				auto deg = world.camera.fov * F32::TO_DEG;
				if (ImGui::DragFloat("fov", &deg)) {
					world.camera.fov = std::round(deg) * F32::TO_RAD;
				}
			}
			else {
				ImGui::DragFloat("size", &world.camera.size);
			}

			if (ImGui::CollapsingHeader("Matrices")) {
				ImGui::Indent();
				edit("view_mat", view_mat);
				edit("view_mat_inv", view_mat_inv);
				edit("proj_mat", proj_mat);
				edit("proj_mat_inv", proj_mat_inv);
				ImGui::Unindent();
			}
			ImGui::Unindent();
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Objects")) {
			ImGui::PushID(1);
			ImGui::Indent();
			world.objects.iter([&](Id<Object> id, Object& obj) {
				std::string s("Object.");
				s += id.to_string();
				if (ImGui::CollapsingHeader(s.data())) {
					ImGui::PushID(id.idx());
					ImGui::Indent();

					object_ui(obj, assets);

					auto mat = obj.transform.get_mat();

					if (ImGui::Button("Remove")) {
						world.remove(id);
					}

					ImGui::Unindent();
					ImGui::PopID();
				}
				});
			if (ImGui::CollapsingHeader("New Object")) {
				static Object new_object = Object(Transform());
				ImGui::Indent();
				object_ui(new_object, assets);

				if (ImGui::Button("Add")) {
					world.add(std::move(new_object));
					new_object = Object(Transform());
				}
				ImGui::Unindent();
			}
			ImGui::Unindent();
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Reflective Objects")) {
			ImGui::PushID(1);
			ImGui::Indent();
			world.reflective.iter([&](Id<Reflective> id, Reflective& obj) {
				std::string s("Object.");
				s += id.to_string();
				if (ImGui::CollapsingHeader(s.data())) {
					ImGui::PushID(id.idx());
					ImGui::Indent();

					object_ui(obj, assets);

					auto mat = obj.transform.get_mat();

					if (ImGui::Button("Remove")) {
						world.remove(id);
					}

					ImGui::Unindent();
					ImGui::PopID();
				}
				});
			if (ImGui::CollapsingHeader("New Object")) {
				static Reflective new_object = Reflective(Transform());
				ImGui::Indent();
				object_ui(new_object, assets);

				if (ImGui::Button("Add")) {
					new_object.create_texture(renderer.ctx.device);
					world.add(std::move(new_object));
					new_object = Reflective(Transform());
				}
				ImGui::Unindent();
			}
			ImGui::Unindent();
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Lights")) {
			ImGui::PushID(2);
			ImGui::Indent();
			usize index = 0;
			world.lights.iter([&](Id<Light> id, Light& light) {
				std::string s("Light.");
				s += id.to_string();
				if (ImGui::CollapsingHeader(s.data())) {
					ImGui::PushID(id.idx());
					ImGui::Indent();

					light_ui(light);

					if (ImGui::CollapsingHeader("Shadow")) {
						ImGui::Indent();
						auto size = renderer.shadow_pass.shadows.size.as<f32>().xy().normalized();
						ImGui::Image(renderer.shadow_pass.shadows.srvs[index], ImVec2(300.0f * size.x / size.y, 300.0f));

						ImGui::Unindent();
					}


					if (ImGui::Button("Remove")) {
						world.remove(id);
					}

					ImGui::Unindent();
					ImGui::PopID();
				}
				++index;
			});

			if (ImGui::CollapsingHeader("New Light")) {
				static Light light = Light{};
				ImGui::Indent();

				light_ui(light);

				if (ImGui::Button("Add")) {
					world.add(std::move(light));
					light = Light{};
				}

				ImGui::Unindent();
			}
			ImGui::Unindent();
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Particle Systems")) {
			ImGui::PushID(3);
			ImGui::Indent();
			world.particle_systems.iter([&](Id<ParticleSystem> id, ParticleSystem& system) {
				ImGui::PushID(id.idx());
				if (ImGui::CollapsingHeader(id.to_string().data())) {
					edit(system.transform);

					select_asset("Material", system.material, assets);

					if (ImGui::Button(system.paused ? "Play" : "Pause")) {
						system.paused = !system.paused;
					}
				}
				ImGui::PopID();
			});
			ImGui::Unindent();
			ImGui::PopID();
		}
	}
	ImGui::End();

	static bool assets_active = true;
	if (ImGui::Begin("Assets editor", &assets_active)) {
		if (ImGui::CollapsingHeader("Meshes")) {
			ImGui::Indent();

			assets.assets<Mesh>().iter([&](AId<Mesh> id, Asset<Mesh>& asset) {
				ImGui::PushID(id.idx());

				std::string s = id.to_string();
				s += " ";
				s += asset.path.as_ptr().map<const char*>([](const std::string* s) { return s->data(); }).unwrap_or("unnamed");

				if (ImGui::CollapsingHeader(s.data())) {
					ImGui::Indent();

					if (ImGui::CollapsingHeader("Submeshes")) {
						ImGui::Indent();

						for (usize i = 0; i < asset.asset.submeshes.len(); ++i) {
							ImGui::PushID(i);
							std::string id = std::to_string(i);
							if (ImGui::CollapsingHeader(id.data())) {
								ImGui::Indent();
								if (ImGui::CollapsingHeader("Material")) {
									ImGui::Indent();

									select_asset("Material", asset.asset.submeshes[i].material, assets);

									ImGui::Unindent();
								}
								ImGui::Unindent();
							}
							ImGui::PopID();
						}

						ImGui::Unindent();
					}

					ImGui::Unindent();
				}
				ImGui::PopID();
				});
			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("Materials")) {
			ImGui::Indent();

			assets.assets<Material>().iter([&](AId<Material> id, Asset<Material>& asset) {
				ImGui::PushID(id.idx());
				std::string s = id.to_string();
				s += " ";
				s += asset.asset.name;

				if (ImGui::CollapsingHeader(s.data())) {
					ImGui::Indent();

					material_ui(asset.asset, renderer, assets);

					ImGui::Unindent();
				}

				ImGui::PopID();
				});

			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("Textures")) {
			ImGui::Indent();

			assets.assets<Image>().iter([&](AId<Image> id, Asset<Image>& asset) {
				ImGui::PushID(id.idx());
				std::string s = id.to_string();
				s += " ";
				s += asset.path.as_ptr().map<const char*>([](const std::string* s) { return s->data(); }).unwrap_or("unnamed");

				if (ImGui::CollapsingHeader(s.data())) {
					ImGui::Indent();

					asset.asset.bind(renderer.ctx.device);
					auto image = asset.asset.binded.as_ptr().unwrap_unchecked()->srv;
					float ratio = (f32)asset.asset.width / (f32)asset.asset.height;
					ImGui::Image(image, ImVec2(300.0f * ratio, 300.0f));

					ImGui::Unindent();
				}

				ImGui::PopID();
				});

			ImGui::Unindent();
		}
	}
	ImGui::End();

}

void update_ui(const Window& window, Renderer& renderer, World& world, AssetHandler& assets, f32 fps, f32 average_frametime) {
	start();

	ImGui_ImplWin32_GetDpiScaleForHwnd(window.window());

	static bool display_info = false;
	static bool display_renderer = true;
	static bool display_editor = true;

	ImGui::BeginMainMenuBar();
	if (ImGui::Button("Toggle Info")) {
		display_info = !display_info;
	}
	if (ImGui::Button("Toggle Editor")) {
		display_editor = !display_editor;
	}
	if (ImGui::Button("Toggle Renderer")) {
		display_renderer = !display_renderer;
	}
	ImGui::Text("fps: %f | frame time: %fms", fps, average_frametime);
	ImGui::EndMainMenuBar();

	if (display_info) {
		if (ImGui::Begin("Info")) {
			ImGui::Text("Move with w, a, s, d, space and left shift");
			ImGui::Text("Speed boost with left control");
			ImGui::Text("Rotate camera with mouse");
		}
		ImGui::End();
	}

	static bool renderer_open = false;
	if (display_renderer) {
		if (ImGui::Begin("Renderer", &renderer_open)) {
			if (ImGui::BeginListBox("Render Mode")) {

				for (usize i = 0; i < NUM_MODES; ++i) {
					if (ImGui::Selectable(MODES[i], (usize)renderer.second_pass.mode == i)) {
						renderer.second_pass.mode = (RenderMode)i;
					}
				}

				ImGui::EndListBox();
			}
			if (ImGui::Button("Toggle Wireframe")) {
				auto temp = renderer.first_pass.rs_default;
				renderer.first_pass.rs_default = renderer.first_pass.rs_wireframe;
				renderer.first_pass.rs_wireframe = temp;
			}
		}
		ImGui::End();
	}
	if (display_editor) {
		editor_ui(window, renderer, world, assets);
	}

	end();
}

bool ui_handle_input(HWND window, UINT msg, WPARAM wp, LPARAM lp) {
	return ImGui_ImplWin32_WndProcHandler(window, msg, wp, lp);
}

void clean_up_ui() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

