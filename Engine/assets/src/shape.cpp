#include<assets/shape.h>

Mesh unit_cube() {
	SubMesh mesh;
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, 1.0f, 1.0f),
			Vec3<f32>(1.0f, 0.0f, 0.0f),
			Vec2<f32>(1.0f, 1.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, -1.0f, 1.0f),
			Vec3<f32>(1.0f, 0.0f, 0.0f),
			Vec2<f32>(1.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, -1.0f, -1.0f),
			Vec3<f32>(1.0f, 0.0f, 0.0f),
			Vec2<f32>(0.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, 1.0f, -1.0f),
			Vec3<f32>(1.0f, 0.0f, 0.0f),
			Vec2<f32>(0.0f, 1.0f),
		}));
	mesh.indices.push(std::move(0));
	mesh.indices.push(std::move(1));
	mesh.indices.push(std::move(2));
	mesh.indices.push(std::move(0));
	mesh.indices.push(std::move(2));
	mesh.indices.push(std::move(3));

	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, 1.0f, 1.0f),
			Vec3<f32>(0.0f, 0.0f, 1.0f),
			Vec2<f32>(1.0f, 1.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, -1.0f, 1.0f),
			Vec3<f32>(0.0f, 0.0f, 1.0f),
			Vec2<f32>(1.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, -1.0f, 1.0f),
			Vec3<f32>(0.0f, 0.0f, 1.0f),
			Vec2<f32>(0.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, 1.0f, 1.0f),
			Vec3<f32>(0.0f, 0.0f, 1.0f),
			Vec2<f32>(0.0f, 1.0f),
		}));
	mesh.indices.push(std::move(4));
	mesh.indices.push(std::move(5));
	mesh.indices.push(std::move(6));
	mesh.indices.push(std::move(4));
	mesh.indices.push(std::move(6));
	mesh.indices.push(std::move(7));

	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, 1.0f, -1.0f),
			Vec3<f32>(-1.0f, 0.0f, 0.0f),
			Vec2<f32>(1.0f, 1.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, -1.0f, -1.0f),
			Vec3<f32>(-1.0f, 0.0f, 0.0f),
			Vec2<f32>(1.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, -1.0f, 1.0f),
			Vec3<f32>(-1.0f, 0.0f, 0.0f),
			Vec2<f32>(0.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, 1.0f, 1.0f),
			Vec3<f32>(-1.0f, 0.0f, 0.0f),
			Vec2<f32>(0.0f, 1.0f),
		}));
	mesh.indices.push(std::move(8));
	mesh.indices.push(std::move(9));
	mesh.indices.push(std::move(10));
	mesh.indices.push(std::move(8));
	mesh.indices.push(std::move(10));
	mesh.indices.push(std::move(11));

	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, 1.0f, -1.0f),
			Vec3<f32>(0.0f, 0.0f, -1.0f),
			Vec2<f32>(1.0f, 1.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, -1.0f, -1.0f),
			Vec3<f32>(0.0f, 0.0f, -1.0f),
			Vec2<f32>(1.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, -1.0f, -1.0f),
			Vec3<f32>(0.0f, 0.0f, -1.0f),
			Vec2<f32>(0.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, 1.0f, -1.0f),
			Vec3<f32>(0.0f, 0.0f, -1.0f),
			Vec2<f32>(0.0f, 1.0f),
		}));
	mesh.indices.push(std::move(12));
	mesh.indices.push(std::move(13));
	mesh.indices.push(std::move(14));
	mesh.indices.push(std::move(12));
	mesh.indices.push(std::move(14));
	mesh.indices.push(std::move(15));

	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, 1.0f, 1.0f),
			Vec3<f32>(0.0f, 1.0f, 0.0f),
			Vec2<f32>(1.0f, 1.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, 1.0f, -1.0f),
			Vec3<f32>(0.0f, 1.0f, 0.0f),
			Vec2<f32>(1.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, 1.0f, -1.0f),
			Vec3<f32>(0.0f, 1.0f, 0.0f),
			Vec2<f32>(0.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, 1.0f, 1.0f),
			Vec3<f32>(0.0f, 1.0f, 0.0f),
			Vec2<f32>(0.0f, 1.0f),
		}));
	mesh.indices.push(std::move(16));
	mesh.indices.push(std::move(17));
	mesh.indices.push(std::move(18));
	mesh.indices.push(std::move(16));
	mesh.indices.push(std::move(18));
	mesh.indices.push(std::move(19));

	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, -1.0f, 1.0f),
			Vec3<f32>(0.0f, -1.0f, 0.0f),
			Vec2<f32>(1.0f, 1.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(1.0f, -1.0f, -1.0f),
			Vec3<f32>(0.0f, -1.0f, 0.0f),
			Vec2<f32>(1.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, -1.0f, -1.0f),
			Vec3<f32>(0.0f, -1.0f, 0.0f),
			Vec2<f32>(0.0f, 0.0f),
		}));
	mesh.vertices.push(std::move(Vertex{
			Vec3<f32>(-1.0f, -1.0f, 1.0f),
			Vec3<f32>(0.0f, -1.0f, 0.0f),
			Vec2<f32>(0.0f, 1.0f),
		}));
	mesh.indices.push(std::move(20));
	mesh.indices.push(std::move(21));
	mesh.indices.push(std::move(22));
	mesh.indices.push(std::move(20));
	mesh.indices.push(std::move(22));
	mesh.indices.push(std::move(23));

	Mesh m;
	m.submeshes.push(std::move(mesh));
	return m;
}