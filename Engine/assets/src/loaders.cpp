#include<assets/assets.h>
#include<rstd/panic.h>
#include<iostream>
#include<fstream>
#include<sstream>

#define STB_IMAGE_IMPLEMENTATION
#include<assets/stb_image.h>

Image Image::load(const std::string& path) {
	int width, height;
	u8* data = stbi_load(path.data(), &width, &height, nullptr, 4);

	if (data == nullptr) {
		PANIC("Failed to load image file");
	}
	Image image;
	image.data = data;
	image.width = width;
	image.height = height;
	image.channels = 4;
	return image;
}

void Image::bind(ID3D11Device* device) {
	if (binded.is_some()) {
		return;
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
	D3D11_SHADER_RESOURCE_VIEW_DESC resource_desc;
	resource_desc.Texture2D = { 0, 1 };
	resource_desc.Format = desc.Format;
	resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ID3D11ShaderResourceView* rsv;
	res = device->CreateShaderResourceView(texture, &resource_desc, &rsv);
	if (FAILED(res)) {
		PANIC("Failed to create shader resource view for texture.");
	}

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.MaxAnisotropy = 4;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* sampler_state;
	res = device->CreateSamplerState(&sampler_desc, &sampler_state);
	if (FAILED(res)) {
		PANIC("Failed to create sampler state for texture.");
	}

	binded.insert(Binded{
			texture,
			rsv,
			sampler_state
		});
}


Image Image::default_asset() {
	Image image;
	image.data = new u8[4] { 1, 1, 1, 1 };
	image.width = 1;
	image.height = 1;
	image.channels = 4;
	return image;
}

void Image::clean_up() {
	if (binded.is_some()) {
		Binded b = binded.take().unwrap_unchecked();
		b.rsv->Release();
		b.texture->Release();
		b.sampler_state->Release();
	}
	stbi_image_free(data);
	data = nullptr;
	width = 0;
	height = 0;
	channels = 0;
}


Vec<std::string> split_string(std::string&& str, char delim) {
	Vec<std::string> ret;
	std::string tmp;
	for (char c: str) {
		if (c != delim) {
			tmp += c;
		}
		else {
			ret.push(std::move(tmp));
			tmp = std::string{};
		}
	}
	if (tmp.size() != 0) {
		ret.push(std::move(tmp));
	}
	return ret;
}

#pragma pack(1)
struct IndexTuple {
	u16 a, b, c;

	bool operator==(const IndexTuple other) const {
		return this->a == other.a && this->b == other.b && this->c == other.c;
	}
};
template<>
struct std::hash<IndexTuple> {
	usize operator()(const IndexTuple& k) const {
		return (std::hash<u16>{}(k.a)) ^ (std::hash<u16>{}(k.b)) ^ (std::hash<u16>{}(k.c));
	}
};

Mesh Mesh::load(const std::string& path) {
	auto file = std::ifstream{path};
	if (!file.is_open()) {
		PANIC("Unable to load mesh!");
	}
	std::string line;
	Vec<SubMesh> submeshes;
	Vec<Vec3<f32>> vertices;
	Vec<Vec3<f32>> normals;
	Vec<Vec2<f32>> uvs;
	std::unordered_map<IndexTuple, usize> submesh_vertices;
	Vec<Vertex> verts;
	Vec<u16> indices;

	while (std::getline(file, line)) {
		auto split = split_string(std::move(line), ' ');
		line = std::string{};
		auto first = split[0];
		if (first == "v") {
			if (split.len() != 4) {
				PANIC("");
			}
			auto a = std::stof(split[1]);
			auto b = std::stof(split[2]);
			auto c = std::stof(split[3]);
			vertices.push(Vec3<f32>{a, b, c});
		}
		else if (first == "vt") {
			if (split.len() != 3) {
				PANIC("");
			}
			auto a = std::stof(split[1]);
			auto b = std::stof(split[2]);
			uvs.push(Vec2<f32>{a, b});
		}
		else if (first == "vn") {
			if (split.len() != 4) {
				PANIC("");
			}
			auto a = std::stof(split[1]);
			auto b = std::stof(split[2]);
			auto c = std::stof(split[3]);
			normals.push(Vec3<f32>{a, b, c});
		}
		else if (first == "f") {
			if (split.len() != 4) {
				PANIC("");
			}
			auto a = split[1];
			auto a_split = split_string(std::move(a), '/');
			auto b = split[2];
			auto b_split = split_string(std::move(b), '/');
			auto c = split[3];
			auto c_split = split_string(std::move(c), '/');
			if (a_split.len() != 3 || c_split.len() != 3 || b_split.len() != 3) {
				PANIC("");
			}
			IndexTuple a_tuple{(u16)std::stoi(a_split[0]), (u16)std::stoi(a_split[1]), (u16)std::stoi(a_split[2])};
			IndexTuple b_tuple{(u16)std::stoi(b_split[0]), (u16)std::stoi(b_split[1]), (u16)std::stoi(b_split[2])};
			IndexTuple c_tuple{(u16)std::stoi(c_split[0]), (u16)std::stoi(c_split[1]), (u16)std::stoi(c_split[2])};
			auto f = [&](IndexTuple tuple) {
				if (submesh_vertices.contains(tuple)) {
					indices.push(std::move(submesh_vertices[tuple]));
				}
				else {
					auto vertex = vertices[tuple.a - 1];
					auto texture = uvs[tuple.b - 1];
					auto normal = normals[tuple.c - 1];
					auto vert = Vertex{
						vertex,
						normal,
						texture,
					};
					auto len = verts.len();
					submesh_vertices.insert({ std::move(tuple), len });
					indices.push(std::move(len));
					verts.push(std::move(vert));
				}
			};
			f(a_tuple);
			f(b_tuple);
			f(c_tuple);
		}
		else if (first == "o") {
			if (submesh_vertices.size() != 0) {
				submeshes.push(SubMesh{
					std::move(verts), std::move(indices)
				});
			}
			verts = Vec<Vertex>{};
			indices = Vec<u16>{};
		} 
	}
	if (submesh_vertices.size() != 0) {
		submeshes.push(SubMesh{
			std::move(verts), std::move(indices)
		});
	}
	file.close();
	return Mesh{
		submeshes
	};
}

void SubMesh::bind(ID3D11Device* device) {
	if (binded.is_some()) {
		return;
	}
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(Vertex) * vertices.len();
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertices.raw();
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	ID3D11Buffer* vertex_buffer;
	if (FAILED(device->CreateBuffer(&bufferDesc, &data, &vertex_buffer))) {
		PANIC("Failed to create vertex buffer for model");
	}

	bufferDesc.ByteWidth = sizeof(u16) * indices.len();
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	data.pSysMem = indices.raw();
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;
	ID3D11Buffer* index_buffer;
	if (FAILED(device->CreateBuffer(&bufferDesc, &data, &index_buffer))) {
		PANIC("Failed to create index buffer for model");
	}

	binded.insert(Binded{
		vertex_buffer,
		index_buffer
	});
}

void SubMesh::clean_up() {
	if (binded.is_some()) {
		Binded b = binded.take().unwrap_unchecked();
		b.vertex_buffer->Release();
		b.index_buffer->Release();
	}
}

void Mesh::bind(ID3D11Device* device) {
	for (SubMesh& s : submeshes) {
		s.bind(device);
	}
}

Mesh Mesh::default_asset() {
	return Mesh();
}

void Mesh::clean_up() {
	for (SubMesh& submesh : submeshes) {
		submesh.clean_up();
	}
}