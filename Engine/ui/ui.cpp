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

bool edit_vertices(Option<Mesh*> mesh, Mat4<f32> mat) {
	bool edited = false;
	ImGui::Indent();
	mesh.then_do([&](Mesh* mesh) {
		for (usize i = 0; i < mesh->submeshes.len(); ++i) {
			auto& vertices = mesh->submeshes[i].vertices;
			for (usize j = 0; j < vertices.len(); ++j) {
				ImGui::PushID(i + j * mesh->submeshes.len());
				auto t = mat.transform_point(vertices[j].v);
				edited |= edit("vertex", t);
				ImGui::PopID();
			}
		}
	});
	ImGui::Unindent();
	return edited;
}

void editor_ui(const Window& window, World& world, AssetHandler& assets) {
	static bool active = true;
	ImGui::Begin("Editor", &active);

	auto view_mat = world.camera.get_view();
	auto view_mat_inv = view_mat.invert();
	auto proj_mat = world.camera.get_proj(window.ratio());
	auto proj_mat_inv = proj_mat.invert();

	if (ImGui::CollapsingHeader("Camera")) {
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
	}

	if (ImGui::CollapsingHeader("Objects")) {
		ImGui::Indent();
		world.objects.iter([&](Id<Object> id, Object* obj) {
			std::string s("Object.");
			s += id.to_string();
			if (ImGui::CollapsingHeader(s.data())) {
				ImGui::PushID(id.idx());
				ImGui::Indent();
				edit("Trans", obj->transform.translation);
				edit("Scale", obj->transform.scale);
				edit("Rotation", obj->transform.rotation);


				ImGui::ColorEdit3("Color", obj->color.data());

				if (ImGui::BeginListBox("Mesh")) {
					assets.assets<Mesh>().iter([&](AId<Mesh> id, const Asset<Mesh>* asset) {
						std::string s = id.to_string();
						s += " ";
						s += asset->path.as_ptr().map<const char*>([](const std::string* s) { return s->data(); }).unwrap_or("unnamed");

						if (ImGui::Selectable(s.data(), obj->mesh == id)) {
							obj->mesh = id;
						}

						});
					ImGui::EndListBox();
				}


				if (ImGui::CollapsingHeader("Camera Relative")) {
					ImGui::Indent();

					auto view = view_mat_inv.transform_point(obj->transform.translation);
					edit("View", view);
					auto proj = proj_mat_inv.transform_point(view);
					edit("Proj", proj);

					ImGui::Unindent();
				}

				auto mat = obj->transform.get_mat();

				if (ImGui::CollapsingHeader("Vertices")) {
					ImGui::Indent();
					if (ImGui::CollapsingHeader("Local")) {
						edit_vertices(assets.get(obj->mesh), Mat4<f32>::identity());
					}
					if (ImGui::CollapsingHeader("Global")) {
						edit_vertices(assets.get(obj->mesh), mat);
					}
					if (ImGui::CollapsingHeader("View")) {
						edit_vertices(assets.get(obj->mesh), mat * view_mat);
					}
					if (ImGui::CollapsingHeader("Proj")) {
						edit_vertices(assets.get(obj->mesh), mat * view_mat * proj_mat);
					}
					ImGui::Unindent();
				}

				ImGui::Unindent();
				ImGui::PopID();
			}
			});
		ImGui::Unindent();
	}

	ImGui::End();
}

void update_ui(const Window& window, World& world, AssetHandler& assets) {
	start();

	ImGui_ImplWin32_GetDpiScaleForHwnd(window.window());

	ImGui::BeginMainMenuBar();
	ImGui::Button("hello!");
	ImGui::EndMainMenuBar();

	editor_ui(window, world, assets);

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

