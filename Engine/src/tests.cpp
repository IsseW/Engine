#ifdef _DEBUG

#include<rstd/panic.h>
#include<math/consts.h>
#include<math/mat.h>
#include<assets/assets.h>
#include<data_structures/sparse_octree.h>
#include<renderer/world.h>

template<typename F>
bool feq(F a, F b) {
	if (abs(a) < 1.0 || abs(b) < 1.0) {
		return abs(a - b) < 0.0001;
	}
	else {
		return abs(a / b - 1.0) < 0.0001;
	}
}

template<typename F, const usize L>
bool vfeq(math::Vec<F, L> a, math::Vec<F, L> b) {
	return a.map<bool>([](const F& a, const F& b) { return feq(a, b); }, b).reduce_and();
}

template<typename F, const usize W, const usize H>
bool mfeq(math::Mat<F, W, H> a, math::Mat<F, W, H> b) {
	return a.rows().map<bool>([](const math::Vec<F, W>& a, const math::Vec<F, W>& b) { return vfeq(a, b); }, b.rows()).reduce_and();
}

namespace math {

	void test_vec() {
		Vec3<i32> t(1, 2, 3);

		ASSERT_EQ(t.with_w(4), Vec4<i32>(1, 2, 3, 4));
	}

	f64 rand_f64() {
		return (f64)rand() / (f64)RAND_MAX + (f64)rand() + (f64)rand() * (f64)RAND_MAX * (rand() < RAND_MAX / 2 ? -1.0 : 1.0);
	}

	template<const usize L>
	Vec<f64, L> rand_vec() {
		Vec<f64, L> res;
		for (usize i = 0; i < L; ++i) {
			res[i] = rand_f64();
		}
		return res;
	}

	template<const usize W, const usize H>
	Mat<f64, W, H> rand_mat() {
		Mat<f64, W, H> res;
		for (usize y = 0; y < H; ++y) {
			res[y] = rand_vec<W>();
		}
		return res;
	}

	template<typename T, const usize S>
	void test_inversion(const Mat<T, S, S>& mat) {
		ASSERT(mfeq(mat * mat.invert(), Mat<T, S, S>::identity()));
	}

	template<const usize S>
	void test_inversions() {
		for (usize i = 0; i < 1000; ++i) {
			test_inversion(rand_mat<S, S>());
		}
	}

	template<const usize W, const usize H>
	void test_identities() {
		// Test A*(B + C) = A*B + A*C
		for (usize i = 0; i < 1000; ++i) {
			auto a = rand_mat<H, W>();
			auto b = rand_mat<W, H>();
			auto c = rand_mat<W, H>();
			ASSERT(mfeq(a * (b + c), a * b + a * c));
		}
	}

	void test_matrix() {
		auto mat = Mat3<f64>(3.0, 2.0, 1.0, 1.0, 2.0, 3.0, 3.0, 1.0, 2.0);
		test_inversion(mat);
		auto euler = Vec3<f64>{ 45.0, 45.0, 0.0 } *F64::TO_RAD;
		auto quat = Quat<f64>::from_euler(euler);
		auto rot_mat = mat_from_euler(euler);
		test_inversion(rot_mat);
		auto test_vec = Vec3<f64>{ 4.0, 2.0, 0.0 };

		ASSERT(vfeq(quat * test_vec, rot_mat * test_vec));

		auto trans = transformation(Vec3<f64>::zero(), Vec3<f64>::one(), Vec3<f64>::zero());
		ASSERT(mfeq(trans, Mat4<f64>::identity()));

		trans = transformation(Vec3<f64>(1.0, 2.0, 3.0), Vec3<f64>::one(), Vec3<f64>::zero());
		ASSERT(vfeq((trans * Vec4<f64>(0.0, 0.0, 0.0, 1.0)).xyz(), Vec3<f64>(1.0, 2.0, 3.0)));
		ASSERT(vfeq((trans.invert() * Vec4<f64>(1.0, 2.0, 3.0, 1.0)).xyz(), Vec3<f64>::zero()));

		srand((u32)time(0));
		test_identities<3, 3>();
		test_identities<4, 4>();
		test_identities<3, 4>();
		test_identities<4, 3>();

		test_inversions<2>();
		test_inversions<3>();
		test_inversions<4>();
	}

	void test_quat() {
		for (usize i = 0; i < 1000; ++i) {
			auto euler = rand_vec<3>().map<f64>([](f64 e) { return fmod(e, 90.0); });
			auto quat = Quat<f64>::from_euler(euler * F64::TO_RAD);
			
			ASSERT(vfeq(euler, quat.to_euler() * F64::TO_DEG));
		}
	}

