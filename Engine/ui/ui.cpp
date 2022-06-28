#include"imgui/imgui.h"
#include"imgui/imgui_impl_win32.h"
#include"imgui/imgui_impl_dx11.h"
#include"ui.h"
#include<sstream>
#include<unordered_set>
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

void update_ui(const Window* window, World& world) {
	start();

	ImGui_ImplWin32_GetDpiScaleForHwnd(window->window());

	static bool active = true;
	ImGui::BeginMainMenuBar();
	ImGui::Button("hello!");
	ImGui::EndMainMenuBar();

	ImGui::Begin("Editor", &active);
	world.objects.iter([&](Id<Object> id, Object* obj) {
		std::string id_s = std::to_string(id.gen());
		id_s += ".";
		id_s += std::to_string(id.idx());
		std::string s("Object.");
		s += id_s;
		static std::unordered_set<Id<Object>> objects;
		bool open = objects.contains(id);
		ImGui::BeginGroup();
		if (ImGui::SmallButton(s.data())) {
			open = !open;
		}
		if (open) {
			objects.insert(id);
		}
		else {
			objects.erase(id);
		}
		if (open) {
			ImGui::BeginGroup();
			std::string label = "Translation";
			label += id_s;
			ImGui::DragFloat3(label.data(), obj->transform.translation.data());
			label = "Scale";
			label += id_s;
			ImGui::DragFloat3(label.data(), obj->transform.scale.data());
			label = "Color";
			label += id_s;
			ImGui::ColorEdit3(label.data(), obj->color.data());
			ImGui::EndGroup();
		}
		ImGui::EndGroup();
	});
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

