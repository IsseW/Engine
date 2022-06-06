#pragma once

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

// Tries to unwrap a result, otherwise return the error.
#define TRY(x, result) {\
	auto x ## __res = result; \
	if (!(x ## __res)) { return std::move((x ## __res).err()); } \
	x = (x ## __res).ok(); \
}
