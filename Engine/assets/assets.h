#pragma once
#include<string>
#include<unordered_map>
#include<rstd/primitives.h>
#include<rstd/depot.h>
#include<math/aab.h>
#include<math/consts.h>
#include<d3d11.h>
#include<array>
#include <filesystem>
namespace fs = std::filesystem;

struct AssetHandler;
template<typename T>
struct Asset {
	Option<std::string> path;
	T asset;
};

template<typename T>
using AId = Id<Asset<T>>;

struct Image {
	struct Binded {
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* srv;
	};

	u8* data;

	usize width;
	usize height;
	usize channels;
	Option<Binded> binded;

	static Image default_asset();
	static Image load(const fs::path& path, AssetHandler& asset_handler);
	void bind(ID3D11Device* device);
	void clean_up();
};

struct MatTex {
	Option<AId<Image>> tex;
	Vec3<f32> color;

	Vec3<f32> get_color() const {
		return tex.as_ptr().map<Vec3<f32>>([&](auto t) {
			return Vec3<f32>::one();
		}).unwrap_or(this->color);
	}
};

struct MaterialData {
	Vec4<f32> ambient;
	Vec4<f32> diffuse;
	Vec4<f32> specular;
	f32 shininess;
	Vec3<f32> dummy;
};

struct Material {
	MatTex ambient;
	MatTex diffuse;
	MatTex specular;
	f32 shininess;

	std::string name;

	static Material default_asset();
	void bind(ID3D11Device* device, AssetHandler& asset_handler);
	void clean_up();

	MaterialData get_data() const;
};


struct MaterialGroup {
	std::unordered_map<std::string, AId<Material>> mats;

	static MaterialGroup default_asset();
	Option<AId<Material>> get(const std::string& mat) const;
	static MaterialGroup load(const fs::path& path, AssetHandler& asset_handler);
	void clean_up();
};

struct Vertex{
	Vec3<f32> v;
	Vec3<f32> vn;
	Vec2<f32> uv;

	bool operator ==(const Vertex& other) const {
		return memcmp(this, &other, sizeof(Vertex)) == 0;
	}
};

using Index = u16;

struct SubMesh {
	Index start_index;
	Index end_index;
	Option<AId<Material>> material;

	void bind(ID3D11Device* device, AssetHandler& asset_handler);
};

static std::vector<D3D11_INPUT_ELEMENT_DESC> VERTEX_LAYOUT = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

struct Mesh {
	struct Binded {
		ID3D11Buffer* vertex_buffer;
		ID3D11Buffer* index_buffer;
	};
	Vec<Vertex> vertices;
	Vec<Index> indices;
	Vec<SubMesh> submeshes;
	Option<Binded> binded;
	Aabb<f32> bounds{ F32::INF, F32::NEG_INF };

	static Mesh default_asset();
	static Mesh load(const fs::path& path, AssetHandler& asset_handler);
	void bind(ID3D11Device* device, AssetHandler& asset_handler);
	void clean_up();
	void calculate_bounds();
	void push_quad(std::array<Vec3<f32>, 4>);
};

template<typename T>
struct Assets {
	Assets() : _items(), _default_asset(T::default_asset()) { }

	AId<T> load(const fs::path& path, AssetHandler& asset_handler) {
		auto asset = path.string();
		auto got = loaded_paths.find(asset);
		if (got == loaded_paths.end()) {
			T a = T::load(path, asset_handler);
			auto id = _items.insert(Asset<T> { some(asset), a });
			loaded_paths.insert({ asset, id });
			return id;
		}
		else {
			return got->second;
		}
	}

	AId<T> insert(T&& asset, std::string name) {
		return _items.insert(std::move(Asset<T> { some(name), asset }));
	}
	AId<T> insert(T&& asset) {
		return _items.insert(std::move(Asset<T> { none<std::string>(), asset }));
	}

	Option<const T*> get(AId<T> handle) const {
		return _items.get(handle).map<const T*>([&](const Asset<T>* asset) { return &asset->asset; });
	}

	Option<T*> get(AId<T> handle) {
		return _items.get(handle).map<T*>([&](Asset<T>* asset) { return &asset->asset; });
	}

	T* default_asset() {
		return &_default_asset;
	}
	const T* default_asset() const {
		return &_default_asset;
	}

	template<typename F>
	void iter(F f) const {
		_items.iter(f);
	}

	template<typename F>
	void iter(F f) {
		_items.iter(f);
	}

	void clean_up() {
		_default_asset.clean_up();
		_items.values([](Asset<T>& item) {
			item.asset.clean_up();
		});
	}

private:
	T _default_asset;
	Depot<Asset<T>> _items;
	std::unordered_map<std::string, AId<T>> loaded_paths;
};

// Could implement hot reloading
struct AssetHandler {
	template<typename T>
	AId<T> load(const fs::path& path) {
		return assets<T>().load(path, *this);
	}

	template<typename T>
	AId<T> load(const std::string& asset) {
		return load<T>(fs::path(asset));
	}

	template<typename T>
	Option<const T*> get(AId<T> handle) const {
		return assets<T>().get(handle);
	}

	template<typename T>
	Option<T*> get(AId<T> handle) {
		return assets<T>().get(handle);
	}

	template<typename T>
	const T* default_asset() const {
		return assets<T>().default_asset();
	}

	template<typename T>
	T* default_asset() {
		return assets<T>().default_asset();
	}

	template<typename T>
	const T* get_or_default(Option<AId<T>> handle) const {
		return handle
			.map<Option<const T*>>([&](AId<T> handle) {
				return this->get<T>(handle);
			})
			.flatten<const T*>()
			.unwrap_or_else([&]() { return this->default_asset<T>(); });
	}

	template<typename T>
	T* get_or_default(Option<AId<T>> handle) {
		return handle
			.map<Option<T*>>([&](AId<T> handle) {
				return this->get<T>(handle);
			})
			.flatten<T*>()
			.unwrap_or_else([&]() { return this->default_asset<T>(); });
	}

	template<typename T>
	AId<T> insert(T&& asset, std::string name) {
		return assets<T>().insert(std::move(asset), name);
	}

	template<typename T>
	AId<T> insert(T&& asset) {
		return assets<T>().insert(std::move(asset));
	}

	template<typename T>
	const Assets<T>& assets() const {
		return std::get<Assets<T>>(asset_handlers);
	}

	template<typename T>
	Assets<T>& assets() {
		return std::get<Assets<T>>(asset_handlers);
	}

	void clean_up() {
		assets<Image>().clean_up();
		assets<Mesh>().clean_up();
	}

private:
	std::tuple<Assets<Image>, Assets<Mesh>, Assets<MaterialGroup>, Assets<Material>> asset_handlers;
	std::string asset_folder;
};