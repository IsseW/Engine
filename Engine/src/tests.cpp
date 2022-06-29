#ifdef _DEBUG

#include<rstd/panic.h>
#include<math/consts.h>
#include<math/mat.h>
#include<assets/assets.h>

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
	template<const usize W, const usize H>
	Mat<f64, W, H> rand_mat() {
		Mat<f64, W, H> res;
		for (usize y = 0; y < H; ++y) {
			for (usize x = 0; x < W; ++x) {
				res[y][x] = rand_f64();
			}
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

	void test() {
		test_vec();
		test_matrix();
	}
}

namespace files {
	void test_obj_load() {
		auto mesh = Mesh::load("resources/test.wavefront");
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
	std::cout << "All tests passed." << std::endl;
}

#endif