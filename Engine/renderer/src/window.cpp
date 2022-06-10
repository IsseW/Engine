#include<renderer/window.h>
#include<ui/ui.h>


Window::Window(HWND window, u32 width, u32 height) : _window(window), _width(width), _height(height) { }
u32 Window::width() const {
	return _width;
}
u32 Window::height() const {
	return _height;
}
const HWND& Window::window() const {
	return _window;
}

void Window::resized() {
	RECT rect;
	GetClientRect(_window, &rect);
	_width = rect.right - rect.left;
	_height = rect.bottom - rect.top;
	if (_renderer) {
		_renderer->resize(_width, _height);
	}
}

void Window::set_renderer(Renderer* renderer) {
	_renderer = renderer;
}

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (handle_input(hWnd, message, wParam, lParam)) {
		return true;
	}
	switch (message)
	{
	case WM_SIZE: {
		Window* window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (window) {
			window->resized();
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void clear_error() {
	SetLastError(0);
}

Option<std::string> last_error_msg()
{
	DWORD error_id = GetLastError();
	if (error_id == 0) {
		return none<std::string>();
	}

	LPSTR msg_buffer = nullptr;

	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg_buffer, 0, NULL);

	std::string message(msg_buffer, size);

	LocalFree(msg_buffer);

	return some(std::move(message));
}

Option<Window*> create_window(HINSTANCE instance, u32 width, u32 height, i32 n_cmd_show)
{
	const wchar_t CLASS_NAME[] = L"Cool Engine";

	WNDCLASS wc = {};
	wc.lpfnWndProc = window_proc;
	wc.hInstance = instance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	RECT rt = { 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hw = CreateWindowEx(0, CLASS_NAME, L"Very cool", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, rt.right - rt.left, rt.bottom - rt.top, nullptr, nullptr, instance, nullptr);
	
	if (hw == nullptr) {
		return none<Window*>();
	}
	ShowWindow(hw, n_cmd_show);

	Window* window = new Window(hw, width, height);

	clear_error();
	SetWindowLongPtr(hw, GWLP_USERDATA, (LONG_PTR)window);
	auto error = last_error_msg();

	if (error.is_some()) {
		std::cerr << error.unwrap_unchecked() << std::endl;
		PANIC("Failed to set window data.");
	}

	return some(std::move(window));
}