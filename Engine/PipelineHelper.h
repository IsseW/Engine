#pragma once

#include <array>
#include <d3d11.h>
#include <directxmath.h>

struct SimpleVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 textcoord;
	DirectX::XMFLOAT3 norm;

	SimpleVertex() = default;
	SimpleVertex(const SimpleVertex& other) = default;

	SimpleVertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& textcoord, const DirectX::XMFLOAT3& normal) : pos(position), textcoord(textcoord), norm(normal) {}
};

const SimpleVertex MESH[] =
{
	{ {0.5f, -0.5f, 0.0f}, {0, 1}, {0, 0, 1} },
	{ {-0.5f, -0.5f, 0.0f}, {1, 1}, {0, 0, 1} },
	{ {-0.5f, 0.5f, 0.0f}, {1, 0}, {0, 0, 1} },

	{ {-0.5f, 0.5f, 0.0f}, {1, 0}, {0, 0, 1} },
	{ {0.5f, -0.5f, 0.0f}, {0, 1}, {0, 0, 1} },
	{ {0.5f, 0.5f, 0.0f}, {0, 0}, {0, 0, 1} },
};

struct Light {
	DirectX::XMFLOAT3 pos;
	float strength;
};

const Light LIGHT = {
	{0.0f, 0.0f, -1.0f},
	1000.0f,
};

struct PS_CONSTANT_BUFFER {
	Light light;
};

struct VS_CONSTANT_BUFFER {
	DirectX::XMFLOAT4X4 world_matrix;
	DirectX::XMFLOAT4X4 view_matrix;
	DirectX::XMFLOAT4X4 projection_matrix;
	Light light;
};

const DirectX::XMFLOAT4X4 VIEW_MATRIX = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1,
};

const DirectX::XMFLOAT4X4 WORLD_MATRIX = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1,
};




template<class T>
struct DXPointer {
	T* ptr{};

	DXPointer() = default;
	DXPointer(T* ptr) : ptr(ptr) {}

	DXPointer& operator=(const T*& ptr) {
		this->ptr = ptr;
		return *this;
	}

	T*& operator->() {
		return ptr;
	}

	operator T*& () {
		return ptr;
	}

	T** get() {
		return &ptr;
	}

	~DXPointer() {
		if (ptr) {
			ptr->Release();
		}
	}
};

struct Pipeline {
	DXPointer<ID3D11Device> device;
	DXPointer<ID3D11DeviceContext> context;
	DXPointer<IDXGISwapChain> swapChain;
	DXPointer<ID3D11RenderTargetView> rtv;
	DXPointer<ID3D11Texture2D> dsTexture;
	DXPointer<ID3D11DepthStencilView> dsView;
	D3D11_VIEWPORT viewport;

	DXPointer<ID3D11VertexShader> vShader;
	DXPointer<ID3D11Buffer> vsConstantBuffer;
	DXPointer<ID3D11PixelShader> pShader;
	DXPointer<ID3D11Buffer> psConstantBuffer;
	DXPointer<ID3D11InputLayout> inputLayout;
	DXPointer<ID3D11Buffer> vertexBuffer;
	DXPointer<ID3D11Texture2D> texture;
	DXPointer<ID3D11ShaderResourceView> shaderResource;
	DXPointer<ID3D11SamplerState> samplerState;
};

bool SetupPipeline(Pipeline& pipeline);

DirectX::XMFLOAT4X4 CreateProjection(Pipeline& pipeline);