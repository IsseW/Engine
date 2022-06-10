#include"imgui/imgui.h"
#include"imgui/imgui_impl_win32.h"
#include"imgui/imgui_impl_dx11.h"
#include"ui.h"

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

void update_ui(const Window* window) {
	start();

	ImGui_ImplWin32_GetDpiScaleForHwnd(window->window());

	static bool active = true;
	ImGui::BeginMainMenuBar();
	ImGui::Button("hello!");
	ImGui::EndMainMenuBar();
	ImGui::Begin("lmao", &active);
	if (ImGui::Button("Test")) {
		std::cout << "lol" << std::endl;
	}
	ImGui::End();

	end();
}

bool handle_input(HWND window, UINT msg, WPARAM wp, LPARAM lp) {
	return ImGui_ImplWin32_WndProcHandler(window, msg, wp, lp);
}

void clean_up_ui() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

