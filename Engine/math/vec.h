#pragma once
#include<rstd/primitives.h>
#include<array>
#include<iostream>
#include<concepts>

namespace math {
#define BINARY_OPERATOR(op)					\
template<typename U>						\
Vec operator op(const U& b) const {			\
	Vec c;									\
	for (usize i = 0; i < L; ++i) {			\
		c[i] = _elems[i] op b;				\
	}										\
	return c;								\
}											\
Vec operator op(const Vec& b) const{		\
	Vec c;									\
	for (usize i = 0; i < L; ++i) {			\
		c[i] = _elems[i] op b[i];			\
	}										\
	return c;								\
}											\
Vec& operator op ## =(const Vec& other) {	\
	for (usize i = 0; i < L; ++i) {			\
		_elems[i] op ## = other[i];			\
	}										\
	return *this;							\
}											\
template<typename U>						\
Vec& operator op ## =(const U& other) {		\
	for (usize i = 0; i < L; ++i) {			\
		_elems[i] op ## = other;			\
	}										\
	return *this;							\
}
#define UNARY_OPERATOR(op)			\
Vec operator op() const {			\
	Vec c;							\
	for (usize i = 0; i < L; ++i) {	\
		c[i] = op _elems[i];		\
	}								\
	return c;						\
}
#define SWIZZLE2(a, b) constexpr Vec<T, 2> a ## b() const { return Vec<T, 2> { a, b }; }
#define SWIZZLE3(a, b, c) constexpr Vec<T, 3> a ## b ## c() const { return Vec<T, 3> { a, b, c }; }
#define SWIZZLE4(a, b, c, d) constexpr Vec<T, 4> a ## b ## c ## d() const { return Vec<T, 4> { a, b, c, d }; }

