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
	std::unique_ptr<ID3D11Device> device;
	std::unique_ptr<ID3D11DeviceContext> context;
	std::unique_ptr<IDXGISwapChain> swapChain;
	D3D11_VIEWPORT iewport;
	HWND window;

	std::unique_ptr<ID3D11RenderTargetView> rtv;
	std::unique_ptr<ID3D11Texture2D> dsTexture;
	std::unique_ptr<ID3D11DepthStencilView> dsView;
};

struct Renderer {
	RendererCtx ctx;
	ObjectRenderer object_renderer;
};

Result<Renderer, RenderCreateError> create_renderer(HINSTANCE instance, u32 width, u32 height, i32 nCmdShow);