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

std::string GetLastErrorAsString()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string("No Error"); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	Console __console{};

#ifdef _DEBUG
	run_tests();
#endif

	Renderer* renderer = create_renderer(hInstance, 600, 600, nCmdShow).unwrap();
	SetWindowLongPtr(renderer->ctx.window, GWLP_USERDATA, (LONG_PTR)renderer);

	setup_ui(renderer->ctx);

	MSG msg = { };
	auto last_frame = std::chrono::high_resolution_clock::now();
	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT) {
		auto now = std::chrono::high_resolution_clock::now();
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (!handle_input(msg.hwnd, msg.message, msg.wParam, msg.lParam)) {
				switch (msg.message) {
				case WM_SIZING:
					PANIC("hello");
					if (renderer->ctx.device != NULL && msg.wParam != SIZE_MINIMIZED) {
						renderer->resize((UINT)LOWORD(msg.lParam), (UINT)HIWORD(msg.lParam));
					}
					break;
				case WM_DESTROY:
					PostQuitMessage(0);
					return 0;
				default:
					break;
				}
			} 
		}
		float dt = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(now - last_frame).count();
		last_frame = now;
		renderer->begin_draw();
		update_ui(renderer);
		renderer->present();

	}
	clean_up_ui();
	renderer->clean_up();
	delete renderer;

	return 0;
}