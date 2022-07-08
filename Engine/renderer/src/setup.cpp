#pragma once

#include<Windows.h>
#include<iostream>
#include<memory>
#include<renderer/window.h>

struct DeviceCreationRes {
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swap_chain;
};

Result<DeviceCreationRes, RenderCreateError>
create_interfaces(const Window& window)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	swapChainDesc.OutputWindow = window.window();
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

D3D11_VIEWPORT create_viewport(Vec2<u16> size)
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (f32)size.x;
	viewport.Height = (f32)size.y;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	return viewport;
}

void RendererCtx::resize(Vec2<u16> size) {
	viewport = create_viewport(size);
	screen.clean_up();
	auto res = swap_chain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(res)) {
		PANIC("Failed to resize swap chain");
	}
	screen = RenderTarget::create(device, swap_chain).unwrap();
}

Result<RendererCtx, RenderCreateError> RendererCtx::create(const Window& window) {
	
	DeviceCreationRes device_res;
	TRY(device_res, create_interfaces(window));
	auto device = device_res.device;
	auto context = device_res.context;
	auto swap_chain = device_res.swap_chain;
	RenderTarget screen;
	TRY(screen, RenderTarget::create(device, swap_chain));

	auto view_port = create_viewport(window.size());

	return ok<RendererCtx, RenderCreateError>(RendererCtx{
		device,
		context,
		swap_chain,
		view_port,
		screen,
	});
}

Result<Renderer, RenderCreateError> Renderer::create(const Window& window) {
	RendererCtx ctx;
	TRY(ctx, RendererCtx::create(window));

	ShadowPass shadow_pass {};
	// TRY(shadow_pass, ShadowPass::create(ctx.device, window.size()));

	FirstPass first_pass;
	TRY(first_pass, FirstPass::create(ctx.device, window.size()));

	SecondPass second_pass;
	TRY(second_pass, SecondPass::create(ctx.device));

	return ok<Renderer, RenderCreateError>(Renderer {
		ctx,
		shadow_pass,
		first_pass,
		second_pass,
	});
}

