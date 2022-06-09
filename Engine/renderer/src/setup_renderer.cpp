#pragma once

#include<Windows.h>
#include<iostream>
#include<ui/ui.h>
#include<memory>
#include<renderer/renderer.h>
#include<renderer/rtv.h>

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (handle_input(hWnd, message, wParam, lParam)) {
		return true;
	}
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND setup_window(HINSTANCE instance, u32 width, u32 height)
{
	const wchar_t CLASS_NAME[] = L"Cool Engine";

	WNDCLASS wc = {};
	wc.lpfnWndProc = window_proc;
	wc.hInstance = instance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	RECT rt = { 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, FALSE);

	return CreateWindowEx(0, CLASS_NAME, L"Very cool", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, rt.right - rt.left, rt.bottom - rt.top, nullptr, nullptr, instance, nullptr);
}

struct DeviceCreationRes {
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swap_chain;
};

Result<DeviceCreationRes, RenderCreateError>
create_interfaces(u32 width, u32 height, HWND window)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swap_chain;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL feature_level;
	const D3D_FEATURE_LEVEL feature_level_array[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL, 
		D3D_DRIVER_TYPE_HARDWARE, 
		NULL, 
		createDeviceFlags, 
		feature_level_array, 
		2, 
		D3D11_SDK_VERSION, 
		&swapChainDesc,
		&swap_chain,
		&device,
		&feature_level,
		&context
	);

	if FAILED(hr) {
		return err<DeviceCreationRes, RenderCreateError>(FailedDeviceCreation);
	}
	else {
		return ok<DeviceCreationRes, RenderCreateError>(DeviceCreationRes { device, context, swap_chain });
	}
}

struct DepthStencilRes {
	ID3D11Texture2D* ds_texture;
	ID3D11DepthStencilView* ds_view;
};

Result<DepthStencilRes, RenderCreateError>  create_depth_stencil(ID3D11Device* device, u32 width, u32 height)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* ds_texture;
	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &ds_texture))) {
		return err<DepthStencilRes, RenderCreateError>(FailedTextureCreation);
	}
	ID3D11DepthStencilView* ds_view;
	HRESULT hr = device->CreateDepthStencilView(ds_texture, 0, &ds_view);

	if (FAILED(hr)) {
		return err<DepthStencilRes, RenderCreateError>(FailedDepthStencilViewCreation);
	}
	else {
		return ok<DepthStencilRes, RenderCreateError>(DepthStencilRes{ ds_texture, ds_view });
	}
}

D3D11_VIEWPORT create_viewport(uint32_t width, uint32_t height)
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	return viewport;
}

Result<RendererCtx, RenderCreateError> create_renderer_ctx(HINSTANCE instance, u32 width, u32 height, i32 n_cmd_show) {
	HWND window = setup_window(instance, width, height);
	if (window == nullptr) {
		return err<RendererCtx, RenderCreateError>(FailedWindowCreation);
	}
	ShowWindow(window, n_cmd_show);
	
	DeviceCreationRes device_res;
	TRY(device_res, create_interfaces(width, height, window));
	auto device = device_res.device;
	auto context = device_res.context;
	auto swap_chain = device_res.swap_chain;
	ID3D11RenderTargetView* rtv;
	TRY(rtv, create_render_target_view(device, swap_chain));

	DepthStencilRes depth_res;
	TRY(depth_res, create_depth_stencil(device, width, height));
	auto ds_texture = depth_res.ds_texture;
	auto ds_view = depth_res.ds_view;

	auto view_port = create_viewport(width, height);

	return ok<RendererCtx, RenderCreateError>(RendererCtx{
		device,
		context,
		swap_chain,
		view_port,
		window,
		rtv,
		ds_texture,
		ds_view,
	});
}

Result<Renderer, RenderCreateError> create_renderer(HINSTANCE instance, u32 width, u32 height, i32 n_cmd_show) {
	RendererCtx ctx;
	TRY(ctx, create_renderer_ctx(instance, width, height, n_cmd_show));

	return ok<Renderer, RenderCreateError>(Renderer {
		ctx
	});
}