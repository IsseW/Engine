#pragma once
#include"particle.h"
#include<data_structures/sparse_octree.h>
#include<math/consts.h>

enum class LightType {
	Directional,
	Spot,
};

struct Light {
	Transform transform;
	Vec3<f32> color;
	// Is directional if false
	LightType light_type;
	f32 angle { F32::PI / 4.0f };
	f32 far_plane { F32::PI / 4.0f };

	Mat4<f32> get_view_mat(const Camera& viewpoint) const;
	Mat4<f32> get_proj_mat(const Camera& viewpoint) const;

	static Light directional(Transform trans, Vec3<f32> col);
	static Light spot(Transform trans, Vec3<f32> col, f32 angle);
};

struct ObjectData {
	Mat4<f32> world;
};

struct Object {
	Object(Transform transform);

	Object&& with_mesh(AId<Mesh> mesh);
	Aabb<f32> get_bounds(const AssetHandler& assets) const;

	Transform transform;
	Option<AId<Mesh>> mesh;


	bool tesselate{ true };
};

struct Reflective {
	Reflective(Transform transform);
	Reflective(ID3D11Device* device, Transform transform);

	Reflective&& with_mesh(AId<Mesh> mesh);
	Aabb<f32> get_bounds(const AssetHandler& assets) const;

	void create_texture(ID3D11Device* device);

	void clean_up();

	CubeTexture cube_texture;

	Transform transform;
	Option<AId<Mesh>> mesh;
	bool tesselate{ true };
};

struct Window;

struct World {
	World() = delete;
	World(Camera camera);

	Id<Light> add(Light&& object);
	Id<Object> add(Object&& object);
	Id<Reflective> add(Reflective&& object);
	Id<ParticleSystem> add(ParticleSystem&& object);

	void remove(Id<Light> id);
	void remove(Id<Object> id);
	void remove(Id<Reflective> id);
	void remove(Id<ParticleSystem> id);

	void update(f32 dt, const Window& window, const AssetHandler& asset_handler);

	void clean_up();

	f32 time{0.0f};
	Camera camera;
	Depot<Light> lights;
	Depot<Object> objects;
	Depot<Reflective> reflective;
	Depot<ParticleSystem> particle_systems;
	SparseOctree<Id<Object>, 5> octree_obj{ Vec3<f32>{0,0,0}, 80 };
	SparseOctree<Id<Reflective>, 5> octree_reflective{ Vec3<f32>{0,0,0}, 80 };
	Vec<Id<Object>> object_to_add_into_octree;
};