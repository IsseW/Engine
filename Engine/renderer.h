#pragma once
#include "util.h"
#include "standard.h"

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
	DXPointer<ID3D11Device> device;
	DXPointer<ID3D11DeviceContext> context;
	DXPointer<IDXGISwapChain> swapChain;
	D3D11_VIEWPORT viewport;
	HWND window;

	DXPointer<ID3D11RenderTargetView> rtv;
	DXPointer<ID3D11Texture2D> dsTexture;
	DXPointer<ID3D11DepthStencilView> dsView;
};

struct Renderer {
	RendererCtx ctx;
	ObjectRenderer object_renderer;
};
Res<Renderer, RenderCreateError> create_renderer(HINSTANCE instance, uint32_t width, uint32_t height, int nCmdShow);