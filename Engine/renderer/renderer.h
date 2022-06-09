#pragma once
#include<d3d11.h>
#include<memory>
#include<rstd/result.h> 

struct ObjectRenderer {

};

enum RenderCreateError {
	FailedWindowCreation,
	FailedDeviceCreation,
	FailedRTVCreation,
	FailedDepthStencilViewCreation,
	FailedTextureCreation,
	FailedBackBuffer,
};

struct RendererCtx {
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swap_chain;
	D3D11_VIEWPORT viewport;
	HWND window;

	ID3D11RenderTargetView* rtv;
	ID3D11Texture2D* ds_texture;
	ID3D11DepthStencilView* ds_view;
};

struct Renderer {
	RendererCtx ctx;
	ObjectRenderer object_renderer;

	void clean_up();
	void present();
};

Result<Renderer, RenderCreateError>&& create_renderer(HINSTANCE instance, u32 width, u32 height, i32 nCmdShow);