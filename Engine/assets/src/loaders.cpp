#include<assets/assets.h>
#include<rstd/panic.h>
#include<iostream>
#include<fstream>
#include<sstream>

#define STB_IMAGE_IMPLEMENTATION
#include<assets/stb_image.h>

Image Image::load(const std::filesystem::path& path, AssetHandler& asset_handler) {
	int width, height;
	u8* data = stbi_load(path.string().data(), &width, &height, nullptr, 4);

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


Vec<std::string> split_string(std::string str, char delim) {
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


template<>
struct std::hash<Vertex> {
	usize operator()(const Vertex& k) const {
		auto hash = [](auto vec) {
			return vec.map<u32>([](f32 e) {
				return *(u32*)&e;
			}).reduce([](u32 a, u32 b) {
				return _rotr(a, 1) ^ b;
			});
		};
		return hash(k.v) ^ _rotr(hash(k.uv), 7) ^ _rotr(hash(k.vn), 17);
	}
};

Mesh Mesh::load(const std::filesystem::path& path, AssetHandler& asset_handler) {
	auto file = std::ifstream{path};
	if (!file.is_open()) {
		PANIC("Unable to load mesh!");
	}

	auto parent_dir = path.parent_path();

	std::string line;
	Vec<SubMesh> submeshes;

	Vec<Vec3<f32>> vertices;
	Vec<Vec3<f32>> normals;
	Vec<Vec2<f32>> uvs;

	Vec<Vertex> verts;
	Vec<Index> indices;

	std::unordered_map<Vertex, Index> vertice_indices;

	Option<AId<Material>> material;
	Option<AId<MaterialGroup>> materials;
	Index starts_at = 0;

	while (std::getline(file, line)) {
		auto split = split_string(line, ' ');
		if (split.len() < 1) {
			continue;
		}
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
			if (split.len() < 3) {
				PANIC("Expected at least two values for vertex uv");
			}
			auto a = std::stof(split[1]);
			auto b = std::stof(split[2]);
			uvs.push(Vec2<f32>{a, b});
		}
		else if (first == "vn") {
			if (split.len() != 4) {
				PANIC("Expected 3 values for vertex normal");
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
			ASSERT(a_split.len() >= 1 && a_split.len() <= 3 && a_split.len() == c_split.len() && a_split.len() == b_split.len());
			
			auto parse_verts = [&](Index i) {
				return Vec3<std::string>(a_split[i], b_split[i], c_split[i]).map<Index>([](std::string str) {
					return (Index)std::stoul(str) - 1;
				});
			};

			auto pos = parse_verts(0).map<Vec3<f32>>([&](Index i) {
				return vertices[i];
			});

			// Skip parsing uv if it's in the format <pos> of <pos>//<norm>
			auto uv = a_split.len() >= 2 && a_split[1].size() > 0 ? 
				parse_verts(1).map<Vec2<f32>>([&](Index i) {
					return uvs[i];
				}) : Vec3<Vec2<f32>>();

			auto norm = a_split.len() >= 3 ? 
				parse_verts(2).map<Vec3<f32>>([&](Index i) {
					return normals[i];
				}) : Vec3<Vec3<f32>>((pos.y - pos.x).cross(pos.z - pos.x));

			auto triangle = pos.map<Vertex>([](Vec3<f32> pos, Vec2<f32> uv, Vec3<f32> norm) {
				return Vertex{
					pos,
					norm,
					uv,
				};
			}, uv, norm).map<Index>([&](Vertex vert) {
				auto found = vertice_indices.find(vert);

				if (found == vertice_indices.end()) {
					Index i = (Index)verts.len();
					vertice_indices.insert({ vert, i });
					verts.push(std::move(vert));
					return i;
				}
				else {
					return found->second;
				}
			});

			indices.push(triangle.x);
			indices.push(triangle.y);
			indices.push(triangle.z);
		}
		else if (first == "o" || first == "g") {
			if (indices.len() > 0) {
				Index end = (Index)indices.len() - 1;
				submeshes.push(SubMesh{
					starts_at, end, material.take(),
				});
				starts_at = end + 1;
			}
		}
		else if (first == "mtllib") {
			materials.insert(asset_handler.load<MaterialGroup>(parent_dir.append(split[1])));
		}
		else if (first == "usemtl") {
			materials.as_ptr().then_do([&](const AId<MaterialGroup>* mat_group) {
				auto mats = asset_handler.get(*mat_group).unwrap();
				material.insert(mats->get(split[1]));
			});
		}
	}
	if (indices.len() > 0) {
		Index end = (Index)indices.len() - 1;
		submeshes.push(SubMesh{
			starts_at, end, material.take(),
		});
	}
	file.close();
	return Mesh{
		verts,
		indices,
		submeshes
	};
}

void SubMesh::bind(ID3D11Device* device, AssetHandler& asset_handler) {
	material.and_then<Material*>([&](AId<Material> mat) {
		return asset_handler.get(mat);
	}).then_do([&](Material* mat) {
		mat->bind(device, asset_handler);
	});
}

void Mesh::bind(ID3D11Device* device, AssetHandler& asset_handler) {
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

	bufferDesc.ByteWidth = sizeof(Index) * indices.len();
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

	for (SubMesh& s : submeshes) {
		s.bind(device, asset_handler);
	}
}

Mesh Mesh::default_asset() {
	return Mesh();
}

void Mesh::clean_up() {
	binded.map<bool>([](Binded binded) {
		binded.vertex_buffer->Release();
		binded.index_buffer->Release();
		return true;
	});
}


void Mesh::calculate_bounds() {
	for (const Vertex& vertex : vertices) {
		bounds.grow_to_contain(vertex.v);
	}
}

Material Material::default_asset() {
	MatTex def = MatTex{
		none<AId<Image>>(),
		Vec3<f32>(1.0, 1.0, 1.0),
	};
	return Material{
		def, def, def,
		MatMap {
			none<AId<Image>>(),
			1.0,
		},
	};
}

void Material::bind(ID3D11Device* device, AssetHandler& asset_handler) {
	auto bind_tex = [&](auto& tex) {
		tex.tex.then_do([&](AId<Image> image) {
			asset_handler.get(image).then_do([&](Image* image) {
				image->bind(device);
			});
		});
	};

	bind_tex(ambient);
	bind_tex(diffuse);
	bind_tex(specular);
	bind_tex(shinyness);
}

void Material::clean_up() {}

MaterialGroup MaterialGroup::default_asset() {
	return MaterialGroup{};
}
AId<Material> MaterialGroup::get(const std::string& mat) const {
	return mats.at(mat);
}
MaterialGroup MaterialGroup::load(const std::filesystem::path& path, AssetHandler& asset_handler) {
	auto file = std::ifstream{ path };
	if (!file.is_open()) {
		PANIC("Unable to load material!");
	}
	std::string line;

	std::unordered_map<std::string, AId<Material>> materials;

	std::string name;
	Material material;

	auto insert = [&]() {
		if (name.size() > 0) {
			auto mat = asset_handler.insert(std::move(material), name);
			materials.insert({ name, mat });
			name = "";
		}
	};

	while (std::getline(file, line)) {
		auto split = split_string(std::move(line), ' ');
		line = std::string{};
		if (split.len() < 1) {
			continue;
		}
		auto first = split[0];

		auto parse_color = [&]() {
			return Vec3<std::string>(split[1], split[2], split[3]).map<f32>([](std::string e) {
				return std::stof(e);
			});
		};

		if (first == "newmtl") {
			insert();
			name = split[1];
		}
		else if (first == "Ns") {
			material.shinyness.value = std::stof(split[1]);
		}
		else if (first == "Ka") {
			material.ambient.color = parse_color();
		}
		else if (first == "Kd") {
			material.diffuse.color = parse_color();
		}
		else if (first == "Ks") {
			material.specular.color = parse_color();
		}
		else if (first == "map_Ns") {
			material.shinyness.tex.insert(asset_handler.load<Image>(split[1]));
		}
		else if (first == "map_Ka") {
			material.ambient.tex.insert(asset_handler.load<Image>(split[1]));
		}
		else if (first == "map_Kd") {
			material.diffuse.tex.insert(asset_handler.load<Image>(split[1]));
		}
		else if (first == "map_Ks") {
			material.specular.tex.insert(asset_handler.load<Image>(split[1]));
		}
	}
	insert();

	file.close();
	return MaterialGroup{
		materials,
	};
}
void MaterialGroup::clean_up() {}