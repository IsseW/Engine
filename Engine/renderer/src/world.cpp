#include<renderer/world.h>


Object::Object(Transform transform, Id<Mesh> mesh, Rgb color) : transform(transform), mesh(mesh), color(color) {}

World::World(Camera camera, DirLight light) : camera(camera), dir_light(light), point_lights(), objects() { }

Id<Object> World::add(Object&& object) {
	return objects.insert(std::move(object));
}
Id<PointLight> World::add(PointLight&& object) {
	return point_lights.insert(std::move(object));
}

void World::update(f32 dt) {

}