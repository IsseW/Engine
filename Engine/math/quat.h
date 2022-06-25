#pragma once

#include"vec.h"
namespace math {
	template<typename T>
	struct Quat {
		constexpr Quat() : _v(Vec<T, 4>::unit_w()) {}
		constexpr Quat(const T& v) : _v(v) {}
		constexpr Quat(const Vec4<T>& v) : _v(v) {}
		constexpr Quat(T x, T y, T z, T w) : _v(x, y, z, w) {}

		constexpr static Quat from_euler(Vec3<T> euler) {
			euler /= (T)2;
			auto c = euler.map<T>([](T e) { return std::cos(e); });
			auto s = euler.map<T>([](T e) { return std::sin(e); });

			return  Quat{
				s.z * c.y * c.x - c.z * s.y * s.x,
				c.z * s.y * c.x + s.z * c.y * s.x,
				c.z * c.y * s.x - s.z * s.y * c.x,
				c.z * c.y * c.x + s.z * s.y * s.x,
			};
		}

		constexpr static Quat<T> angle_axis(Vec3<T> axis, T angle) {
			T s = std::sin(angle / (T)2);
			Vec3<T> u = axis.normalized();
			return Quat(u.x * s, u.y * s, u.z * s, std::cos(angle / (T)2));
		}

		constexpr static Quat<T> looking_dir(Vec3<T> dir, Vec3<T> forward, Vec3<T> up) {
			Vec3<T> rot_axis = forward.cross(dir).normalized();
			if (rot_axis.length_sqr() == 0.0f) {
				rot_axis = up;
			}
			T dot = forward.dot(dir);
			T angle = std::acos(dot);
			return angle_axis(rot_axis, angle);
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