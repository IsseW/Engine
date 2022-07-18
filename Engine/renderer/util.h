#pragma once

#include<d3d11.h>
#include<rstd/result.h>
#include<math/vec.h>
#include<vector>

enum RenderCreateError {
	FailedWindowCreation,
	FailedDeviceCreation,
	FailedRTVCreation,
	FailedUAVCreation,
	FailedDSVCreation,
	FailedSRVCreation,
	FailedTextureCreation,
	FailedSamplerStateCreation,
	FailedBackBuffer,
	MissingShaderFile,
	FailedShaderCreation,
	FailedLayoutCreation,
	FailedBufferCreation,
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
		if (buffer) buffer->Release();
	}

	static Result<Uniform, RenderCreateError> create(ID3D11Device* device) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(T);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		ID3D11Buffer* buffer;
		if (FAILED(device->CreateBuffer(&desc, nullptr, &buffer))) {
			return FailedBufferCreation;
		}

		return ok<Uniform<T>, RenderCreateError>(Uniform<T> { buffer });
	}
};

template<typename T> requires (sizeof(T) % 4 == 0)
struct SBuffer {
	ID3D11Buffer* buffer;

	void update(ID3D11DeviceContext* ctx, const T* data, usize count) {
		D3D11_MAPPED_SUBRESOURCE resource;

		if (FAILED(ctx->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource))) {
			PANIC("Failed to map uniform");
		}
		T* data_ptr = (T*)resource.pData;
		memcpy(data_ptr, data, sizeof(T) * count);
		ctx->Unmap(buffer, 0);
	}
	void clean_up() {
		if (buffer) buffer->Release();
	}

	static Result<SBuffer, RenderCreateError> create(ID3D11Device* device, usize count) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(T) * count;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = sizeof(T);

		ID3D11Buffer* buffer;
		if (FAILED(device->CreateBuffer(&desc, nullptr, &buffer))) {
			return FailedBufferCreation;
		}

		return ok<SBuffer, RenderCreateError>(SBuffer{ buffer });
	}
};


struct DepthTexture {
	ID3D11DepthStencilView* view;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;

	static Result<DepthTexture, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void clean_up();

	void clear(ID3D11DeviceContext* ctx);
};

struct DepthTextures {
	ID3D11DepthStencilView* view;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;

	Vec3<u16> size;

	static Result<DepthTextures, RenderCreateError> create(ID3D11Device* device, Vec3<u16> size);

	void resize(ID3D11Device* device, Vec3<u16> size);
	void clean_up();

	void clear(ID3D11DeviceContext* ctx);
};

struct RenderTexture {
	ID3D11RenderTargetView* rtv;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;

	DXGI_FORMAT format;

	static Result<RenderTexture, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size, DXGI_FORMAT format);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void clean_up();
};

struct RenderTarget {
	ID3D11Texture2D* texture;
	ID3D11UnorderedAccessView* uav;
	ID3D11RenderTargetView* rtv;

	static Result<RenderTarget, RenderCreateError> create(ID3D11Device* device, IDXGISwapChain* swap_chain);

	void resize(ID3D11Device* device, IDXGISwapChain* swap_chain);

	void clean_up();
};

Result<ID3D11PixelShader*, RenderCreateError> load_pixel(ID3D11Device* device, const char* file);

struct VSIL {
	ID3D11VertexShader* vs;
	ID3D11InputLayout* il;
};
Result<VSIL, RenderCreateError> load_vertex(ID3D11Device* device, const char* file, const std::vector<D3D11_INPUT_ELEMENT_DESC>& input);

Result<ID3D11ComputeShader*, RenderCreateError> load_compute(ID3D11Device* device, const char* file);

struct Renderer;
struct RendererCtx;