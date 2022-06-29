#include"imgui/imgui.h"
#include"imgui/imgui_impl_win32.h"
#include"imgui/imgui_impl_dx11.h"
#include"ui.h"
#include<sstream>
#include<unordered_set>
#include<math/consts.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void setup_ui(const Window* window, Renderer& renderer) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

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

void edit(const char* label, Vec2<f32>& v) {
	ImGui::DragFloat2(label, v.data());
}

void edit(const char* label, Vec3<f32>& v) {
	ImGui::DragFloat3(label, v.data());
}

void edit(const char* label, Vec4<f32>& v) {
	ImGui::DragFloat4(label, v.data());
}

void edit(const char* label, Mat3<f32>& v) {
	std::string s = label;
	s += " row 0";
	edit(s.data(), v[0]);
	s = label;
	s += " row 1";
	edit(s.data(), v[1]);
	s = label;
	s += " row 2";
	edit(s.data(), v[2]);
}

void edit(const char* label, Mat4<f32>& v) {
	if (ImGui::CollapsingHeader(label)) {
		ImGui::Indent();
		std::string s = label;
		s += " row 0";
		edit(s.data(), v[0]);
		s = label;
		s += " row 1";
		edit(s.data(), v[1]);
		s = label;
		s += " row 2";
		edit(s.data(), v[2]);
		s += " row 3";
		edit(s.data(), v[3]);
		ImGui::Unindent();
	}
}

void edit_vertices(Option<Mesh*> mesh, Mat4<f32> mat) {
	ImGui::Indent();
	mesh.then_do([&](Mesh* mesh) {
		for (usize i = 0; i < mesh->submeshes.len(); ++i) {
			ImGui::PushID(i);
			auto& vertices = mesh->submeshes[i].vertices;
			for (usize j = 0; j < vertices.len(); ++j) {
				ImGui::PushID(j);
				auto t = mat.transform_point(vertices[j].v);
				edit("vertex", t);
				ImGui::PopID();
			}
			ImGui::PopID();
		}
	});
	ImGui::Unindent();
}

void update_ui(const Window* window, World& world, AssetHandler& assets) {
	start();

	ImGui_ImplWin32_GetDpiScaleForHwnd(window->window());

	static bool active = true;
	ImGui::BeginMainMenuBar();
	ImGui::Button("hello!");
	ImGui::EndMainMenuBar();

	ImGui::Begin("Editor", &active);

	auto view_mat = world.camera.get_view();
	auto view_mat_inv = view_mat.invert();
	auto proj_mat = world.camera.get_proj(window->width(), window->height());
	auto proj_mat_inv = proj_mat.invert();

	if (ImGui::CollapsingHeader("Camera")) {
		ImGui::Indent();
		ImGui::DragFloat3("Camera Pos", world.camera.transform.translation.data());

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
			std::string id_s = std::to_string(id.gen());
			id_s += ".";
			id_s += std::to_string(id.idx());
			std::string s("Object.");
			s += id_s;
			if (ImGui::CollapsingHeader(s.data())) {
				ImGui::PushID(id.idx());
				ImGui::Indent();
				edit("Trans", obj->transform.translation);
				edit("Scale", obj->transform.scale);
				ImGui::ColorEdit3("Color", obj->color.data());

				if (ImGui::CollapsingHeader("Camera Relative")) {
					ImGui::Indent();

					auto view = (view_mat * obj->transform.translation.with_w(1.0)).xyz();
					edit("View", view);
					auto proj = (proj_mat * view.with_w(1.0)).xyz();
					edit("Proj", proj);

					ImGui::Unindent();
				}

				if (ImGui::CollapsingHeader("Vertices")) {
					ImGui::Indent();
					if (ImGui::CollapsingHeader("Local")) {
						edit_vertices(assets.get(obj->mesh), Mat4<f32>::identity());
					}
					if (ImGui::CollapsingHeader("Global")) {
						edit_vertices(assets.get(obj->mesh), obj->transform.get_mat());
					}
					if (ImGui::CollapsingHeader("View")) {
						edit_vertices(assets.get(obj->mesh), obj->transform.get_mat() * view_mat_inv);
					}
					if (ImGui::CollapsingHeader("Proj")) {
						edit_vertices(assets.get(obj->mesh), obj->transform.get_mat() * view_mat_inv * proj_mat_inv);
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

