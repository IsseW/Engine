#pragma once

#include<Windows.h>
#include<iostream>
#include<memory>
#include<renderer/window.h>
#include<dxgidebug.h>

struct DeviceCreationRes {
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swap_chain;
};

Result<DeviceCreationRes, RenderCreateError>
create_interfaces(const Window& window)
{
	DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};

	swap_chain_desc.BufferCount = 2;
	swap_chain_desc.BufferDesc.Width = 0;
	swap_chain_desc.BufferDesc.Height = 0;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	swap_chain_desc.OutputWindow = window.window();
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.Windowed = TRUE;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swap_chain;

	UINT creation_flags = 0;
#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
	const D3D_FEATURE_LEVEL feature_level_array[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
	
	if (FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creation_flags, feature_level_array, 2, D3D11_SDK_VERSION, &device, &feature_level, &context))) {
		return FailedDeviceCreation;
	}

#if defined(_DEBUG)
	// IDXGIDebug* debug = nullptr;
	// if (FAILED(DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&debug))) {
	// 	return FailedDeviceCreation;
	// }
	// debug->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_ALL);
#endif

	IDXGIDevice* idxgi_device = nullptr;
	if (FAILED(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&idxgi_device))) {
		return FailedSwapChainCreation;
	}
	
	IDXGIAdapter* adapter = nullptr;
	if (FAILED(idxgi_device->GetAdapter(&adapter))) {
		return FailedSwapChainCreation;
	}

	IDXGIFactory* factory = nullptr;
	if (FAILED(adapter->GetParent(__uuidof(IDXGIFactory), (void**)&factory))) {
		return FailedSwapChainCreation;
	}

	if (FAILED(factory->CreateSwapChain(device, &swap_chain_desc, &swap_chain))) {
		return FailedSwapChainCreation;
	}
	
	/*
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL, 
		D3D_DRIVER_TYPE_HARDWARE, 
		NULL, 
		creation_flags, 
		feature_level_array, 
		2, 
		D3D11_SDK_VERSION, 
		&swap_chain_desc,
		&swap_chain,
		&device,
		&feature_level,
		&context
	);
	if FAILED(hr) {
		return err<DeviceCreationRes, RenderCreateError>(FailedDeviceCreation);
	}
	*/

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
	screen_depth.resize(device, size);
	screen_gbuffer.resize(device, size);
}

Result<RendererCtx, RenderCreateError> RendererCtx::create(const Window& window) {
	
	DeviceCreationRes device_res;
	TRY(device_res, create_interfaces(window));
	auto device = device_res.device;
	auto context = device_res.context;
	auto swap_chain = device_res.swap_chain;
	RenderTarget screen;
	TRY(screen, RenderTarget::create(device, swap_chain));

	DepthTexture screen_depth;
	TRY(screen_depth, DepthTexture::create(device, window.size()));

	GBuffer screen_gbuffer;
	TRY(screen_gbuffer, GBuffer::create(device, window.size()));

	DepthTexture reflection_depth;
	TRY(reflection_depth, DepthTexture::create(device, REFLECTION_SIZE));

	GBuffer reflection_gbuffer;
	TRY(reflection_gbuffer, GBuffer::create(device, REFLECTION_SIZE));

	auto view_port = create_viewport(window.size());
	auto reflection_viewport = create_viewport(REFLECTION_SIZE);

	return ok<RendererCtx, RenderCreateError>(RendererCtx{
		device,
		context,
		swap_chain,
		view_port,
		screen,
		screen_depth,
		screen_gbuffer,
		reflection_viewport,
		reflection_depth,
		reflection_gbuffer,
	});
}

Result<Renderer, RenderCreateError> Renderer::create(const Window& window) {
	RendererCtx ctx;
	TRY(ctx, RendererCtx::create(window));

	ShadowPass shadow_pass;
	TRY(shadow_pass, ShadowPass::create(ctx.device, window.size()));

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

