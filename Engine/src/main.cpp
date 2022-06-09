#include<math/mat.h>
#include<renderer/renderer.h>
#include<chrono>
#include<Windows.h>
#include<ui/ui.h>


struct Console {
	Console() {
		AllocConsole();
		freopen_s(&console_err, "CONOUT$", "w", stderr);
		freopen_s(&console_out, "CONOUT$", "w", stdout);
		freopen_s(&console_in, "CONIN$", "r", stdin);
	}
	~Console() {
		if (console_err) {
			fclose(console_err);
		}
		if (console_out) {
			fclose(console_out);
		}
		if (console_in) {
			fclose(console_in);
		}
	}
private:
	FILE* console_err;
	FILE* console_out;
	FILE* console_in;
};
// TODO: Add a TEST define?
#ifdef _DEBUG
void run_tests();
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	Console __console{};

#ifdef _DEBUG
	run_tests();
#endif

	Renderer renderer = create_renderer(hInstance, 600, 600, nCmdShow).unwrap();

	setup_ui(renderer.ctx);

	MSG msg = { };
	auto last_frame = std::chrono::high_resolution_clock::now();
	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT) {
		auto now = std::chrono::high_resolution_clock::now();
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (!handle_input(msg.hwnd, msg.message, msg.wParam, msg.lParam)) {
				switch (msg.message) {
				case WM_SIZE:
					PANIC("hello");
					if (renderer.ctx.device != NULL && msg.wParam != SIZE_MINIMIZED) {
						renderer.resize((UINT)LOWORD(msg.lParam), (UINT)HIWORD(msg.lParam));
					}
					break;
				case WM_DESTROY:
					PostQuitMessage(0);
					return 0;
				default:
					break;
				}
			} 
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		float dt = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(now - last_frame).count();
		last_frame = now;
		renderer.begin_draw();
		update_ui(renderer);
		renderer.present();

	}
	clean_up_ui();
	renderer.clean_up();

	return 0;
}