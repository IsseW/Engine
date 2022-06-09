#pragma once
#include<string>
#include<rstd/primitives.h>
#include<rstd/depot.h>
#include<rstd/vector.h>
#include<math/vec.h>
#include<d3d11.h>

struct Image {
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* rsv;

	usize width;
	usize height;
	usize channels;

	static Image load(const std::string& path, ID3D11Device* device);
};

struct Mesh {
	Vec<Vec3<f32>> vertices;
	Vec<Vec3<f32>> normal;
	Vec<Vec3<f32>> uv0;
	usize num_vertices;
	u16* indices;
	usize num_indices;

	static Mesh load(const std::string& path, ID3D11Device* device);
};

template<typename T>
struct Assets {
	Id<T> load(const std::string& asset, ID3D11Device* device) {
		return items.insert(std::move(T::load(asset, device)));
	}
	Option<const T&> get(Id<T> handle) const {
		return items.get(handle);
	}
private:
	Depot<T> items;
};

// Could implement hot reloading
struct AssetHandler {
	template<typename T>
	Id<T> load(const std::string& asset, ID3D11Device* device) {
		Assets<T>& a = assets<T>();
		return a.load(asset, device);
	}

	template<typename T>
	Option<const T&> get(Id<T> handle) const {
		Assets<T>& a = assets<T>();
		return a.get(handle);
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