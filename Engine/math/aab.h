#pragma once
#include"mat.h"

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

		bool intersects(const Aab& other) const {
			return this->min.map<bool>([&](const auto& amin, const auto& amax, const auto& bmin, const auto& bmax) {
				return (amin >= bmin && amin < bmax) || (amax > bmin && amax <= bmax) || (bmin >= amin && bmin < amax) || (bmax > amin && bmax <= amax);
			}, this->max, other.min, other.max).reduce_and();
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
	
		Vec<T, L> center() const {
			return (min + max) / (T)2;
		}
		Vec<T, L> size() const {
			return max - min;
		}
		Vec<T, L> extends() const {
			return size() / (T)2;
		}

		T distance_to_sqr(const Vec<T, L>& point) const {
			constexpr T ZERO = (T)0;
			Vec<T, L> d = point.map<T>([&](const T& t, const T& min, const T& max) {
				auto a = min - t;
				auto b = t - max;
				auto v = a > b ? a : b;
				return  v > ZERO ? v : ZERO;
			}, min, max);
		
			return d.length_sqr();
		}
		
		T distance_to(const Vec<T, L>& point) const {
			return std::sqrt(distance_to_sqr(point));
		}

		Aab rotated(const Mat<T, L, L>& mat) const {
			return Aab{
				mat * min,
				mat * max,
			}.valid();
		}

		Aab transformed(const Mat<T, L + 1, L + 1>& mat) const {
			constexpr T ONE = (T)1;
			return Aab{
				Vec<T, L>::from_higher(mat * min.with<L>(ONE)),
				Vec<T, L>::from_higher(mat * max.with<L>(ONE)),
			}.valid();
		}
	};
}

template<typename T>
using Range = math::Aab<T, 1>;

template<typename T>
using Aabb = math::Aab<T, 3>;

template<typename T>
using Aabr = math::Aab<T, 2>;