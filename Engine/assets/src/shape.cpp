#include<assets/shape.h>

void SubMesh::push_quad(std::array<Vec3<f32>, 4> points) {
	Vec3<f32> normal = (points[1] - points[0]).cross(points[2] - points[0]);
	u32 i = vertices.len();
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
	indices.push(i + 1);
	indices.push(i + 2);
	indices.push(i + 0);
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
	/*
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
	*/
	Mesh m;
	m.submeshes.push(std::move(mesh));
	return m;
}