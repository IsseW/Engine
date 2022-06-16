#include"assets.h"
#include<rstd/panic.h>
#include<iostream>
#include<fstream>
#include<sstream>
#include<unordered_map>

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
	std::string line;
	Vec<SubMesh> submeshes;
	Vec<Vec3<f32>> vertices;
	Vec<Vec3<f32>> normals;
	Vec<Vec3<f32>> uvs;
	std::unordered_map<IndexTuple, Vertex> submesh_vertices;
	Vec<IndexTuple> indices;

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
			if (split.len() != 4) {
				PANIC("");
			}
			auto a = std::stof(split[1]);
			auto b = std::stof(split[2]);
			auto c = std::stof(split[3]);
			uvs.push(Vec3<f32>{a, b, c});
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
			if (submesh_vertices.contains(a_tuple)) {
				indices.push(std::move(a_tuple));
			}
			else {
				submesh_vertices.insert({ std::move(a_tuple),  });
				indices.push(index_count++);
			}
			if (submesh_vertices.contains(b_tuple)) {
				indices.push(std::move(b_tuple));
			}
			else {
				submesh_vertices.insert({ std::move(b_vert), index_count });
				indices.push(index_count++);
			}
			if (submesh_vertices.contains(c_tuple)) {
				indices.push(std::move(c_tuple));
			}
			else {
				submesh_vertices.insert({ std::move(c_vert), index_count});
				indices.push(index_count++);
			}
		}
		else if (first == "o") {
			if (index_count != 0 && vertices.len() != 0) {
				auto vec = Vec<Vertex>{ Vertex{0,0,0}, index_count };
				//for (auto [vertex, index] : submesh_vertices) {
				//	vec[index] = vertex;
				//}
				submeshes.push(SubMesh{
					std::move(vec), std::move(indices)
				});
				vertices = Vec<Vec3<f32>>{};
				normals = Vec<Vec3<f32>>{};
				uvs = Vec<Vec3<f32>>{};
				submesh_vertices = std::unordered_map<IndexTuple, Vertex>{};
				indices = Vec<u16>{};
				index_count = 0;
			}
			else {
				PANIC("");
			}
		}
	}
	file.close();
	return Mesh{
		submeshes
	};
}