#define PROPERTY(n, i) \
	__declspec(property( put = set_##n, get = get_##n)) T n;\
	constexpr const T& get_##n() const requires (i < L) { return _elems[i]; } \
	void set_##n(const T& e) requires (i < L) { _elems[i] = e; }
	
	template<typename T, const usize L>
	struct Vec {

		constexpr Vec() : _elems() {}
		constexpr Vec(const T& splat) {
			for (usize i = 0; i < L; ++i) {
				_elems[i] = splat;
			}
		}
		constexpr Vec(const std::array<T, L>& elems) : _elems(elems) { }
		constexpr Vec(const Vec& other) : _elems(other._elems) {}

		template<typename... Args>
		constexpr Vec(Args... args) : _elems({T(args)...}) { }

		template<const usize OL>
		static constexpr Vec from_higher(Vec<T, OL> higher) requires (OL > L) {
			Vec res;
			for (usize i = 0; i < L; ++i) {
				res[i] = higher[i];
			}
			return res;
		}

		Vec& operator=(const Vec& other) {
			_elems = other._elems;
			return *this;
		}

		T& operator[](const usize& i) {
			return _elems[i];
		}
		const T& operator[](const usize& i) const {
			return _elems[i];
		}

		bool operator==(const Vec& other) const {
			for (usize i = 0; i < L; ++i) {
				if (_elems[i] != other._elems[i]) {
					return false;
				}
			}
			return true;
		}

		bool operator!=(const Vec& other) const {
			return !this->operator==(other);
		}

		BINARY_OPERATOR(+);
		BINARY_OPERATOR(-);
		BINARY_OPERATOR(*);
		BINARY_OPERATOR(/);
		BINARY_OPERATOR(%);
		BINARY_OPERATOR(&);
		BINARY_OPERATOR(|);
		BINARY_OPERATOR(^);

		UNARY_OPERATOR(-);
		UNARY_OPERATOR(~);
		UNARY_OPERATOR(!);

		template<typename U>
		constexpr Vec<U, L> as() const {
			Vec<U, L> c;
			for (usize i = 0; i < L; ++i) {
				c[i] = (U)_elems[i];
			}
			return c;
		}

		template<typename U, typename F>
		constexpr Vec<U, L> map(F map) const {
			Vec<U, L> c;
			for (usize i = 0; i < L; ++i) {
				c[i] = map(_elems[i]);
			}
			return std::move(c);
		}
		template<typename U, typename F, typename... Args>
		constexpr Vec<U, L> map(F map, Vec<Args, L>... args) const {
			Vec<U, L> c;
			for (usize i = 0; i < L; ++i) {
				c[i] = map(_elems[i], args._elems[i]...);
			}
			return std::move(c);
		}

		constexpr T dot(const Vec<T, L>& other) const {
			T t {};
			for (usize i = 0; i < L; ++i) {
				t += _elems[i] * other[i];
			}
			return t;
		}
		constexpr T length_sqr() const {
			return dot(*this);
		}
		constexpr T length() const {
			return std::sqrt(length_sqr());
		}
		constexpr void normalize() {
			this->operator/=(length());
		}
		constexpr Vec normalized() const {
			return this->operator/(length());
		}

		constexpr Vec<T, 3> cross(const Vec<T, 3>& o) const requires (L == 3) {
			return Vec<T, 3> { crs(o, 1, 2), crs(o, 2, 0), crs(o, 0, 1) };
		}
		constexpr Vec<T, 7> cross(const Vec<T, 7>& o) const requires(L == 7) {
			return Vec<T, 7> {  
				crs(o, 1, 3) + crs(o, 2, 6) + crs(o, 4, 5),
				crs(o, 2, 4) + crs(o, 3, 0) + crs(o, 5, 6),
				crs(o, 3, 5) + crs(o, 4, 1) + crs(o, 6, 0),
				crs(o, 4, 6) + crs(o, 5, 2) + crs(o, 0, 1),
				crs(o, 5, 0) + crs(o, 6, 3) + crs(o, 1, 2),
				crs(o, 6, 1) + crs(o, 0, 4) + crs(o, 2, 3),
				crs(o, 0, 2) + crs(o, 1, 5) + crs(o, 3, 4),
			};
		}

		template<typename F>
		constexpr T reduce(F f) const {
			T acc = _elems[0];
			for (usize i = 1; i < L; ++i) {
				acc = f(acc, _elems[1]);
			}
			return acc;
		}

		constexpr T reduce_and() const {
			return reduce([](const T& a, const T& b) { return a && b; });
		}
		constexpr T reduce_or() const {
			return reduce([](const T& a, const T& b) { return a || b; });
		}
		constexpr T sum() const {
			return reduce([](const T& a, const T& b) { return a + b; });
		}

		PROPERTY(x, 0);
		PROPERTY(y, 1);
		PROPERTY(z, 2);
		PROPERTY(w, 3);
		
		template<const usize E>
		Vec with(const T& e) const requires (E < L) {
			Vec res = *this;
			res._elems[E] = e;
			return res;
		}
		template<const usize E>
		Vec<T, L + 1> with(const T& e) const requires (E == L) {
			Vec<T, L + 1> res;
			std::copy(_elems.begin(), _elems.end(), res.begin());
			res.back() = e;
			return res;
		}

		constexpr auto with_x(const T& e) const {
			return with<0>(e);
		}
		constexpr auto with_y(const T& e) const {
			return with<1>(e);
		}
		constexpr auto with_z(const T& e) const {
			return with<2>(e);
		}
		constexpr auto with_w(const T& e) const {
			return with<3>(e);
		}

		template<const usize E>
		constexpr static Vec<T, L> unit() requires (E < L) { return Vec<T, L>().with<E>((T)1); }

		constexpr static Vec<T, L> unit_x() { return unit<0>(); }
		constexpr static Vec<T, L> unit_y() { return unit<1>(); }
		constexpr static Vec<T, L> unit_z() { return unit<2>(); }
		constexpr static Vec<T, L> unit_w() { return unit<3>(); }
		constexpr static Vec<T, L> zero() { return Vec<T, L>(); }
		constexpr static Vec<T, L> one() {
			Vec<T, L> res;
			for (usize i = 0; i < L; ++i) {
				res[i] = T{ 1 };
			}
			return res;
		}

		auto begin() {
			return _elems.begin();
		}
		auto begin() const {
			return _elems.begin();
		}

		auto end() {
			return _elems.begin();
		}
		auto end() const {
			return _elems.begin();
		}

		T* data() {
			return _elems.data();
		}
		const T* data() const {
			return _elems.data();
		}

		T& back() {
			return _elems.back();
		}
		const T& back() const {
			return _elems.back();
		}

		SWIZZLE2(x, x);
		SWIZZLE2(x, y);
		SWIZZLE2(x, z);
		SWIZZLE2(x, w);
		SWIZZLE2(x, _);
		SWIZZLE2(y, x);
		SWIZZLE2(y, y);
		SWIZZLE2(y, z);
		SWIZZLE2(y, w);
		SWIZZLE2(y, _);
		SWIZZLE2(z, x);
		SWIZZLE2(z, y);
		SWIZZLE2(z, z);
		SWIZZLE2(z, w);
		SWIZZLE2(z, _);
		SWIZZLE2(w, x);
		SWIZZLE2(w, y);
		SWIZZLE2(w, z);
		SWIZZLE2(w, w);
		SWIZZLE2(w, _);
		SWIZZLE2(_, x);
		SWIZZLE2(_, y);
		SWIZZLE2(_, z);
		SWIZZLE2(_, w);

		SWIZZLE3(x, x, x);
		SWIZZLE3(x, x, y);
		SWIZZLE3(x, x, z);
		SWIZZLE3(x, x, w);
		SWIZZLE3(x, x, _);
		SWIZZLE3(x, y, x);
		SWIZZLE3(x, y, y);
		SWIZZLE3(x, y, z);
		SWIZZLE3(x, y, w);
		SWIZZLE3(x, y, _);
		SWIZZLE3(x, z, x);
		SWIZZLE3(x, z, y);
		SWIZZLE3(x, z, z);
		SWIZZLE3(x, z, w);
		SWIZZLE3(x, z, _);
		SWIZZLE3(x, w, x);
		SWIZZLE3(x, w, y);
		SWIZZLE3(x, w, z);
		SWIZZLE3(x, w, w);
		SWIZZLE3(x, w, _);
		SWIZZLE3(x, _, x);
		SWIZZLE3(x, _, y);
		SWIZZLE3(x, _, z);
		SWIZZLE3(x, _, w);
		SWIZZLE3(x, _, _);
		SWIZZLE3(y, x, x);
		SWIZZLE3(y, x, y);
		SWIZZLE3(y, x, z);
		SWIZZLE3(y, x, w);
		SWIZZLE3(y, x, _);
		SWIZZLE3(y, y, x);
		SWIZZLE3(y, y, y);
		SWIZZLE3(y, y, z);
		SWIZZLE3(y, y, w);
		SWIZZLE3(y, y, _);
		SWIZZLE3(y, z, x);
		SWIZZLE3(y, z, y);
		SWIZZLE3(y, z, z);
		SWIZZLE3(y, z, w);
		SWIZZLE3(y, z, _);
		SWIZZLE3(y, w, x);
		SWIZZLE3(y, w, y);
		SWIZZLE3(y, w, z);
		SWIZZLE3(y, w, w);
		SWIZZLE3(y, w, _);
		SWIZZLE3(y, _, x);
		SWIZZLE3(y, _, y);
		SWIZZLE3(y, _, z);
		SWIZZLE3(y, _, w);
		SWIZZLE3(y, _, _);
		SWIZZLE3(z, x, x);
		SWIZZLE3(z, x, y);
		SWIZZLE3(z, x, z);
		SWIZZLE3(z, x, w);
		SWIZZLE3(z, x, _);
		SWIZZLE3(z, y, x);
		SWIZZLE3(z, y, y);
		SWIZZLE3(z, y, z);
		SWIZZLE3(z, y, w);
		SWIZZLE3(z, y, _);
		SWIZZLE3(z, z, x);
		SWIZZLE3(z, z, y);
		SWIZZLE3(z, z, z);
		SWIZZLE3(z, z, w);
		SWIZZLE3(z, z, _);
		SWIZZLE3(z, w, x);
		SWIZZLE3(z, w, y);
		SWIZZLE3(z, w, z);
		SWIZZLE3(z, w, w);
		SWIZZLE3(z, w, _);
		SWIZZLE3(z, _, x);
		SWIZZLE3(z, _, y);
		SWIZZLE3(z, _, z);
		SWIZZLE3(z, _, w);
		SWIZZLE3(z, _, _);
		SWIZZLE3(w, x, x);
		SWIZZLE3(w, x, y);
		SWIZZLE3(w, x, z);
		SWIZZLE3(w, x, w);
		SWIZZLE3(w, x, _);
		SWIZZLE3(w, y, x);
		SWIZZLE3(w, y, y);
		SWIZZLE3(w, y, z);
		SWIZZLE3(w, y, w);
		SWIZZLE3(w, y, _);
		SWIZZLE3(w, z, x);
		SWIZZLE3(w, z, y);
		SWIZZLE3(w, z, z);
		SWIZZLE3(w, z, w);
		SWIZZLE3(w, z, _);
		SWIZZLE3(w, w, x);
		SWIZZLE3(w, w, y);
		SWIZZLE3(w, w, z);
		SWIZZLE3(w, w, w);
		SWIZZLE3(w, w, _);
		SWIZZLE3(w, _, x);
		SWIZZLE3(w, _, y);
		SWIZZLE3(w, _, z);
		SWIZZLE3(w, _, w);
		SWIZZLE3(w, _, _);
		SWIZZLE3(_, x, x);
		SWIZZLE3(_, x, y);
		SWIZZLE3(_, x, z);
		SWIZZLE3(_, x, w);
		SWIZZLE3(_, x, _);
		SWIZZLE3(_, y, x);
		SWIZZLE3(_, y, y);
		SWIZZLE3(_, y, z);
		SWIZZLE3(_, y, w);
		SWIZZLE3(_, y, _);
		SWIZZLE3(_, z, x);
		SWIZZLE3(_, z, y);
		SWIZZLE3(_, z, z);
		SWIZZLE3(_, z, w);
		SWIZZLE3(_, z, _);
		SWIZZLE3(_, w, x);
		SWIZZLE3(_, w, y);
		SWIZZLE3(_, w, z);
		SWIZZLE3(_, w, w);
		SWIZZLE3(_, w, _);
		SWIZZLE3(_, _, x);
		SWIZZLE3(_, _, y);
		SWIZZLE3(_, _, z);
		SWIZZLE3(_, _, w);

		SWIZZLE4(x, x, x, x);
		SWIZZLE4(x, x, x, y);
		SWIZZLE4(x, x, x, z);
		SWIZZLE4(x, x, x, w);
		SWIZZLE4(x, x, x, _);
		SWIZZLE4(x, x, y, x);
		SWIZZLE4(x, x, y, y);
		SWIZZLE4(x, x, y, z);
		SWIZZLE4(x, x, y, w);
		SWIZZLE4(x, x, y, _);
		SWIZZLE4(x, x, z, x);
		SWIZZLE4(x, x, z, y);
		SWIZZLE4(x, x, z, z);
		SWIZZLE4(x, x, z, w);
		SWIZZLE4(x, x, z, _);
		SWIZZLE4(x, x, w, x);
		SWIZZLE4(x, x, w, y);
		SWIZZLE4(x, x, w, z);
		SWIZZLE4(x, x, w, w);
		SWIZZLE4(x, x, w, _);
		SWIZZLE4(x, x, _, x);
		SWIZZLE4(x, x, _, y);
		SWIZZLE4(x, x, _, z);
		SWIZZLE4(x, x, _, w);
		SWIZZLE4(x, x, _, _);
		SWIZZLE4(x, y, x, x);
		SWIZZLE4(x, y, x, y);
		SWIZZLE4(x, y, x, z);
		SWIZZLE4(x, y, x, w);
		SWIZZLE4(x, y, x, _);
		SWIZZLE4(x, y, y, x);
		SWIZZLE4(x, y, y, y);
		SWIZZLE4(x, y, y, z);
		SWIZZLE4(x, y, y, w);
		SWIZZLE4(x, y, y, _);
		SWIZZLE4(x, y, z, x);
		SWIZZLE4(x, y, z, y);
		SWIZZLE4(x, y, z, z);
		SWIZZLE4(x, y, z, w);
		SWIZZLE4(x, y, z, _);
		SWIZZLE4(x, y, w, x);
		SWIZZLE4(x, y, w, y);
		SWIZZLE4(x, y, w, z);
		SWIZZLE4(x, y, w, w);
		SWIZZLE4(x, y, w, _);
		SWIZZLE4(x, y, _, x);
		SWIZZLE4(x, y, _, y);
		SWIZZLE4(x, y, _, z);
		SWIZZLE4(x, y, _, w);
		SWIZZLE4(x, y, _, _);
		SWIZZLE4(x, z, x, x);
		SWIZZLE4(x, z, x, y);
		SWIZZLE4(x, z, x, z);
		SWIZZLE4(x, z, x, w);
		SWIZZLE4(x, z, x, _);
		SWIZZLE4(x, z, y, x);
		SWIZZLE4(x, z, y, y);
		SWIZZLE4(x, z, y, z);
		SWIZZLE4(x, z, y, w);
		SWIZZLE4(x, z, y, _);
		SWIZZLE4(x, z, z, x);
		SWIZZLE4(x, z, z, y);
		SWIZZLE4(x, z, z, z);
		SWIZZLE4(x, z, z, w);
		SWIZZLE4(x, z, z, _);
		SWIZZLE4(x, z, w, x);
		SWIZZLE4(x, z, w, y);
		SWIZZLE4(x, z, w, z);
		SWIZZLE4(x, z, w, w);
		SWIZZLE4(x, z, w, _);
		SWIZZLE4(x, z, _, x);
		SWIZZLE4(x, z, _, y);
		SWIZZLE4(x, z, _, z);
		SWIZZLE4(x, z, _, w);
		SWIZZLE4(x, z, _, _);
		SWIZZLE4(x, w, x, x);
		SWIZZLE4(x, w, x, y);
		SWIZZLE4(x, w, x, z);
		SWIZZLE4(x, w, x, w);
		SWIZZLE4(x, w, x, _);
		SWIZZLE4(x, w, y, x);
		SWIZZLE4(x, w, y, y);
		SWIZZLE4(x, w, y, z);
		SWIZZLE4(x, w, y, w);
		SWIZZLE4(x, w, y, _);
		SWIZZLE4(x, w, z, x);
		SWIZZLE4(x, w, z, y);
		SWIZZLE4(x, w, z, z);
		SWIZZLE4(x, w, z, w);
		SWIZZLE4(x, w, z, _);
		SWIZZLE4(x, w, w, x);
		SWIZZLE4(x, w, w, y);
		SWIZZLE4(x, w, w, z);
		SWIZZLE4(x, w, w, w);
		SWIZZLE4(x, w, w, _);
		SWIZZLE4(x, w, _, x);
		SWIZZLE4(x, w, _, y);
		SWIZZLE4(x, w, _, z);
		SWIZZLE4(x, w, _, w);
		SWIZZLE4(x, w, _, _);
		SWIZZLE4(x, _, x, x);
		SWIZZLE4(x, _, x, y);
		SWIZZLE4(x, _, x, z);
		SWIZZLE4(x, _, x, w);
		SWIZZLE4(x, _, x, _);
		SWIZZLE4(x, _, y, x);
		SWIZZLE4(x, _, y, y);
		SWIZZLE4(x, _, y, z);
		SWIZZLE4(x, _, y, w);
		SWIZZLE4(x, _, y, _);
		SWIZZLE4(x, _, z, x);
		SWIZZLE4(x, _, z, y);
		SWIZZLE4(x, _, z, z);
		SWIZZLE4(x, _, z, w);
		SWIZZLE4(x, _, z, _);
		SWIZZLE4(x, _, w, x);
		SWIZZLE4(x, _, w, y);
		SWIZZLE4(x, _, w, z);
		SWIZZLE4(x, _, w, w);
		SWIZZLE4(x, _, w, _);
		SWIZZLE4(x, _, _, x);
		SWIZZLE4(x, _, _, y);
		SWIZZLE4(x, _, _, z);
		SWIZZLE4(x, _, _, w);
		SWIZZLE4(x, _, _, _);
		SWIZZLE4(y, x, x, x);
		SWIZZLE4(y, x, x, y);
		SWIZZLE4(y, x, x, z);
		SWIZZLE4(y, x, x, w);
		SWIZZLE4(y, x, x, _);
		SWIZZLE4(y, x, y, x);
		SWIZZLE4(y, x, y, y);
		SWIZZLE4(y, x, y, z);
		SWIZZLE4(y, x, y, w);
		SWIZZLE4(y, x, y, _);
		SWIZZLE4(y, x, z, x);
		SWIZZLE4(y, x, z, y);
		SWIZZLE4(y, x, z, z);
		SWIZZLE4(y, x, z, w);
		SWIZZLE4(y, x, z, _);
		SWIZZLE4(y, x, w, x);
		SWIZZLE4(y, x, w, y);
		SWIZZLE4(y, x, w, z);
		SWIZZLE4(y, x, w, w);
		SWIZZLE4(y, x, w, _);
		SWIZZLE4(y, x, _, x);
		SWIZZLE4(y, x, _, y);
		SWIZZLE4(y, x, _, z);
		SWIZZLE4(y, x, _, w);
		SWIZZLE4(y, x, _, _);
		SWIZZLE4(y, y, x, x);
		SWIZZLE4(y, y, x, y);
		SWIZZLE4(y, y, x, z);
		SWIZZLE4(y, y, x, w);
		SWIZZLE4(y, y, x, _);
		SWIZZLE4(y, y, y, x);
		SWIZZLE4(y, y, y, y);
		SWIZZLE4(y, y, y, z);
		SWIZZLE4(y, y, y, w);
		SWIZZLE4(y, y, y, _);
		SWIZZLE4(y, y, z, x);
		SWIZZLE4(y, y, z, y);
		SWIZZLE4(y, y, z, z);
		SWIZZLE4(y, y, z, w);
		SWIZZLE4(y, y, z, _);
		SWIZZLE4(y, y, w, x);
		SWIZZLE4(y, y, w, y);
		SWIZZLE4(y, y, w, z);
		SWIZZLE4(y, y, w, w);
		SWIZZLE4(y, y, w, _);
		SWIZZLE4(y, y, _, x);
		SWIZZLE4(y, y, _, y);
		SWIZZLE4(y, y, _, z);
		SWIZZLE4(y, y, _, w);
		SWIZZLE4(y, y, _, _);
		SWIZZLE4(y, z, x, x);
		SWIZZLE4(y, z, x, y);
		SWIZZLE4(y, z, x, z);
		SWIZZLE4(y, z, x, w);
		SWIZZLE4(y, z, x, _);
		SWIZZLE4(y, z, y, x);
		SWIZZLE4(y, z, y, y);
		SWIZZLE4(y, z, y, z);
		SWIZZLE4(y, z, y, w);
		SWIZZLE4(y, z, y, _);
		SWIZZLE4(y, z, z, x);
		SWIZZLE4(y, z, z, y);
		SWIZZLE4(y, z, z, z);
		SWIZZLE4(y, z, z, w);
		SWIZZLE4(y, z, z, _);
		SWIZZLE4(y, z, w, x);
		SWIZZLE4(y, z, w, y);
		SWIZZLE4(y, z, w, z);
		SWIZZLE4(y, z, w, w);
		SWIZZLE4(y, z, w, _);
		SWIZZLE4(y, z, _, x);
		SWIZZLE4(y, z, _, y);
		SWIZZLE4(y, z, _, z);
		SWIZZLE4(y, z, _, w);
		SWIZZLE4(y, z, _, _);
		SWIZZLE4(y, w, x, x);
		SWIZZLE4(y, w, x, y);
		SWIZZLE4(y, w, x, z);
		SWIZZLE4(y, w, x, w);
		SWIZZLE4(y, w, x, _);
		SWIZZLE4(y, w, y, x);
		SWIZZLE4(y, w, y, y);
		SWIZZLE4(y, w, y, z);
		SWIZZLE4(y, w, y, w);
		SWIZZLE4(y, w, y, _);
		SWIZZLE4(y, w, z, x);
		SWIZZLE4(y, w, z, y);
		SWIZZLE4(y, w, z, z);
		SWIZZLE4(y, w, z, w);
		SWIZZLE4(y, w, z, _);
		SWIZZLE4(y, w, w, x);
		SWIZZLE4(y, w, w, y);
		SWIZZLE4(y, w, w, z);
		SWIZZLE4(y, w, w, w);
		SWIZZLE4(y, w, w, _);
		SWIZZLE4(y, w, _, x);
		SWIZZLE4(y, w, _, y);
		SWIZZLE4(y, w, _, z);
		SWIZZLE4(y, w, _, w);
		SWIZZLE4(y, w, _, _);
		SWIZZLE4(y, _, x, x);
		SWIZZLE4(y, _, x, y);
		SWIZZLE4(y, _, x, z);
		SWIZZLE4(y, _, x, w);
		SWIZZLE4(y, _, x, _);
		SWIZZLE4(y, _, y, x);
		SWIZZLE4(y, _, y, y);
		SWIZZLE4(y, _, y, z);
		SWIZZLE4(y, _, y, w);
		SWIZZLE4(y, _, y, _);
		SWIZZLE4(y, _, z, x);
		SWIZZLE4(y, _, z, y);
		SWIZZLE4(y, _, z, z);
		SWIZZLE4(y, _, z, w);
		SWIZZLE4(y, _, z, _);
		SWIZZLE4(y, _, w, x);
		SWIZZLE4(y, _, w, y);
		SWIZZLE4(y, _, w, z);
		SWIZZLE4(y, _, w, w);
		SWIZZLE4(y, _, w, _);
		SWIZZLE4(y, _, _, x);
		SWIZZLE4(y, _, _, y);
		SWIZZLE4(y, _, _, z);
		SWIZZLE4(y, _, _, w);
		SWIZZLE4(y, _, _, _);
		SWIZZLE4(z, x, x, x);
		SWIZZLE4(z, x, x, y);
		SWIZZLE4(z, x, x, z);
		SWIZZLE4(z, x, x, w);
		SWIZZLE4(z, x, x, _);
		SWIZZLE4(z, x, y, x);
		SWIZZLE4(z, x, y, y);
		SWIZZLE4(z, x, y, z);
		SWIZZLE4(z, x, y, w);
		SWIZZLE4(z, x, y, _);
		SWIZZLE4(z, x, z, x);
		SWIZZLE4(z, x, z, y);
		SWIZZLE4(z, x, z, z);
		SWIZZLE4(z, x, z, w);
		SWIZZLE4(z, x, z, _);
		SWIZZLE4(z, x, w, x);
		SWIZZLE4(z, x, w, y);
		SWIZZLE4(z, x, w, z);
		SWIZZLE4(z, x, w, w);
		SWIZZLE4(z, x, w, _);
		SWIZZLE4(z, x, _, x);
		SWIZZLE4(z, x, _, y);
		SWIZZLE4(z, x, _, z);
		SWIZZLE4(z, x, _, w);
		SWIZZLE4(z, x, _, _);
		SWIZZLE4(z, y, x, x);
		SWIZZLE4(z, y, x, y);
		SWIZZLE4(z, y, x, z);
		SWIZZLE4(z, y, x, w);
		SWIZZLE4(z, y, x, _);
		SWIZZLE4(z, y, y, x);
		SWIZZLE4(z, y, y, y);
		SWIZZLE4(z, y, y, z);
		SWIZZLE4(z, y, y, w);
		SWIZZLE4(z, y, y, _);
		SWIZZLE4(z, y, z, x);
		SWIZZLE4(z, y, z, y);
		SWIZZLE4(z, y, z, z);
		SWIZZLE4(z, y, z, w);
		SWIZZLE4(z, y, z, _);
		SWIZZLE4(z, y, w, x);
		SWIZZLE4(z, y, w, y);
		SWIZZLE4(z, y, w, z);
		SWIZZLE4(z, y, w, w);
		SWIZZLE4(z, y, w, _);
		SWIZZLE4(z, y, _, x);
		SWIZZLE4(z, y, _, y);
		SWIZZLE4(z, y, _, z);
		SWIZZLE4(z, y, _, w);
		SWIZZLE4(z, y, _, _);
		SWIZZLE4(z, z, x, x);
		SWIZZLE4(z, z, x, y);
		SWIZZLE4(z, z, x, z);
		SWIZZLE4(z, z, x, w);
		SWIZZLE4(z, z, x, _);
		SWIZZLE4(z, z, y, x);
		SWIZZLE4(z, z, y, y);
		SWIZZLE4(z, z, y, z);
		SWIZZLE4(z, z, y, w);
		SWIZZLE4(z, z, y, _);
		SWIZZLE4(z, z, z, x);
		SWIZZLE4(z, z, z, y);
		SWIZZLE4(z, z, z, z);
		SWIZZLE4(z, z, z, w);
		SWIZZLE4(z, z, z, _);
		SWIZZLE4(z, z, w, x);
		SWIZZLE4(z, z, w, y);
		SWIZZLE4(z, z, w, z);
		SWIZZLE4(z, z, w, w);
		SWIZZLE4(z, z, w, _);
		SWIZZLE4(z, z, _, x);
		SWIZZLE4(z, z, _, y);
		SWIZZLE4(z, z, _, z);
		SWIZZLE4(z, z, _, w);
		SWIZZLE4(z, z, _, _);
		SWIZZLE4(z, w, x, x);
		SWIZZLE4(z, w, x, y);
		SWIZZLE4(z, w, x, z);
		SWIZZLE4(z, w, x, w);
		SWIZZLE4(z, w, x, _);
		SWIZZLE4(z, w, y, x);
		SWIZZLE4(z, w, y, y);
		SWIZZLE4(z, w, y, z);
		SWIZZLE4(z, w, y, w);
		SWIZZLE4(z, w, y, _);
		SWIZZLE4(z, w, z, x);
		SWIZZLE4(z, w, z, y);
		SWIZZLE4(z, w, z, z);
		SWIZZLE4(z, w, z, w);
		SWIZZLE4(z, w, z, _);
		SWIZZLE4(z, w, w, x);
		SWIZZLE4(z, w, w, y);
		SWIZZLE4(z, w, w, z);
		SWIZZLE4(z, w, w, w);
		SWIZZLE4(z, w, w, _);
		SWIZZLE4(z, w, _, x);
		SWIZZLE4(z, w, _, y);
		SWIZZLE4(z, w, _, z);
		SWIZZLE4(z, w, _, w);
		SWIZZLE4(z, w, _, _);
		SWIZZLE4(z, _, x, x);
		SWIZZLE4(z, _, x, y);
		SWIZZLE4(z, _, x, z);
		SWIZZLE4(z, _, x, w);
		SWIZZLE4(z, _, x, _);
		SWIZZLE4(z, _, y, x);
		SWIZZLE4(z, _, y, y);
		SWIZZLE4(z, _, y, z);
		SWIZZLE4(z, _, y, w);
		SWIZZLE4(z, _, y, _);
		SWIZZLE4(z, _, z, x);
		SWIZZLE4(z, _, z, y);
		SWIZZLE4(z, _, z, z);
		SWIZZLE4(z, _, z, w);
		SWIZZLE4(z, _, z, _);
		SWIZZLE4(z, _, w, x);
		SWIZZLE4(z, _, w, y);
		SWIZZLE4(z, _, w, z);
		SWIZZLE4(z, _, w, w);
		SWIZZLE4(z, _, w, _);
		SWIZZLE4(z, _, _, x);
		SWIZZLE4(z, _, _, y);
		SWIZZLE4(z, _, _, z);
		SWIZZLE4(z, _, _, w);
		SWIZZLE4(z, _, _, _);
		SWIZZLE4(w, x, x, x);
		SWIZZLE4(w, x, x, y);
		SWIZZLE4(w, x, x, z);
		SWIZZLE4(w, x, x, w);
		SWIZZLE4(w, x, x, _);
		SWIZZLE4(w, x, y, x);
		SWIZZLE4(w, x, y, y);
		SWIZZLE4(w, x, y, z);
		SWIZZLE4(w, x, y, w);
		SWIZZLE4(w, x, y, _);
		SWIZZLE4(w, x, z, x);
		SWIZZLE4(w, x, z, y);
		SWIZZLE4(w, x, z, z);
		SWIZZLE4(w, x, z, w);
		SWIZZLE4(w, x, z, _);
		SWIZZLE4(w, x, w, x);
		SWIZZLE4(w, x, w, y);
		SWIZZLE4(w, x, w, z);
		SWIZZLE4(w, x, w, w);
		SWIZZLE4(w, x, w, _);
		SWIZZLE4(w, x, _, x);
		SWIZZLE4(w, x, _, y);
		SWIZZLE4(w, x, _, z);
		SWIZZLE4(w, x, _, w);
		SWIZZLE4(w, x, _, _);
		SWIZZLE4(w, y, x, x);
		SWIZZLE4(w, y, x, y);
		SWIZZLE4(w, y, x, z);
		SWIZZLE4(w, y, x, w);
		SWIZZLE4(w, y, x, _);
		SWIZZLE4(w, y, y, x);
		SWIZZLE4(w, y, y, y);
		SWIZZLE4(w, y, y, z);
		SWIZZLE4(w, y, y, w);
		SWIZZLE4(w, y, y, _);
		SWIZZLE4(w, y, z, x);
		SWIZZLE4(w, y, z, y);
		SWIZZLE4(w, y, z, z);
		SWIZZLE4(w, y, z, w);
		SWIZZLE4(w, y, z, _);
		SWIZZLE4(w, y, w, x);
		SWIZZLE4(w, y, w, y);
		SWIZZLE4(w, y, w, z);
		SWIZZLE4(w, y, w, w);
		SWIZZLE4(w, y, w, _);
		SWIZZLE4(w, y, _, x);
		SWIZZLE4(w, y, _, y);
		SWIZZLE4(w, y, _, z);
		SWIZZLE4(w, y, _, w);
		SWIZZLE4(w, y, _, _);
		SWIZZLE4(w, z, x, x);
		SWIZZLE4(w, z, x, y);
		SWIZZLE4(w, z, x, z);
		SWIZZLE4(w, z, x, w);
		SWIZZLE4(w, z, x, _);
		SWIZZLE4(w, z, y, x);
		SWIZZLE4(w, z, y, y);
		SWIZZLE4(w, z, y, z);
		SWIZZLE4(w, z, y, w);
		SWIZZLE4(w, z, y, _);
		SWIZZLE4(w, z, z, x);
		SWIZZLE4(w, z, z, y);
		SWIZZLE4(w, z, z, z);
		SWIZZLE4(w, z, z, w);
		SWIZZLE4(w, z, z, _);
		SWIZZLE4(w, z, w, x);
		SWIZZLE4(w, z, w, y);
		SWIZZLE4(w, z, w, z);
		SWIZZLE4(w, z, w, w);
		SWIZZLE4(w, z, w, _);
		SWIZZLE4(w, z, _, x);
		SWIZZLE4(w, z, _, y);
		SWIZZLE4(w, z, _, z);
		SWIZZLE4(w, z, _, w);
		SWIZZLE4(w, z, _, _);
		SWIZZLE4(w, w, x, x);
		SWIZZLE4(w, w, x, y);
		SWIZZLE4(w, w, x, z);
		SWIZZLE4(w, w, x, w);
		SWIZZLE4(w, w, x, _);
		SWIZZLE4(w, w, y, x);
		SWIZZLE4(w, w, y, y);
		SWIZZLE4(w, w, y, z);
		SWIZZLE4(w, w, y, w);
		SWIZZLE4(w, w, y, _);
		SWIZZLE4(w, w, z, x);
		SWIZZLE4(w, w, z, y);
		SWIZZLE4(w, w, z, z);
		SWIZZLE4(w, w, z, w);
		SWIZZLE4(w, w, z, _);
		SWIZZLE4(w, w, w, x);
		SWIZZLE4(w, w, w, y);
		SWIZZLE4(w, w, w, z);
		SWIZZLE4(w, w, w, w);
		SWIZZLE4(w, w, w, _);
		SWIZZLE4(w, w, _, x);
		SWIZZLE4(w, w, _, y);
		SWIZZLE4(w, w, _, z);
		SWIZZLE4(w, w, _, w);
		SWIZZLE4(w, w, _, _);
		SWIZZLE4(w, _, x, x);
		SWIZZLE4(w, _, x, y);
		SWIZZLE4(w, _, x, z);
		SWIZZLE4(w, _, x, w);
		SWIZZLE4(w, _, x, _);
		SWIZZLE4(w, _, y, x);
		SWIZZLE4(w, _, y, y);
		SWIZZLE4(w, _, y, z);
		SWIZZLE4(w, _, y, w);
		SWIZZLE4(w, _, y, _);
		SWIZZLE4(w, _, z, x);
		SWIZZLE4(w, _, z, y);
		SWIZZLE4(w, _, z, z);
		SWIZZLE4(w, _, z, w);
		SWIZZLE4(w, _, z, _);
		SWIZZLE4(w, _, w, x);
		SWIZZLE4(w, _, w, y);
		SWIZZLE4(w, _, w, z);
		SWIZZLE4(w, _, w, w);
		SWIZZLE4(w, _, w, _);
		SWIZZLE4(w, _, _, x);
		SWIZZLE4(w, _, _, y);
		SWIZZLE4(w, _, _, z);
		SWIZZLE4(w, _, _, w);
		SWIZZLE4(w, _, _, _);
		SWIZZLE4(_, x, x, x);
		SWIZZLE4(_, x, x, y);
		SWIZZLE4(_, x, x, z);
		SWIZZLE4(_, x, x, w);
		SWIZZLE4(_, x, x, _);
		SWIZZLE4(_, x, y, x);
		SWIZZLE4(_, x, y, y);
		SWIZZLE4(_, x, y, z);
		SWIZZLE4(_, x, y, w);
		SWIZZLE4(_, x, y, _);
		SWIZZLE4(_, x, z, x);
		SWIZZLE4(_, x, z, y);
		SWIZZLE4(_, x, z, z);
		SWIZZLE4(_, x, z, w);
		SWIZZLE4(_, x, z, _);
		SWIZZLE4(_, x, w, x);
		SWIZZLE4(_, x, w, y);
		SWIZZLE4(_, x, w, z);
		SWIZZLE4(_, x, w, w);
		SWIZZLE4(_, x, w, _);
		SWIZZLE4(_, x, _, x);
		SWIZZLE4(_, x, _, y);
		SWIZZLE4(_, x, _, z);
		SWIZZLE4(_, x, _, w);
		SWIZZLE4(_, x, _, _);
		SWIZZLE4(_, y, x, x);
		SWIZZLE4(_, y, x, y);
		SWIZZLE4(_, y, x, z);
		SWIZZLE4(_, y, x, w);
		SWIZZLE4(_, y, x, _);
		SWIZZLE4(_, y, y, x);
		SWIZZLE4(_, y, y, y);
		SWIZZLE4(_, y, y, z);
		SWIZZLE4(_, y, y, w);
		SWIZZLE4(_, y, y, _);
		SWIZZLE4(_, y, z, x);
		SWIZZLE4(_, y, z, y);
		SWIZZLE4(_, y, z, z);
		SWIZZLE4(_, y, z, w);
		SWIZZLE4(_, y, z, _);
		SWIZZLE4(_, y, w, x);
		SWIZZLE4(_, y, w, y);
		SWIZZLE4(_, y, w, z);
		SWIZZLE4(_, y, w, w);
		SWIZZLE4(_, y, w, _);
		SWIZZLE4(_, y, _, x);
		SWIZZLE4(_, y, _, y);
		SWIZZLE4(_, y, _, z);
		SWIZZLE4(_, y, _, w);
		SWIZZLE4(_, y, _, _);
		SWIZZLE4(_, z, x, x);
		SWIZZLE4(_, z, x, y);
		SWIZZLE4(_, z, x, z);
		SWIZZLE4(_, z, x, w);
		SWIZZLE4(_, z, x, _);
		SWIZZLE4(_, z, y, x);
		SWIZZLE4(_, z, y, y);
		SWIZZLE4(_, z, y, z);
		SWIZZLE4(_, z, y, w);
		SWIZZLE4(_, z, y, _);
		SWIZZLE4(_, z, z, x);
		SWIZZLE4(_, z, z, y);
		SWIZZLE4(_, z, z, z);
		SWIZZLE4(_, z, z, w);
		SWIZZLE4(_, z, z, _);
		SWIZZLE4(_, z, w, x);
		SWIZZLE4(_, z, w, y);
		SWIZZLE4(_, z, w, z);
		SWIZZLE4(_, z, w, w);
		SWIZZLE4(_, z, w, _);
		SWIZZLE4(_, z, _, x);
		SWIZZLE4(_, z, _, y);
		SWIZZLE4(_, z, _, z);
		SWIZZLE4(_, z, _, w);
		SWIZZLE4(_, z, _, _);
		SWIZZLE4(_, w, x, x);
		SWIZZLE4(_, w, x, y);
		SWIZZLE4(_, w, x, z);
		SWIZZLE4(_, w, x, w);
		SWIZZLE4(_, w, x, _);
		SWIZZLE4(_, w, y, x);
		SWIZZLE4(_, w, y, y);
		SWIZZLE4(_, w, y, z);
		SWIZZLE4(_, w, y, w);
		SWIZZLE4(_, w, y, _);
		SWIZZLE4(_, w, z, x);
		SWIZZLE4(_, w, z, y);
		SWIZZLE4(_, w, z, z);
		SWIZZLE4(_, w, z, w);
		SWIZZLE4(_, w, z, _);
		SWIZZLE4(_, w, w, x);
		SWIZZLE4(_, w, w, y);
		SWIZZLE4(_, w, w, z);
		SWIZZLE4(_, w, w, w);
		SWIZZLE4(_, w, w, _);
		SWIZZLE4(_, w, _, x);
		SWIZZLE4(_, w, _, y);
		SWIZZLE4(_, w, _, z);
		SWIZZLE4(_, w, _, w);
		SWIZZLE4(_, w, _, _);
		SWIZZLE4(_, _, x, x);
		SWIZZLE4(_, _, x, y);
		SWIZZLE4(_, _, x, z);
		SWIZZLE4(_, _, x, w);
		SWIZZLE4(_, _, x, _);
		SWIZZLE4(_, _, y, x);
		SWIZZLE4(_, _, y, y);
		SWIZZLE4(_, _, y, z);
		SWIZZLE4(_, _, y, w);
		SWIZZLE4(_, _, y, _);
		SWIZZLE4(_, _, z, x);
		SWIZZLE4(_, _, z, y);
		SWIZZLE4(_, _, z, z);
		SWIZZLE4(_, _, z, w);
		SWIZZLE4(_, _, z, _);
		SWIZZLE4(_, _, w, x);
		SWIZZLE4(_, _, w, y);
		SWIZZLE4(_, _, w, z);
		SWIZZLE4(_, _, w, w);
		SWIZZLE4(_, _, w, _);
		SWIZZLE4(_, _, _, x);
		SWIZZLE4(_, _, _, y);
		SWIZZLE4(_, _, _, z);
		SWIZZLE4(_, _, _, w);

	private:
		std::array<T, L> _elems;
		constexpr T crs(const Vec& o, const usize a, const usize b) const {
			return _elems[a] * o._elems[b] - _elems[b] * o._elems[a];
		}
		__declspec(property(put = set_none, get = get_none)) T _;
		constexpr T set_none(const T& e) const { }
		constexpr T get_none() const {
			return T{};
		}
	};
#undef BINARY_OPERATOR
#undef UNARY_OPERATOR
#undef PROPERTY
#undef SWIZZLE2
#undef SWIZZLE3
#undef SWIZZLE4

	template<typename T, const usize L>
	std::ostream& operator<<(std::ostream& os, const Vec<T, L>& vec) {
		os << "{ ";
		for (usize i = 0; i < L; ++i) {
			if (i > 0) {
				os << ", ";
			}
			os << vec[i];
		}
		return os << " }";
	}
	
}

template<typename T>
using Vec2 = math::Vec<T, 2>;
template<typename T>
using Vec3 = math::Vec<T, 3>;
template<typename T>
using Vec4 = math::Vec<T, 4>;