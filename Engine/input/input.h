#pragma once

#include<rstd/primitives.h>
#include<bitset>
#include<math/vec.h>

enum class Key {
	Mouse0, Mouse1, Mouse2, Mouse3, Mouse4,
	N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,
	D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
	A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,
	BackSpace,
	Tab,
	Return,
	Shift,
	Ctrl,
	Alt,
	CapsLock,
	NumLock,
	ScollLock,
	Esc,
	Space,
	PageUp,
	PageDown,
	End,
	Home,
	Left,
	Up,
	Right,
	Down,
	Insert,
	Delete,
	Multiply, 
	Add, 
	Separator, 
	Subtract, 
	Decimal, 
	Divide,

	Other,
	NKEYS,
};

struct tagMSG;
struct Window;

struct Input {
	
	void handle_window_message(const tagMSG& msg, const Window* window);
	void new_frame(const Window* window);

	bool just_pressed(Key key) const;
	bool pressed(Key key) const;
	bool just_released(Key key) const;
	Vec2<f32> mouse_pos() const;
	Vec2<f32> mouse_delta() const;
	f32 scroll_delta() const;
	bool mouse_locked() const;

private:
	f32 _scroll_delta;
	Vec2<f32> _mouse_pos_last;
	Vec2<f32> _mouse_pos;
	bool _mouse_locked = false;

	std::bitset<(usize)Key::NKEYS> _just_pressed;
	std::bitset<(usize)Key::NKEYS> _just_released;
	std::bitset<(usize)Key::NKEYS> _pressed;
	Key translate_vkey(u32 vkey);
	void handle_key(Key key, bool pressed);
};