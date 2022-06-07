#pragma once
#include<rstd/option.h>
#include"quat.h"

namespace math {
	template<typename T, const usize W, const usize H>
	struct Mat {
		Mat() : _rows() {}
		Mat(const Vec<Vec<T, W>, H>& rows) : _rows(rows) {}
		template<typename... Args>
		constexpr Mat(Args... args) {
			std::array arr = std::array<T, W* H> { args... };
			_rows = *reinterpret_cast<Vec<Vec<T, W>, H>*>(&arr);
		}

		const Vec<Vec<T, H>, W>& rows() const {
			return _rows;
		}

		constexpr Vec<T, W> row(const usize& y) const {
			return _rows[y];
		}
		constexpr Vec<T, W> col(const usize& x) const {
			Vec<T, W> col{};
			for (usize y = 0; y < W; ++y) {
				col[y] = _rows[y][x];
			}
			return col;
		}

		Vec<T, H>& operator[](usize i) {
			return _rows[i];
		}
		const Vec<T, H>& operator[](usize i) const {
			return _rows[i];
		}

		Mat operator+(const T& other) const {
			return Mat(_rows + other);
		}
		Mat operator+(const Mat& other) const {
			return Mat(_rows + other._rows);
		}
		Mat& operator+=(const T& other) {
			_rows += other;
			return *this;
		}
		Mat& operator+=(const Mat& other) {
			_rows += other._rows;
			return *this;
		}

		Mat operator-(const T& other) const {
			return Mat(_rows + other);
		}
		Mat operator-(const Mat& other) const {
			return Mat(_rows + other._rows);
		}
		Mat& operator-=(const T& other) {
			_rows -= other;
			return *this;
		}
		Mat& operator-=(const Mat& other) {
			_rows -= other._rows;
			return *this;
		}


		Mat operator*(const T& other) const {
			return Mat(_rows * other);
		}
		Mat& operator*=(const T& other) {
			_rows *= other;
			return *this;
		}

		Mat operator/(const T& other) const {
			return Mat(_rows / other);
		}
		Mat& operator/=(const T& other) {
			_rows /= other;
			return *this;
		}

		template<const usize OW>
		Mat<T, OW, H> operator*(const Mat<T, OW, W>& other) const {
			Mat<T, OW, H> res;
			for (usize y = 0; y < H; ++y) {
				for (usize x = 0; x < OW; ++x) {
					res[y][x] = row(y).dot(other.col(x));
				}
			}
			return res;
		}
		Mat<T, W, H> operator/(const Mat<T, W, H>& other) const {
			return this->operator*(other.invert());
		}

		Mat& operator*=(const Mat<T, W, H>& other) {
			return *this = this->operator*(other);
		}
		Mat& operator/=(const Mat<T, W, H>& other) {
			return *this = this->operator*(other.invert());
		}
		Vec<T, H> operator*(const Vec<T, H>& other) const {
			Vec<T, H> res;
			for (usize y = 0; y < H; ++y) {
				res[y] = row(y).dot(other);
			}
			return res;
		}

		constexpr Mat<T, H, W> transposed() const {
			Mat<T, H, W> m;
			for (usize y = 0; y < H; ++y) {
				m[y] = col(y);
			}
			return m;
		}

		constexpr T determinant() const requires (W == 1 && H == 1) {
			return _rows.x.x;
		}
		constexpr T determinant() const requires (W == 2 && H == 2) {
			return _rows.x.x * _rows.y.y - _rows.x.y * _rows.y.x;
		}
		constexpr T determinant() const requires (W == H && W > 2) {
			T res{};
			bool sign = false;
			constexpr usize S = W - 1;
			for (usize x = 0; x < W; ++x) {
				T det = _rows[0][x] * minor(x, 0).determinant();
				if (sign) {
					res -= det;
				}
				else {
					res += det;
				}
				sign = !sign;
			}
			return res;
		}

