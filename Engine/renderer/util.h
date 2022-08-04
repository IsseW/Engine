#pragma once

#include<d3d11.h>
#include<rstd/result.h>
#include<math/vec.h>
#include<rstd/vector.h>
#include<vector>

enum RenderCreateError {
	FailedWindowCreation,
	FailedDeviceCreation,
	FailedSwapChainCreation,
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
	FailedRSCreation,
};

template<typename T> requires (sizeof(T) % 16 == 0)
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

/// <summary>
/// Will always contain float4 for now.
/// </summary>
struct Buffer {
	ID3D11Buffer* buffer;
	ID3D11UnorderedAccessView* uav;
	usize len;

	static Result<Buffer, RenderCreateError> create(ID3D11Device* device, const Vec4<f32>* data, usize len);
	void clean_up();
};

template<typename T>
struct SBuffer {
	ID3D11Buffer* buffer;
	ID3D11ShaderResourceView* srv;
	usize capacity;

	void update(ID3D11DeviceContext* ctx, const T* data, usize count) {
		if (count >= capacity) {
			clean_up();
			ID3D11Device* device;
			ctx->GetDevice(&device);
			*this = create(device, capacity * 2).unwrap();
		}
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
		if (srv) srv->Release();
	}

	static Result<SBuffer, RenderCreateError> create(ID3D11Device* device, usize capacity) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(T) * capacity;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(T);

		ID3D11Buffer* buffer;
		if (FAILED(device->CreateBuffer(&desc, nullptr, &buffer))) {
			return FailedBufferCreation;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC resource_desc;
		resource_desc.Format = DXGI_FORMAT_UNKNOWN;
		resource_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		resource_desc.Buffer.FirstElement = 0;
		resource_desc.Buffer.NumElements = capacity;

		ID3D11ShaderResourceView* srv;
		if (FAILED(device->CreateShaderResourceView(buffer, &resource_desc, &srv))) {
			buffer->Release();
			return FailedSRVCreation;
		}

		return ok<SBuffer, RenderCreateError>(SBuffer{ buffer, srv, capacity });
	}
};

template<typename T>
struct UAVSBuffer {
	ID3D11Buffer* buffer;
	ID3D11UnorderedAccessView* uav;

	void clean_up() {
		if (buffer) buffer->Release();
		if (uav) uav->Release();
	}

	static Result<UAVSBuffer, RenderCreateError> create(ID3D11Device* device, const T* data, usize len) {
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(T) * len;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(T);

		D3D11_SUBRESOURCE_DATA res_data;
		D3D11_SUBRESOURCE_DATA* res_data_ptr = nullptr;
		if (data != nullptr) {
			res_data.pSysMem = data;
			res_data.SysMemPitch = 0;
			res_data.SysMemSlicePitch = 0;
			res_data_ptr = &res_data;
		}

		ID3D11Buffer* buffer;
		if (FAILED(device->CreateBuffer(&desc, res_data_ptr, &buffer))) {
			return FailedBufferCreation;
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uav_desc.Format = DXGI_FORMAT_UNKNOWN;
		uav_desc.Buffer = {
			0,
			len,
			0,
		};

		ID3D11UnorderedAccessView* uav;
		if (FAILED(device->CreateUnorderedAccessView(buffer, &uav_desc, &uav))) {
			return FailedUAVCreation;
		}

		return ok<UAVSBuffer, RenderCreateError>(UAVSBuffer{ buffer, uav });
	}
};

struct DepthTexture {
	ID3D11DepthStencilView* dsv;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;

	static Result<DepthTexture, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void clean_up();

	void clear(ID3D11DeviceContext* ctx);
};

struct DepthTextures {
	ID3D11DepthStencilView** dsvs;
	ID3D11ShaderResourceView** srvs;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;

	Vec3<u16> size;

	static Result<DepthTextures, RenderCreateError> create(ID3D11Device* device, Vec3<u16> size);

	void resize(ID3D11Device* device, Vec3<u16> size);
	void clean_up();

	void clear(ID3D11DeviceContext* ctx);

	ID3D11DepthStencilView** begin();
	ID3D11DepthStencilView** end();
	ID3D11DepthStencilView** begin() const;
	ID3D11DepthStencilView** end() const;
};

struct RenderTarget {
	ID3D11Texture2D* texture;
	ID3D11UnorderedAccessView* uav;
	ID3D11RenderTargetView* rtv;

	static Result<RenderTarget, RenderCreateError> create(ID3D11Device* device, IDXGISwapChain* swap_chain);

	void resize(ID3D11Device* device, IDXGISwapChain* swap_chain);

	void clean_up();
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

struct CubeTexture {
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;
	std::array<ID3D11UnorderedAccessView*, 6> uav;

	static Result<CubeTexture, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void clean_up();
};

Result<ID3D11PixelShader*, RenderCreateError> load_pixel(ID3D11Device* device, const char* file);

struct VSIL {
	ID3D11VertexShader* vs;
	ID3D11InputLayout* il;
};
Result<VSIL, RenderCreateError> load_vertex(ID3D11Device* device, const char* file, const std::vector<D3D11_INPUT_ELEMENT_DESC>& input);
Result<ID3D11VertexShader*, RenderCreateError> load_vertex_without_layout(ID3D11Device* device, const char* file);

Result<ID3D11ComputeShader*, RenderCreateError> load_compute(ID3D11Device* device, const char* file);

Result<ID3D11GeometryShader*, RenderCreateError> load_geometry(ID3D11Device* device, const char* file);

Result<ID3D11HullShader*, RenderCreateError> load_hull(ID3D11Device* device, const char* file);
Result<ID3D11DomainShader*, RenderCreateError> load_domain(ID3D11Device* device, const char* file);

struct Renderer;
struct RendererCtx;
struct Viewpoint;