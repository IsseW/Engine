#pragma once

#include"vec.h"
namespace math {
	template<typename T>
	struct Quat {
		constexpr Quat() : _v() {}
		constexpr Quat(const T& v) : _v(v) {}
		constexpr Quat(const Vec4<T>& v) : _v(v) {}
		constexpr Quat(T x, T y, T z, T w) : _v(x, y, z, w) {}

		constexpr static Quat<f64> from_euler(Vec3<f64> euler) {
			euler /= 2.0;
			auto c = euler.map<f64>([](f64 e) { return cos(e); });
			auto s = euler.map<f64>([](f64 e) { return sin(e); });

			return  Quat{
				s.z * c.y * c.x - c.z * s.y * s.x,
				c.z * s.y * c.x + s.z * c.y * s.x,
				c.z * c.y * s.x - s.z * s.y * c.x,
				c.z * c.y * c.x + s.z * s.y * s.x,
			};
		}

		constexpr void normalize() {
			_v.normalize();
		}

		constexpr Quat normalized() const {
			return Quat{ _v.normalized() };
		}

		constexpr Quat conjugate() const {
			return Quat{ -_v.x, -_v.y, -_v.z, _v.w };
		}

		Quat operator+(const Quat& other) const {
			return Quat(std::move(_v + other._v));
		}
		Quat& operator+=(const Quat& other) {
			_v += other._v;
			return *this;
		}

		Quat operator-(const Quat& other) const {
			return Quat(std::move(_v - other._v));
		}
		Quat& operator-=(const Quat& other) {
			_v -= other._v;
			return *this;
		}

		Quat operator*(const T& other) const {
			return Quat(std::move(_v * other));
		}
		Quat operator/(const T& other) const {
			return Quat(std::move(_v / other));
		}
		Quat operator*(const Quat& other) const {
			const Vec4<T>& q1 = _v;
			const Vec4<T>& q2 = other._v;
			return Quat{
				 q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x,
				-q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y,
				 q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z,
				-q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w
			};
		}
		Vec3<T> operator*(const Vec3<T>& v) const {
			Quat p{ v.xyz_() };
			Quat c = this->operator*(p) * conjugate();
			return c._v.xyz();
		}
		const Vec4<T> inner() const {
			return _v;
		}
	private:
		Vec4<T> _v;
	};

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const Quat<T>& quat) {
		return os << quat.inner();
	}

}
template<typename T>
using Quat = math::Quat<T>;