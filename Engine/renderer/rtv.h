#pragma once
#include<rstd/result.h>
#include<d3d11.h>
#include<renderer/renderer.h>

inline Result<ID3D11RenderTargetView*, RenderCreateError> create_render_target_view(ID3D11Device* device, IDXGISwapChain* swap_chain)
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