	void test() {
		test_vec();
		test_matrix();
		test_quat();
	}
}

namespace data_structures {
	void test_sparse_octree_vec() {
		SparseOctree<u32, 8> octree{ Vec3<f32>::zero(), 500};
		auto insert1_pos = Vec3<f32>::one() / 2;
		auto insert1 = octree.insert(insert1_pos, 1);
		ASSERT(insert1.is_ok());
		auto insert2_pos = Vec3<f32>{ 200, 300, 500 };
		auto insert2 = octree.insert(insert2_pos, 2);
		ASSERT(insert2.is_ok());
		auto insert3_pos = Vec3<f32>{ 20, 10, 5 };
		auto insert3 = octree.insert(insert3_pos, 3);
		ASSERT(insert3.is_ok());
		auto insert4_pos = insert3_pos;
		auto insert4 = octree.insert(insert3_pos, 4);
		ASSERT(insert4.is_ok());
		ASSERT(octree.at(insert1_pos) != nullptr);
		ASSERT(*octree.at(insert1_pos)->get(0).unwrap() == 1);
		ASSERT(octree.at(insert2_pos) != nullptr);
		ASSERT(*octree.at(insert2_pos)->get(0).unwrap() == 2);
		ASSERT(octree.at(insert3_pos) != nullptr);
		ASSERT(*octree.at(insert3_pos)->get(0).unwrap() == 3);
		ASSERT(octree.at(insert4_pos) != nullptr);
		ASSERT(*octree.at(insert4_pos)->get(1).unwrap() == 4);
		ASSERT(octree.at(insert1_pos + insert2_pos + insert3_pos) == nullptr);
	}

	void test_sparse_octree_aabb() {
		SparseOctree<u32, 5> octree{ Vec3<f32>::zero(), 2 };
		auto insert_min = Vec3<f32>{ 0,0,0 };
		auto insert_max = Vec3<f32>{ 1,1,1 };
		auto insert_aabb = Aabb<f32>{ insert_min, insert_max };
		auto insert = octree.insert(insert_aabb, 1);
		ASSERT(insert.is_ok());
		ASSERT(octree.at(insert_min) != nullptr);
		ASSERT(*octree.at(insert_min)->get(0).unwrap() == 1);
		ASSERT(octree.at(insert_max) != nullptr);
		ASSERT(*octree.at(insert_max)->get(0).unwrap() == 1);
		ASSERT(octree.at(insert_min - 0.40f) == nullptr);
		ASSERT(octree.at(insert_max + 0.40f) == nullptr);
		ASSERT(octree.at(insert_min + 0.40f) != nullptr);
		ASSERT(octree.at(insert_max - 0.40f) != nullptr);
		ASSERT(*octree.at(insert_min + 0.40f)->get(0).unwrap() == 1);
		ASSERT(*octree.at(insert_max - 0.40f)->get(0).unwrap() == 1);
		ASSERT(octree.at(insert_min + (insert_max - insert_min ) / 2.0f) != nullptr);
	}

	void test() {
		test_sparse_octree_vec();
		test_sparse_octree_aabb();
	}
}

namespace files {
	void test_obj_load() {
		AssetHandler handler{};
		auto mesh = handler.load<Mesh>(std::filesystem::path{ "resources/cube.wavefront" });
		handler.clean_up();
	}

	void test() {
		test_obj_load();
	}
}

namespace world {

	void test_transform() {
		Transform trans = Transform::from_translation(Vec3<f32>(5.0f, 0.0f, 0.0f)).looking_at(Vec3<f32>::zero());
		auto mat = trans.get_mat();
		ASSERT(vfeq((mat * Vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f)).xyz(), Vec3<f32>(5.0f, 0.0f, 0.0f)));
		auto inv = mat.invert();
		ASSERT(vfeq((inv * Vec4<f32>(5.0f, 0.0f, 0.0f, 1.0f)).xyz(), Vec3<f32>::zero()));

		trans = Transform::from_translation(Vec3<f32>(5.0f, 5.0f, 5.0f)).looking_at(Vec3<f32>::zero());
		mat = trans.get_mat();
		ASSERT(vfeq((mat * Vec4<f32>::unit_z()).xyz(), -Vec3<f32>::one().normalized()));
	}

	void test() {
		test_transform();
	}
}


void run_tests() {
	math::test();
	files::test();
	world::test();
	data_structures::test();
	std::cout << "All tests passed." << std::endl;
}

#endif