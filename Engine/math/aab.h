#pragma once
#include"vec.h"

namespace math {

	template<typename T, const usize L>
	struct Aab {
		Vec<T, L> min;
		Vec<T, L> max;

		void validate() {
			for (usize i = 0; i < L; ++i) {
				if (min[i] > max[i]) {
					std::swap(min[i], max[i]);
				}
			}
		}

		Aab valid() const {
			Aab aab = *this;
			aab.validate();
			return aab;
		}

		bool contains(const Vec<T, L>& point) const {
			return point.map<bool>([&](const T& e, const T& min, const T& max) {
				return min <= e && e <= max;
			}, min, max).reduce_and();
		}

		void grow_to_contain(const Vec<T, L>& point) {
			for (usize i = 0; i < L; ++i) {
				if (min[i] > point[i]) {
					min[i] = point[i];
				}
				if (max[i] < point[i]) {
					max[i] = point[i];
				}
			}
		}
	};
}

template<typename T>
using Range = math::Aab<T, 1>;

template<typename T>
using Aabb = math::Aab<T, 3>;

template<typename T>
using Aabr = math::Aab<T, 2>;