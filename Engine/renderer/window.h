#pragma once
#include<d3d11.h>
#include<rstd/depot.h>
#include"renderer.h"

struct Window {
	Window(HWND window, u32 width, u32 height);
	u32 width() const;
	u32 height() const;
	const HWND& window() const;
	void resized();
	void set_renderer(Renderer* renderer);
private:
	HWND _window;
	u32 _width;
	u32 _height;
	Renderer* _renderer;
};

Option<Window*> create_window(HINSTANCE instance, u32 width, u32 height, i32 n_command_show);