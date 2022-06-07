#ifdef _DEBUG

#include<rstd/panic.h>
#include<math/consts.h>
#include<math/mat.h>

namespace math {
	bool feq(f64 a, f64 b) {
		return abs(a - b) < 0.001;
	}

	template<const usize L>
	bool vfeq(Vec<f64, L> a, Vec<f64, L> b) {
		return a.map<bool>([](const f64& a, const f64& b) { return feq(a, b); }, b).reduce_and();
	}

	template<typename T, const usize S>
	void test_inversion(const Mat<T, S, S>& mat) {
		auto id = mat * mat.invert();
		for (usize i = 0; i < S; ++i) {
			Vec<T, S> v{};
			v[i] = (T)1;
			ASSERT(vfeq(id.row(i), v));
		}
	}

	void test_matrix() {
		test_inversion(Mat3<f64>{ 3.0, 2.0, 1.0, 1.0, 2.0, 3.0, 3.0, 1.0, 2.0 });
		auto euler = Vec3<f64>{ 45.0, 45.0, 0.0 } *F64::TO_RAD;
		auto quat = Quat<f64>::from_euler(euler);
		auto rot_mat = mat_from_euler(euler);
		test_inversion(rot_mat);
		auto test_vec = Vec3<f64>{ 4.0, 2.0, 0.0 };

		ASSERT(vfeq(quat * test_vec, rot_mat * test_vec));
	}

	void test() {
		test_matrix();
	}
}


void run_tests() {
	math::test();
	std::cout << "All tests passed." << std::endl;
}

#endif