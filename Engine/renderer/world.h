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
	Object(Transform transform, Id<Mesh> mesh, Rgb color);
	Transform transform;
	Id<Mesh> mesh;
	Rgb color;
	Option<Id<Image>> image;
};

struct World {
	World() = delete;
	World(Camera camera, DirLight light);

	Id<Object> add(Object&& object);
	Id<PointLight> add(PointLight&& object);

	void update(f32 dt);

	Camera camera;
	DirLight dir_light;
	Depot<PointLight> point_lights;
	Depot<Object> objects;
};