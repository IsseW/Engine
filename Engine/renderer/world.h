#pragma once
#include"transform.h"
#include<assets/assets.h>

using Rgb = Vec3<f32>;
struct Light {
	Rgb _color;
	f32 _strength;
};

struct DirLight {
	Quat<f32> _rotation;
	Light _light;
};

struct PointLight {
	Transform _transform;
	Light _light;
};

struct Object {
	Object(Transform transform, Rgb color);

	Object&& with_mesh(AId<Mesh> mesh);
	Object&& with_image(AId<Image> image);

	Transform transform;
	Option<AId<Mesh>> mesh;
	Rgb color;
	Option<AId<Image>> image;
};

struct Window;

struct World {
	World() = delete;
	World(Camera camera, DirLight light);

	Id<Object> add(Object&& object);
	Id<PointLight> add(PointLight&& object);

	void remove(Id<Object> id);
	void remove(Id<PointLight> id);

	void update(f32 dt, const Window& window);

	Camera camera;
	DirLight dir_light;
	Depot<PointLight> point_lights;
	Depot<Object> objects;
};