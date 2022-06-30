#pragma once

#include<Windows.h>
#include<iostream>
#include<memory>
#include<renderer/window.h>
#include<fstream>

struct DeviceCreationRes {
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swap_chain;
};

Result<DeviceCreationRes, RenderCreateError>
create_interfaces(const Window* window)
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
	swapChainDesc.OutputWindow = window->window();
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

Result<RendererCtx, RenderCreateError> create_renderer_ctx(const Window* window) {
	
	DeviceCreationRes device_res;
	TRY(device_res, create_interfaces(window));
	auto device = device_res.device;
	auto context = device_res.context;
	auto swap_chain = device_res.swap_chain;
	ID3D11RenderTargetView* rtv;
	TRY(rtv, create_render_target_view(device, swap_chain));

	DepthStencilRes depth_res;
	TRY(depth_res, create_depth_stencil(device, window->width(), window->height()));
	auto ds_texture = depth_res.ds_texture;
	auto ds_view = depth_res.ds_view;

	auto view_port = create_viewport(window->width(), window->height());

	return ok<RendererCtx, RenderCreateError>(RendererCtx{
		device,
		context,
		swap_chain,
		view_port,
		rtv,
		ds_texture,
		ds_view,
	});
}

template<typename T>
Result<Uniform<T>, RenderCreateError> create_uniform(RendererCtx& ctx) requires (sizeof(T) % 4 == 0) {
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(T);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	ID3D11Buffer* buffer;
	if (FAILED(ctx.device->CreateBuffer(&desc, nullptr, &buffer))) {
		return FailedBufferCreation;
	}

	return ok<Uniform<T>, RenderCreateError>(Uniform<T> { buffer });
}

Result<ObjectRenderer, RenderCreateError> create_object_renderer(RendererCtx& ctx)
{
	std::string vs_data;
	std::ifstream reader;
	reader.open("VertexShader.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		return MissingShaderFile;
	}

	reader.seekg(0, std::ios::end);
	vs_data.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	vs_data.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());
	ID3D11VertexShader* vs;
	if (FAILED(ctx.device->CreateVertexShader(vs_data.c_str(), vs_data.length(), nullptr, &vs)))
	{
		return FailedShaderCreation;
	}
	std::string ps_data;
	reader.close();
	reader.open("PixelShader.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		return MissingShaderFile;
	}

	reader.seekg(0, std::ios::end);
	ps_data.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	ps_data.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());

	ID3D11PixelShader* ps;
	if (FAILED(ctx.device->CreatePixelShader(ps_data.c_str(), ps_data.length(), nullptr, &ps)))
	{
		return FailedShaderCreation;
	}

	D3D11_INPUT_ELEMENT_DESC input_desc[3] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	ID3D11InputLayout* layout;

	if (FAILED(ctx.device->CreateInputLayout(input_desc, 3, vs_data.c_str(), vs_data.length(), &layout))) {
		return FailedLayoutCreation;
	}

	Uniform<ObjectRenderer::Locals> locals;
	TRY(locals, create_uniform<ObjectRenderer::Locals>(ctx));

	return ok<ObjectRenderer, RenderCreateError>(ObjectRenderer {
			vs,
			ps,
			layout,
			locals
		});
}

Result<FirstPass, RenderCreateError> create_first_pass(RendererCtx& ctx) {
	ObjectRenderer object_renderer;
	TRY(object_renderer, create_object_renderer(ctx));

	Uniform<Globals> globals;
	TRY(globals, create_uniform<Globals>(ctx));
	// Bind globals
	//ctx.context->VSSetConstantBuffers(0, 1, &globals.buffer);
	//ctx.context->PSSetConstantBuffers(0, 1, &globals.buffer);
	
	return ok<FirstPass, RenderCreateError>(FirstPass{
			object_renderer,
			globals
		});
}

Result<Renderer, RenderCreateError> create_renderer(const Window* window) {
	RendererCtx ctx;
	TRY(ctx, create_renderer_ctx(window));

	FirstPass first_pass;
	TRY(first_pass, create_first_pass(ctx));

	return ok<Renderer, RenderCreateError>(Renderer {
		ctx,
		first_pass,
	});
}

Result<ID3D11RenderTargetView*, RenderCreateError> create_render_target_view(ID3D11Device* device, IDXGISwapChain* swap_chain)
{
	// get the address of the back buffer
	ID3D11Texture2D* backBuffer = nullptr;
	if (FAILED(swap_chain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
	{
		return err<ID3D11RenderTargetView*, RenderCreateError>(FailedBackBuffer);
	}
	ID3D11RenderTargetView* rtv;
	// use the back buffer address to create the render target
	// null as description to base it on the backbuffers values
	HRESULT hr = device->CreateRenderTargetView(backBuffer, NULL, &rtv);
	backBuffer->Release();
	if (FAILED(hr)) {
		return err<ID3D11RenderTargetView*, RenderCreateError>(FailedRTVCreation);
	}
	else {
		return ok<ID3D11RenderTargetView*, RenderCreateError>(rtv);
	}
}
