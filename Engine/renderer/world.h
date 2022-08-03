#pragma once
#include"particle.h"

struct Light {
	Vec3<f32> color;
	f32 strength;
};

struct DirLight {
	Transform transform;
	Light light;

	Mat4<f32> get_view_mat(const Camera& viewpoint) const;
	Mat4<f32> get_proj_mat(const Camera& viewpoint) const;
};

struct SpotLight {
	Transform transform;
	Light light;

	Mat4<f32> get_view_mat(const Camera& viewpoint) const;
	Mat4<f32> get_proj_mat(const Camera& viewpoint) const;
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
	Id<ParticleSystem> add(ParticleSystem&& object);

	void remove(Id<Object> id);
	void remove(Id<SpotLight> id);
	void remove(Id<DirLight> id);
	void remove(Id<ParticleSystem> id);

	void update(f32 dt, const Window& window);

	void clean_up();

	f32 time{0.0f};
	Camera camera;
	Depot<SpotLight> spot_lights;
	Depot<DirLight> dir_lights;
	Depot<Object> objects;
	Depot<ParticleSystem> particle_systems;
};