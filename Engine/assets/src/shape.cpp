#include<assets/shape.h>

void SubMesh::push_quad(std::array<Vec3<f32>, 4> points) {
	Vec3<f32> normal = (points[1] - points[0]).cross(points[2] - points[0]);
	Index i = (Index)vertices.len();
	vertices.push(Vertex{
			points[0],
			normal,
			Vec2<f32>(0.0f, 0.0f),
		});
	vertices.push(Vertex{
			points[1],
			normal,
			Vec2<f32>(1.0f, 0.0f),
		});
	vertices.push(Vertex{
			points[2],
			normal,
			Vec2<f32>(0.0f, 1.0f),
		});
	vertices.push(Vertex{
			points[3],
			normal,
			Vec2<f32>(1.0f, 1.0f),
		});
	indices.push(i + 0);
	indices.push(i + 2);
	indices.push(i + 1);

	indices.push(i + 1);
	indices.push(i + 2);
	indices.push(i + 3);
}

Mesh unit_cube() {
	SubMesh mesh;
	mesh.push_quad({
			Vec3<f32>( 1.0f, -1.0f, -1.0f),
			Vec3<f32>( 1.0f, -1.0f,  1.0f),
			Vec3<f32>( 1.0f,  1.0f, -1.0f),
			Vec3<f32>( 1.0f,  1.0f,  1.0f),
		});
	mesh.push_quad({
			Vec3<f32>( 1.0f, -1.0f,  1.0f),
			Vec3<f32>(-1.0f, -1.0f,  1.0f),
			Vec3<f32>( 1.0f,  1.0f,  1.0f),
			Vec3<f32>(-1.0f,  1.0f,  1.0f),
		});
	mesh.push_quad({
			Vec3<f32>(-1.0f, -1.0f,  1.0f),
			Vec3<f32>(-1.0f, -1.0f, -1.0f),
			Vec3<f32>(-1.0f,  1.0f,  1.0f),
			Vec3<f32>(-1.0f,  1.0f, -1.0f),
		});
	mesh.push_quad({
			Vec3<f32>(-1.0f, -1.0f, -1.0f),
			Vec3<f32>( 1.0f, -1.0f, -1.0f),
			Vec3<f32>(-1.0f,  1.0f, -1.0f),
			Vec3<f32>( 1.0f,  1.0f, -1.0f),
		});
	mesh.push_quad({
			Vec3<f32>( 1.0f,  1.0f, -1.0f),
			Vec3<f32>( 1.0f,  1.0f,  1.0f),
			Vec3<f32>(-1.0f,  1.0f, -1.0f),
			Vec3<f32>(-1.0f,  1.0f,  1.0f),
		});
	mesh.push_quad({
			Vec3<f32>(-1.0f, -1.0f, -1.0f),
			Vec3<f32>(-1.0f, -1.0f,  1.0f),
			Vec3<f32>( 1.0f, -1.0f, -1.0f),
			Vec3<f32>( 1.0f, -1.0f,  1.0f),
		});
	Mesh m;
	m.submeshes.push(std::move(mesh));
	return m;
}