#include"assets.h"
#include<rstd/panic.h>
#include<iostream>
#include<fstream>

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

Image Image::load(const std::string& path) {
	int width, height, comp;
	u8* data = stbi_load(path.data(), &width, &height, &comp, 0);

	if (data == nullptr) {
		PANIC("Failed to load image file");
	}

	return Image{
		data,
		(u32)width,
		(u32)height,
		(u32)comp,
		none<Binded>()
	};
}

void Image::bind(ID3D11Device* device) {
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

	switch (channels) {
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
	data.pSysMem = this->data;
	data.SysMemPitch = width * sizeof(unsigned char) * channels;
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

	binded.insert(Binded{
			texture,
			rsv
		});
}


Mesh Mesh::load(const std::string& path, ID3D11Device* device) {
	auto file = std::ifstream{path};
	std::string line;
	while (std::getline(file, line)) {
		printf("%s", line.c_str());
	}
	file.close();
}