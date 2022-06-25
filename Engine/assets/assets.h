#pragma once
#include<string>
#include<rstd/primitives.h>
#include<rstd/depot.h>
#include<rstd/vector.h>
#include<math/vec.h>
#include<d3d11.h>

struct Image {
	struct Binded {
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* rsv;
		ID3D11SamplerState* sampler_state;
	};

	u8* data;

	usize width;
	usize height;
	usize channels;
	Option<Binded> binded;

	static Image default_asset();
	static Image load(const std::string& path);
	void bind(ID3D11Device* device);
	void clean_up();
};

struct Vertex{
	Vec3<f32> v;
	Vec3<f32> vn;
	Vec2<f32> uv;
};

struct SubMesh {
	struct Binded {
		ID3D11Buffer* vertex_buffer;
		ID3D11Buffer* index_buffer;
	};
	Vec<Vertex> vertices;
	Vec<u16> indices;
	Option<Binded> binded;

	void bind(ID3D11Device* device);
	void clean_up();
};

struct Mesh {
	Vec<SubMesh> submeshes;

	static Mesh default_asset();
	static Mesh load(const std::string& path);
	void bind(ID3D11Device* device);
	void clean_up();
};

template<typename T>
struct Assets {
	Assets() : _items(), _default_asset(T::default_asset()) { }

	Id<T> load(const std::string& asset) {
		return _items.insert(std::move(T::load(asset)));
	}
	Id<T> insert(T&& asset) {
		return _items.insert(std::move(asset));
	}

	Option<const T*> get(Id<T> handle) const {
		return _items.get(handle);
	}

	Option<T*> get(Id<T> handle) {
		return _items.get(handle);
	}

	T* default_asset() {
		return &_default_asset;
	}
	const T* default_asset() const {
		return &_default_asset;
	}

	void clean_up() {
		_default_asset.clean_up();
		_items.values([](T* item) {
			item->clean_up();
		});
	}

private:
	T _default_asset;
	Depot<T> _items;
};

// Could implement hot reloading
struct AssetHandler {
	template<typename T>
	Id<T> load(const std::string& asset) {
		return assets<T>().load(asset);
	}

	template<typename T>
	Option<const T*> get(Id<T> handle) const {
		return assets<T>().get(handle);
	}

	template<typename T>
	Option<T*> get(Id<T> handle) {
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
	Id<T> insert(T&& asset) {
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
	std::tuple<Assets<Image>, Assets<Mesh>> asset_handlers;
	std::string asset_folder;
};