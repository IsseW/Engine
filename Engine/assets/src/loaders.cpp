#include<assets/assets.h>
#include<rstd/panic.h>
#include<iostream>
#include<fstream>
#include<sstream>
#include<renderer/util.h>

#define STB_IMAGE_IMPLEMENTATION
#include<assets/stb_image.h>

Image Image::load(const fs::path& path, AssetHandler& asset_handler) {
	int width, height;
	u8* data = stbi_load(path.string().data(), &width, &height, nullptr, 4);

	if (data == nullptr) {
		std::cerr << "Failed loading: " << path << std::endl;
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
	dxname(texture, "Asset texture");
	D3D11_SHADER_RESOURCE_VIEW_DESC resource_desc;
	resource_desc.Texture2D = { 0, 1 };
	resource_desc.Format = desc.Format;
	resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ID3D11ShaderResourceView* srv;
	res = device->CreateShaderResourceView(texture, &resource_desc, &srv);
	if (FAILED(res)) {
		PANIC("Failed to create shader resource view for texture.");
	}
	dxname(srv, "Asset texture srv");

	binded.insert(Binded{
			texture,
			srv,
	});
}


Image Image::default_asset() {
	Image image;
	image.data = new u8[4] { 255, 255, 255, 255 };
	image.width = 1;
	image.height = 1;
	image.channels = 4;
	return image;
}

void Image::clean_up() {
	if (binded.is_some()) {
		Binded b = binded.take().unwrap_unchecked();
		b.srv->Release();
		b.texture->Release();
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

Vec<std::string> split_whitespace(std::string str) {
	Vec<std::string> ret;
	std::string tmp;

	usize i = 0;
	for (; i < str.size() && std::isspace(str[i]); ++i);

	for (; i < str.size(); ++i) {
		// Skip multiple spaces
		bool space = false;
		for (; i < str.size() && std::isspace(str[i]); ++i) space = true;
		char c = str[i];
		if (space) {
			ret.push(std::move(tmp));
			tmp = std::string{};
		}
		tmp += c;
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

Mesh Mesh::load(const fs::path& path, AssetHandler& asset_handler) {
	auto file = std::ifstream{path};
	if (!file.is_open()) {
		std::cout << "Couldn't find: " << path << std::endl;
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

	auto use_mtl = [&](std::string mat) {
		materials.as_ptr().then_do([&](const AId<MaterialGroup>* mat_group) {
			auto mats = asset_handler.get(*mat_group).unwrap();
			mats->get(mat).then_do([&](AId<Material> mat) {
				material.insert(mat);
			});
		});
	};

	while (std::getline(file, line)) {
		auto split = split_whitespace(line);
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
				}) : Vec3<Vec3<f32>>((pos.y - pos.x).cross(pos.z - pos.x).normalized());

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
			Index ends_at = (Index)indices.len();
			if (starts_at < ends_at) {
				submeshes.push(SubMesh{
					starts_at, ends_at, material.take(),
				});
				starts_at = ends_at;
			}
			if (split.len() >= 2) {
				use_mtl(split[1]);
			}
		}
		else if (first == "mtllib") {
			materials.insert(asset_handler.load<MaterialGroup>(fs::path(parent_dir).append(split[1])));
		}
		else if (first == "usemtl") {
			use_mtl(split[1]);
		}
	}
	if (indices.len() > 0) {
		Index end = (Index)indices.len();
		submeshes.push(SubMesh{
			starts_at, end, material.take(),
		});
	}
	file.close();
	auto mesh = Mesh{
		verts,
		indices,
		submeshes
	};
	mesh.calculate_bounds();
	return mesh;
}

void SubMesh::bind(ID3D11Device* device, AssetHandler& asset_handler) {
	material.as_ptr().and_then<Material*>([&](const AId<Material>* mat) {
		return asset_handler.get(*mat);
	}).then_do([&](Material* mat) {
		mat->bind(device, asset_handler);
	});
}

void Mesh::bind(ID3D11Device* device, AssetHandler& asset_handler) {
	for (SubMesh& s : submeshes) {
		s.bind(device, asset_handler);
	}
	if (binded.is_some()) {
		return;
	}
	if (vertices.len() == 0) {
		binded.insert(Binded{ nullptr, nullptr });
		return;
	}

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(Vertex) * vertices.len();
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
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
	dxname(vertex_buffer, "Asset vertex buffer");

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
	dxname(index_buffer, "Asset index buffer");

	binded.insert(Binded{
		vertex_buffer,
		index_buffer
	});
}

Mesh Mesh::default_asset() {
	return Mesh();
}

void Mesh::clean_up() {
	binded.map<bool>([](Binded binded) {
		if (binded.vertex_buffer) binded.vertex_buffer->Release();
		if (binded.index_buffer) binded.index_buffer->Release();
		return true;
	});
}


void Mesh::calculate_bounds() {
	for (const Vertex& vertex : vertices) {
		bounds.grow_to_contain(vertex.v.xyz());
	}
}

Material Material::default_asset() {
	MatTex def = MatTex{
		none<AId<Image>>(),
		Vec3<f32>(1.0, 1.0, 1.0),
	};
	MatTex norm = MatTex{
		none<AId<Image>>(),
		Vec3<f32>(0.5, 0.5, 1.0),
	};
	return Material{
		def, def, def, norm, 1.0f,
	};
}

void Material::bind(ID3D11Device* device, AssetHandler& asset_handler) {
	auto bind_tex = [&](const auto& tex) {
		tex.tex.as_ptr().then_do([&](const AId<Image>* image) {
			asset_handler.get(*image).then_do([&](Image* image) {
				image->bind(device);
			});
		});
	};

	bind_tex(ambient);
	bind_tex(diffuse);
	bind_tex(specular);
	bind_tex(normal);
}

void Material::clean_up() {}

MaterialGroup MaterialGroup::default_asset() {
	return MaterialGroup{};
}
Option<AId<Material>> MaterialGroup::get(const std::string& mat) const {
	auto f = mats.find(mat);
	if (f == mats.end()) {
		return none<AId<Material>>();
	}
	return some(f->second);
}
MaterialGroup MaterialGroup::load(const fs::path& path, AssetHandler& asset_handler) {
	auto file = std::ifstream{ path };
	if (!file.is_open()) {
		PANIC("Unable to load material!");
	}
	auto parent_dir = path.parent_path();
	std::string line {};

	std::unordered_map<std::string, AId<Material>> materials {};

	std::string name {};
	Material material = Material::default_asset();

	auto insert = [&]() {
		if (name.size() > 0) {
			auto mat = asset_handler.insert(std::move(material), name);
			materials.insert({ name, mat });
			name.clear();
			material = Material::default_asset();
		}
	};

	while (std::getline(file, line)) {
		auto split = split_whitespace(line);
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
			material.name = split[1];
		}
		else if (first == "Ns") {
			material.shininess = std::stof(split[1]);
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
		else if (first == "map_Ka") {
			material.ambient.tex.insert(asset_handler.load<Image>(fs::path(parent_dir).append(split[1])));
		}
		else if (first == "map_Kd") {
			material.diffuse.tex.insert(asset_handler.load<Image>(fs::path(parent_dir).append(split[1])));
		}
		else if (first == "map_Ks") {
			material.specular.tex.insert(asset_handler.load<Image>(fs::path(parent_dir).append(split[1])));
		}
		else if (first == "norm") {
			material.normal.tex.insert(asset_handler.load<Image>(fs::path(parent_dir).append(split[1])));
		}
	}
	insert();

	file.close();
	return MaterialGroup{
		materials,
	};
}
void MaterialGroup::clean_up() {}

MaterialData Material::get_data() const {
	return MaterialData {
		ambient.get_color().with_w(1.0),
		diffuse.get_color().with_w(1.0),
		specular.get_color().with_w(1.0),
		normal.get_color().with_w(1.0),
		shininess,
	};
}