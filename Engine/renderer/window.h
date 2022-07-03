#pragma once
#include<d3d11.h>
#include<rstd/depot.h>
#include"renderer.h"
#include<input/input.h>

struct Window {
	Window(HWND window);
	const Vec2<u16>& pos() const;
	const Vec2<u16>& size() const;
	const Vec2<u16>& window_size() const;
	Vec2<u16> center() const;
	f32 ratio() const;
	const HWND& window() const;
	void moved(u16, u16);
	void resized(u16, u16);
	void set_renderer(Renderer* renderer);
	void update(const MSG& msg);
	void new_frame();
	const Input& input() const;
private:

	void update_ps();

	HWND _window;
	Vec2<u16> _window_pos;
	Vec2<u16> _window_size;
	Vec2<u16> _client_pos;
	Vec2<u16> _client_size;
	Renderer* _renderer;
	Input _input;
};

Option<Window*> create_window(HINSTANCE instance, u32 width, u32 height, i32 n_command_show);