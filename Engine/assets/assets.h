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
	};


	u8* data;

	usize width;
	usize height;
	usize channels;
	Option<Binded> binded;

	static Image load(const std::string& path);
	void bind(ID3D11Device* device);
};

struct Vertex{
	Vec3<f32> v;
	Vec3<f32> vn;
	Vec3<f32> uv;
};

struct SubMesh {
	Vec<Vertex> vertices;
	Vec<u16> indices;
};

struct Mesh {
	Vec<SubMesh> submeshes;

	static Mesh load(const std::string& path);
};

template<typename T>
struct Assets {
	Id<T> load(const std::string& asset) {
		return items.insert(std::move(T::load(asset)));
	}
	Id<T> insert(T&& asset) {
		return items.insert(std::move(asset));
	}

	Option<const T*> get(Id<T> handle) const {
		return items.get(handle);
	}
private:
	Depot<T> items;
};

// Could implement hot reloading
struct AssetHandler {
	template<typename T>
	Id<T> load(const std::string& asset) {
		Assets<T>& a = assets<T>();
		return a.load(asset);
	}

	template<typename T>
	Option<const T*> get(Id<T> handle) const {
		const Assets<T>& a = assets<T>();
		return a.get(handle);
	}

	template<typename T>
	Id<T> insert(T&& asset) {
		return assets<T>().insert(asset);
	}

	template<typename T>
	const Assets<T>& assets() const {
		return std::get<Assets<T>>(asset_handlers);
	}

	template<typename T>
	Assets<T>& assets() {
		return std::get<Assets<T>>(asset_handlers);
	}

private:
	std::tuple<Assets<Image>, Assets<Mesh>> asset_handlers;
	std::string asset_folder;
};