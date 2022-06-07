#pragma once
#include <d3d11.h>

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

