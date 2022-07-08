#pragma once
#include"transform.h"
#include<assets/assets.h>

struct Light {
	Vec3<f32> color;
	f32 strength;
};

struct DirLight {
	Quat<f32> rotation;
	Light light;
};

struct SpotLight {
	Transform transform;
	Light light;
};

struct Object {
	Object(Transform transform, Vec3<f32> color);

	Object&& with_mesh(AId<Mesh> mesh);
	Object&& with_image(AId<Image> image);

	Transform transform;
	Option<AId<Mesh>> mesh;
	Vec3<f32> color;
	Option<AId<Image>> image;
};

struct Window;

struct World {
	World() = delete;
	World(Camera camera);

	Id<Object> add(Object&& object);
	Id<SpotLight> add(SpotLight&& object);
	Id<DirLight> add(DirLight&& object);

	void remove(Id<Object> id);
	void remove(Id<SpotLight> id);
	void remove(Id<DirLight> id);

	void update(f32 dt, const Window& window);

	Camera camera;
	Depot<SpotLight> point_lights;
	Depot<DirLight> dir_lights;
	Depot<Object> objects;
};