#pragma once
#include<d3d11.h>
#include<memory>
#include<rstd/result.h> 
#include"world.h"

struct ObjectRenderer {
	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;
	ID3D11InputLayout* layout;
};

struct Globals {
	static Globals from_world(const World& world, u32 width, u32 height);

	Mat4<f32> view_matrix;
	Mat4<f32> proj_matrix;
};

template<typename T> requires (sizeof(T) % 4 == 0)
struct Uniform {
	ID3D11Buffer* buffer;

	void update(ID3D11DeviceContext* ctx, const T* data) {
		D3D11_MAPPED_SUBRESOURCE resource;

		if (FAILED(ctx->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource))) {
			PANIC("Failed to map uniform");
		}
		T* data_ptr = (T*)resource.pData;
		memcpy(data_ptr, data, sizeof(T));
		ctx->Unmap(buffer, 0);
	}
	void clean_up() {
		buffer->Release();
	}
};

struct FirstPass {
	ObjectRenderer object_renderer;
	Uniform<Globals> globals;
};

enum RenderCreateError {
	FailedWindowCreation,
	FailedDeviceCreation,
	FailedRTVCreation,
	FailedDepthStencilViewCreation,
	FailedTextureCreation,
	FailedBackBuffer,
	MissingShaderFile,
	FailedShaderCreation,
	FailedLayoutCreation,
	FailedBufferCreation,
};


struct RendererCtx {
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swap_chain;
	D3D11_VIEWPORT viewport;

	ID3D11RenderTargetView* rtv;
	ID3D11Texture2D* ds_texture;
	ID3D11DepthStencilView* ds_view;
};

struct Window;

struct Renderer {
	RendererCtx ctx;
	FirstPass first_pass;

	void clean_up();
	void begin_draw();
	void draw_first_pass(const Window* window, const World& world, const AssetHandler& assets);
	void resize(u32 width, u32 height);
	void present();
};

Result<Renderer, RenderCreateError> create_renderer(const Window* window);