		constexpr Mat<T, W - 1, H - 1> minor(const usize x, const usize y) const requires (W > 1 && H > 1) {
			Mat<T, W - 1, H - 1> m;
			usize m_x = 0, m_y = 0;
			for (usize y_ = 0; y_ < H; ++y_) {
				m_x = 0;
				if (y_ != y) {
					for (usize x_ = 0; x_ < W; ++x_) {
						if (x_ != x) {
							m[m_y][m_x] = _rows[y_][x_];
							m_x += 1;
						}
					}
					m_y += 1;
				}
			}
			return m;
		}

		constexpr Mat<T, W, H> cofactor() const {
			Mat<T, W, H> res;
			for (usize y = 0; y < H; ++y) {
				for (usize x = 0; x < W; ++x) {
					auto md = minor(x, y).determinant();
					if ((y + x) % 2 == 0) {
						res[y][x] = md;
					}
					else {
						res[y][x] = -md;
					}
				}
			}
			return res;
		}

		constexpr Mat<T, W, H> adjugate() const {
			return cofactor().transposed();
		}

		constexpr Mat<T, W, H> invert() const {
			auto det = determinant();
			if (det == T{}) {
				PANIC("Tried to invert a matrix with a determinant of zero");
			}
			return adjugate() / det;
		}

		constexpr Option<Mat<T, W, H>> try_invert() const {
			auto det = determinant();
			if (det == T{}) {
				return none<Mat<T, W, H>>();
			}
			else {
				return some(adjugate() / det);
			}
		}
	private:
		Vec<Vec<T, W>, H> _rows;
	};

	template<typename T, const usize W, const usize H>
	std::ostream& operator<<(std::ostream& os, const Mat<T, W, H>& mat) {
		return os << mat.rows();
	}
}
template<typename T>
using Mat2 = math::Mat<T, 2, 2>;
template<typename T>
using Mat3 = math::Mat<T, 3, 3>;
template<typename T>
using Mat4 = math::Mat<T, 4, 4>;

namespace math {
	template<typename T>
	constexpr Mat3<T> mat_from_quat(const Quat<T>& q) {
		constexpr T ONE = (T)1;
		constexpr T TWO = (T)2;
		auto v = q.inner();
		return Mat3<T> {
			TWO * (v.w * v.w + v.x * v.x) - ONE,
			TWO * (v.x * v.y - v.w * v.z),
			TWO * (v.x * v.z + v.w * v.y),
			TWO * (v.x * v.y + v.w * v.z),
			TWO * (v.w * v.w + v.y * v.y) - ONE,
			TWO * (v.y * v.z - v.w * v.x),
			TWO * (v.x * v.z - v.w * v.y),
			TWO * (v.y * v.z + v.w * v.x),
			TWO * (v.w * v.w + v.z * v.z) - ONE
		};
	}

	constexpr Mat3<f64> mat_from_euler(const Vec3<f64>& euler) {
		return mat_from_quat(Quat<f64>::from_euler(euler));
	}

	template<typename T>
	constexpr Mat4<T> transformation(const Vec3<T>& t, const Vec3<T>& s, const Mat3<T>& r) {
		constexpr T ZERO = T{};
		constexpr T ONE = const_cast<T>(1);
		constexpr T TWO = const_cast<T>(2);
		return Mat3<T> {
			s.x * r[0][0], s.y * r[0][1], s.z * r[0][2], t.x,
			s.x * r[1][0], s.y * r[1][1], s.z * r[1][2], t.x,
			s.x * r[2][0], s.y * r[2][1], s.z * r[2][2], t.x,
			ZERO, ZERO, ZERO, ONE
		};
	}

	template<typename T>
	constexpr Mat4<T> transformation(const Vec3<T>& t, const Vec3<T>& s, const Quat<T>& quat) {
		auto r = mat_from_quat(quat);
		return transformation(t, s, r);
	}

	template<typename T>
	constexpr Mat4<T> transformation(const Vec3<T>& t, const Vec3<T>& s, const Vec3<T>& euler) {
		auto r = mat_from_euler(euler);
		return transformation(t, s, r);
	}
}