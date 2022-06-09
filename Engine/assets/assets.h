#pragma once
#include<string>
#include<rstd/primitives.h>
#include<rstd/depot.h>
#include<math/vec.h>

struct Image {


	usize width;
	usize height;
	usize channels;

	static Image load(const std::string& path);
};

struct Mesh {
	Vec3<f32>* vertices;
	Vec3<f32>* normal;
	Vec3<f32>* uv0;
	usize num_vertices;
	u16* indices;
	usize num_indices;

	static Mesh load(const std::string& path);
};

template<typename T>
struct Assets {
	Id<T> load(const std::string& asset) {
		return items.insert(std::move(T::load(asset)));
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
	Id<T> load(const std::string& asset) {
		Assets<T>& a = assets<T>();
		return a.load(asset);
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