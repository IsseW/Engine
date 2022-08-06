#include<renderer/window.h>
#include<ui/ui.h>


Window::Window(HWND window) : _window(window), _renderer{nullptr} {
	update_ps();
}

void Window::update_ps() {
	RECT rect;
	GetWindowRect(_window, &rect);
	_window_pos = Vec2<u16>(rect.left, rect.top);
	_window_size = Vec2<u16>(rect.right - rect.left, rect.bottom - rect.top);
	GetClientRect(_window, &rect);
	_client_pos = Vec2<u16>(rect.left, rect.top);
	_client_size = Vec2<u16>(rect.right - rect.left, rect.bottom - rect.top);
}

void Window::update(const MSG& msg) {
	_input.handle_window_message(msg, this);
}
void Window::new_frame() {
	_input.new_frame(this);
}

const Input& Window::input() const {
	return _input;
}

const Vec2<u16>& Window::pos() const {
	return _window_pos;
}
const Vec2<u16>& Window::size() const {
	return _client_size;
}
const Vec2<u16>& Window::window_size() const {
	return _window_size;
}
Vec2<u16> Window::center() const {
	return pos() + window_size() / 2;
}
f32 Window::ratio() const {
	return (f32)size().x / (f32)size().y;
}
const HWND& Window::window() const {
	return _window;
}

void Window::moved(u16 x, u16 y) {
	update_ps();
}

void Window::resized(u16 width, u16 height) {
	update_ps();
	if (_renderer) {
		_renderer->resize(*this, size());
	}
}

void Window::set_renderer(Renderer* renderer) {
	_renderer = renderer;
}

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_MOVE: {
		if (window) {
			window->moved(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	}
	case WM_SIZE: {
		Window* window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (window) {
			window->resized(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	if ((window == nullptr || !window->input().mouse_locked()) && ui_handle_input(hWnd, message, wParam, lParam)) {
		return true;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void clear_error() {
	SetLastError(0);
}

std::string last_error_msg()
{
	DWORD error_id = GetLastError();
	if (error_id == 0) {
		return std::string();
	}

	LPSTR msg_buffer = nullptr;

	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg_buffer, 0, NULL);

	std::string message(msg_buffer, size);

	LocalFree(msg_buffer);

	return message;
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

	Window* window = new Window(hw);

	clear_error();
	SetWindowLongPtr(hw, GWLP_USERDATA, (LONG_PTR)window);
	auto error = last_error_msg();

	if (error.size() > 0) {
		std::cerr << error << std::endl;
		PANIC("Failed to set window data.");
	}

	return some(std::move(window));
}