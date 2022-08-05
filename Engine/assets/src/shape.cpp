#include<assets/shape.h>

void Mesh::push_quad(std::array<Vec3<f32>, 4> points) {
	Vec3<f32> normal = (points[2] - points[0]).cross(points[1] - points[0]).normalized();
	Vec3<f32> tan = (points[2] - points[0]).normalized();
	Index i = (Index)vertices.len();
	vertices.push(Vertex{
		points[0],
		normal,
		tan,
		Vec3<f32>(0.0f, 0.0f, 0.0f),
	});
	vertices.push(Vertex{
		points[1],
		normal,
		tan,
		Vec3<f32>(1.0f, 0.0f, 0.0f),
		});
	vertices.push(Vertex{
		points[2],
		normal,
		tan,
		Vec3<f32>(0.0f, 1.0f, 0.0f),
		});
	vertices.push(Vertex{
		points[3],
		normal,
		tan,
		Vec3<f32>(1.0f, 1.0f, 0.0f),
	});
	indices.push(i + 0);
	indices.push(i + 2);
	indices.push(i + 1);

	indices.push(i + 1);
	indices.push(i + 2);
	indices.push(i + 3);
}

Mesh unit_cube() {
	Mesh mesh;
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
	Index end = (Index)mesh.indices.len();
	mesh.submeshes.push(SubMesh{
		0,
		end,
		none<AId<Material>>(),
	});
	mesh.calculate_bounds();
	return mesh;
}