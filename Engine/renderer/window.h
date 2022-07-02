#pragma once
#include<d3d11.h>
#include<rstd/depot.h>
#include"renderer.h"
#include<input/input.h>

struct Window {
	Window(HWND window);
	Vec2<u32> pos() const;
	Vec2<u32> size() const;
	Vec2<u32> center() const;
	f32 ratio() const;
	const HWND& window() const;
	void moved(u16, u16);
	void resized();
	void set_renderer(Renderer* renderer);
	void update(const MSG& msg);
	void new_frame();
	const Input& input() const;
private:
	HWND _window;
	u32 _x;
	u32 _y;
	u32 _width;
	u32 _height;
	Renderer* _renderer;
	Input _input;
};

Option<Window*> create_window(HINSTANCE instance, u32 width, u32 height, i32 n_command_show);