#include"assets.h"
#include<rstd/panic.h>
#include<iostream>

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

Image Image::load(const std::string& path, ID3D11Device* device) {
	int width, height, comp;
	auto* image = stbi_load(path.data(), &width, &height, &comp, 0);

	if (image == nullptr) {
		PANIC("Failed to load image file");
	}

	D3D11_TEXTURE2D_DESC desc;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Width = width;
	desc.Height = height;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	switch (comp) {
		case 1: {
			desc.Format = DXGI_FORMAT_R8_UNORM;
			break;
		};
		case 2: {
			desc.Format = DXGI_FORMAT_R8G8_UNORM;
			break;
		};
		case 4: {
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		};
		default: {
			TODO;
		}
	}

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = image;
	data.SysMemPitch = width * sizeof(unsigned char) * comp;
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* texture;
	auto res = device->CreateTexture2D(&desc, &data, &texture);
	if (FAILED(res)) {
		PANIC("Failed to create loaded texture.");
	}
	ID3D11ShaderResourceView* rsv;
	res = device->CreateShaderResourceView(texture, nullptr, &rsv);
	if (FAILED(res)) {
		PANIC("Failed to create shader resource view for texture.");
	}

	return Image{
		texture,
		rsv,
		(usize)width,
		(usize)height,
		(usize)comp,
	};
}

Mesh Mesh::load(const std::string& path, ID3D11Device* device) {
	TODO;
}