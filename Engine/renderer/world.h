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
	Transform transform;
	Id<Mesh> mesh;
};

struct World {
	Camera camera;

	DirLight dir_light;
	Vec<PointLight> point_lights;

	Vec<Object> objects;
};