#pragma once
#include"transform.h"
#include<assets/assets.h>

struct Light {
	Vec3<f32> color;
	f32 strength;
};

struct DirLight {
	Transform transform;
	Light light;

	Mat4<f32> get_texture_mat(const Camera& viewpoint) const;
};

struct SpotLight {
	Transform transform;
	Light light;

	Mat4<f32> get_texture_mat(const Camera& viewpoint) const;
};

struct ObjectData {
	Mat4<f32> world;
};

struct Object {
	Object(Transform transform);

	Object&& with_mesh(AId<Mesh> mesh);
	ObjectData get_data() const;

	Transform transform;
	Option<AId<Mesh>> mesh;
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
	Depot<SpotLight> spot_lights;
	Depot<DirLight> dir_lights;
	Depot<Object> objects;
};