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
	if (edit(label, euler)) {
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

bool edit(Light& light) {
	bool e = ImGui::ColorEdit3("Color", light.color.data());
	e |= ImGui::DragFloat("Strength", &light.strength);

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



void object_ui(Object& obj, AssetHandler& assets) {
	edit(obj.transform);

	if (ImGui::BeginListBox("Mesh")) {
		assets.assets<Mesh>().iter([&](AId<Mesh> id, const Asset<Mesh>& asset) {
			std::string s = id.to_string();
			s += " ";
			s += asset.path.as_ptr().map<const char*>([](const std::string* s) { return s->data(); }).unwrap_or("unnamed");

			if (ImGui::Selectable(s.data(), obj.mesh.is_some() && *obj.mesh.as_ptr().unwrap_unchecked() == id)) {
				obj.mesh = some(id);
			}
		});
		ImGui::EndListBox();
	}
}

void dir_light_ui(DirLight& light) {
	edit(light.transform);
	edit(light.light);
}

void spot_light_ui(SpotLight& light) {
	edit(light.transform);
	edit(light.light);
}

void material_ui(Material& mat, Renderer& renderer, AssetHandler& assets) {
	mat.bind(renderer.ctx.device, assets);

	auto size = ImVec2(100, 100);

	auto col = mat.ambient.get_color();
	auto image = assets.get_or_default(mat.ambient.tex)->binded.as_ptr().unwrap_unchecked()->srv;
	ImGui::Text("Ambient: ");
	ImGui::Image(image, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(col.x, col.y, col.z, 1.0f));

	col = mat.diffuse.get_color();
	image = assets.get_or_default(mat.diffuse.tex)->binded.as_ptr().unwrap_unchecked()->srv;
	ImGui::Text("Diffuse: ");
	ImGui::Image(image, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(col.x, col.y, col.z, 1.0f));

	col = mat.specular.get_color();
	image = assets.get_or_default(mat.specular.tex)->binded.as_ptr().unwrap_unchecked()->srv;
	ImGui::Text("Specular: ");
	ImGui::Image(image, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(col.x, col.y, col.z, 1.0f));

	auto val = mat.shinyness.get_value();
	image = assets.get_or_default(mat.specular.tex)->binded.as_ptr().unwrap_unchecked()->srv;
	ImGui::Text("Shinyness: ");
	ImGui::Image(image, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(val, val, val, 1.0f));
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

					obj.mesh.as_ptr().then_do([&](AId<Mesh>* mesh) {
						if (ImGui::CollapsingHeader("Vertices")) {
							ImGui::Indent();
							if (ImGui::CollapsingHeader("Local")) {
								edit_vertices(assets.get(*mesh), Mat4<f32>::identity());
							}
							if (ImGui::CollapsingHeader("Global")) {
								edit_vertices(assets.get(*mesh), mat);
							}
							if (ImGui::CollapsingHeader("View")) {
								edit_vertices(assets.get(*mesh), mat * view_mat);
							}
							if (ImGui::CollapsingHeader("Proj")) {
								edit_vertices(assets.get(*mesh), mat * view_mat * proj_mat);
							}
							ImGui::Unindent();
						}
						});

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

		if (ImGui::CollapsingHeader("Dir Lights")) {
			ImGui::PushID(2);
			ImGui::Indent();
			usize index = 0;
			world.dir_lights.iter([&](Id<DirLight> id, DirLight& light) {
				std::string s("Light.");
				s += id.to_string();
				if (ImGui::CollapsingHeader(s.data())) {
					ImGui::PushID(id.idx());
					ImGui::Indent();

					dir_light_ui(light);

					if (ImGui::CollapsingHeader("Shadow")) {
						ImGui::Indent();
						auto size = renderer.shadow_pass.directional_shadows.size.as<f32>().xy().normalized();
						ImGui::Image(renderer.shadow_pass.directional_shadows.srvs[index], ImVec2(300.0f * size.x / size.y, 300.0f));

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
				static DirLight light = DirLight{};
				ImGui::Indent();

				dir_light_ui(light);

				if (ImGui::Button("Add")) {
					world.add(std::move(light));
					light = DirLight{};
				}

				ImGui::Unindent();
			}
			ImGui::Unindent();
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Spot Lights")) {
			ImGui::PushID(3);
			ImGui::Indent();
			usize index = 0;
			world.spot_lights.iter([&](Id<SpotLight> id, SpotLight& light) {
				std::string s("Light.");
				s += id.to_string();
				if (ImGui::CollapsingHeader(s.data())) {
					ImGui::PushID(id.idx());
					ImGui::Indent();

					spot_light_ui(light);

					if (ImGui::CollapsingHeader("Shadow")) {
						ImGui::Indent();
						auto size = renderer.shadow_pass.spot_shadows.size.as<f32>().xy().normalized();
						ImGui::Image(renderer.shadow_pass.spot_shadows.srvs[index], ImVec2(300.0f * size.x / size.y, 300.0f));

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
				static SpotLight light = SpotLight{};
				ImGui::Indent();

				spot_light_ui(light);

				if (ImGui::Button("Add")) {
					world.add(std::move(light));
					light = SpotLight{};
				}

				ImGui::Unindent();
			}
			ImGui::Unindent();
			ImGui::PopID();
		}
	}
	ImGui::End();

	static bool assets_active = true;
	if (ImGui::Begin("Assets editor", &assets_active)) {
		if (ImGui::CollapsingHeader("Meshes")) {
			ImGui::Indent();

			assets.assets<Mesh>().iter([&](AId<Mesh> id, const Asset<Mesh>& asset) {
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
									const char* s = asset.asset.submeshes[i].material.as_ptr().map<const char*>([&](const AId<Material>* mat_handle) {
										const Material* mat = assets.get(*mat_handle).unwrap();
										return mat->name.data();
										}).unwrap_or("No Material");
										ImGui::Text("Material: %s", s);
										Material* mat = assets.get_or_default(asset.asset.submeshes[i].material);

										material_ui(*mat, renderer, assets);

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
					ImGui::Image(image, ImVec2(100, 100));

					ImGui::Unindent();
				}

				ImGui::PopID();
				});

			ImGui::Unindent();
		}
	}
	ImGui::End();

}

void update_ui(const Window& window, Renderer& renderer, World& world, AssetHandler& assets) {
	start();

	ImGui_ImplWin32_GetDpiScaleForHwnd(window.window());

	ImGui::BeginMainMenuBar();
	ImGui::Button("hello!");
	ImGui::EndMainMenuBar();
	static bool renderer_open = false;
	if (ImGui::Begin("Renderer", &renderer_open)) {
		if (ImGui::BeginListBox("Render Mode")) {

			for (usize i = 0; i < 7; ++i) {
				if (ImGui::Selectable(MODES[i], (usize)renderer.second_pass.mode == i)) {
					renderer.second_pass.mode = (RenderMode)i;
				}
			}

			ImGui::EndListBox();
		}
	}
	ImGui::End();

	editor_ui(window, renderer, world, assets);

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

