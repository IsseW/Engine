#include<renderer/window.h>
#include<input/input.h>
#include<ui/ui.h>

void Input::new_frame(const Window* window) {

	
	if (_mouse_locked) {
		if (just_pressed(Key::Esc)) {
			_mouse_locked = false;
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
	}
	else {
		if (just_pressed(Key::Esc)) {
			_mouse_locked = true;
			SetCursor(NULL);
		}
	}

	if (mouse_locked()) {
		SetCursorPos(window->center().x, window->center().y);
	}

	_just_pressed.reset();
	_just_released.reset();
	_mouse_pos_last = _mouse_pos;
}

bool Input::just_pressed(Key key) const {
	return _just_pressed.test((usize)key);
}
bool Input::pressed(Key key) const {
	return _pressed.test((usize)key);
}
bool Input::just_released(Key key) const {
	return _just_released.test((usize)key);
}
Vec2<f32> Input::mouse_pos() const {
	return _mouse_pos;
}
Vec2<f32> Input::mouse_delta() const {
	return _mouse_pos - _mouse_pos_last;
}
f32 Input::scroll_delta() const {
	return _scroll_delta;
}
bool Input::mouse_locked() const {
	return _mouse_locked;
}

void Input::handle_key(Key key, bool down) {
	_just_pressed.set((usize)key, down);
	_pressed.set((usize)key, down);
	_just_released.set((usize)key, !down);
}

void Input::handle_window_message(const MSG& msg, const Window* window) {
	switch (msg.message) {
	case WM_MOUSEMOVE: {
		if (mouse_locked()) {
			i16 x = LOWORD(msg.lParam);
			i16 y = HIWORD(msg.lParam);

			_mouse_pos = Vec2<f32>(x, window->size().y - y);
		}
		break;
	}
	case WM_MOUSEWHEEL: {
		if (!capturing_mouse()) {
			i16 delta = HIWORD(msg.wParam);
			_scroll_delta += (f32)delta / (f32)WHEEL_DELTA;
		}
		break;
	}
	case WM_LBUTTONDOWN:
		if (!capturing_mouse()) handle_key(Key::Mouse0, true);
		break;
	case WM_LBUTTONUP:
		if (!capturing_mouse()) handle_key(Key::Mouse0, false);
		break;
	case WM_RBUTTONDOWN:
		if (!capturing_mouse()) handle_key(Key::Mouse1, true);
		break;
	case WM_RBUTTONUP:
		if (!capturing_mouse()) handle_key(Key::Mouse1, false);
		break;
	case WM_MBUTTONDOWN:
		if (!capturing_mouse()) handle_key(Key::Mouse2, true);
		break;
	case WM_MBUTTONUP:
		if (!capturing_mouse()) handle_key(Key::Mouse2, false);
		break;
	case WM_XBUTTONDOWN: {
		if (!capturing_mouse()) {
			switch (HIWORD(msg.wParam)) {
			case XBUTTON1:
				handle_key(Key::Mouse3, true);
				break;
			case XBUTTON2:
				handle_key(Key::Mouse4, true);
				break;
			}
		}
		break;
	}
	case WM_XBUTTONUP: {
		if (!capturing_mouse()) {
			switch (HIWORD(msg.wParam)) {
			case XBUTTON1:
				handle_key(Key::Mouse3, false);
				break;
			case XBUTTON2:
				handle_key(Key::Mouse4, false);
				break;
			}
		}
		break;

	case WM_KEYDOWN:
		if (!capturing_keyboard()) handle_key(translate_vkey((u32)msg.wParam), true);
		break;
	case WM_KEYUP:
		if (!capturing_keyboard()) handle_key(translate_vkey((u32)msg.wParam), false);
		break;

	// default: std::cout << msg.message << std::endl; break;
	}

	}
}

Key Input::translate_vkey(u32 vkey) {
	if (vkey >= 0x30 && vkey < 0x39) {
		return (Key)((u32)Key::N0 + vkey - 0x30);
	}
	if (vkey >= 0x60 && vkey < 0x69) {
		return (Key)((u32)Key::D0 + vkey - 0x60);
	}
	if (vkey >= 0x70 && vkey < 0x87) {
		return (Key)((u32)Key::F1 + vkey - 0x70);
	}
	if (vkey >= 0x41 && vkey < 0x5A) {
		return (Key)((u32)Key::A + vkey - 0x41);
	}
	switch (vkey) {
	case VK_BACK: return Key::BackSpace;
	case VK_TAB: return Key::Tab;
	case VK_RETURN: return Key::Return;
	case VK_SHIFT: return Key::Shift;
	case VK_CONTROL: return Key::Ctrl;
	case VK_MENU: return Key::Alt;
	case VK_CAPITAL: return Key::CapsLock;
	case VK_NUMLOCK: return Key::NumLock;
	case VK_SCROLL: return Key::ScollLock;
	case VK_ESCAPE: return Key::Esc;
	case VK_SPACE: return Key::Space;
	case VK_PRIOR: return Key::PageUp;
	case VK_NEXT: return Key::PageDown;
	case VK_END: return Key::End;
	case VK_HOME: return Key::Home;
	case VK_LEFT: return Key::Left;
	case VK_UP: return Key::Up;
	case VK_RIGHT: return Key::Right;
	case VK_DOWN: return Key::Down;
	case VK_INSERT: return Key::Insert;
	case VK_DELETE: return Key::Delete;
	case VK_MULTIPLY: return Key::Multiply;
	case VK_ADD: return Key::Add;
	case VK_SEPARATOR: return Key::Separator;
	case VK_SUBTRACT: return Key::Subtract;
	case VK_DECIMAL: return Key::Decimal;
	case VK_DIVIDE: return Key::Divide;
	}
	return Key::Other;
}