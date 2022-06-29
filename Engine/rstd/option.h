#pragma once
#include<utility>
#include<iostream>
#include"panic.h"
#include<concepts>

template<typename T>
struct Option {
	static Option none() {
		return Option();
	}
	static Option some(const T& v) requires std::copyable<T> {
		return Option(v);
	}
	static Option some(T&& v) requires std::movable<T> {
		return Option(v);
	}

	~Option() {
		if (_is_some) {
			_v.~T();
		}
	}

	Option() : _is_some(false) { zero(); }

	Option(Option&& other) {
		if (other.is_some()) {
			_is_some = true;
			zero();
			_v = std::move(other._v);
		}
		else {
			_is_some = false;
			zero();
		}
		other._is_some = false;
	}

	Option(const Option& other) {
		if (other.is_some()) {
			_is_some = true;
			zero();
			_v = other._v;
		}
		else {
			_is_some = false;
			zero();
		}
	}

	Option& operator=(const Option& other) {
		if (other.is_some()) {
			_is_some = true;
			_v = other._v;
		}
		else {
			_is_some = false;
			zero();
		}
		return *this;
	}

	Option&& take() requires std::movable<T> {
		Option res(std::move(*this));
		_is_some = false;
		return std::move(res);
	}

	template<typename U, typename F>
	Option<U> and_then(F f) {
		if (is_some()) {
			return f(unwrap_unchecked());
		}
		else {
			return Option<U>::none();
		}
	}

	template<typename U>
	Option<U>&& flatten() requires std::same_as<Option<U>, T> {
		if (_is_some) {
			return std::move(unwrap_unchecked());
		}
		return std::move(Option<U>::none());
	}

	T&& unwrap_unchecked() requires std::movable<T> {
		return std::move(take()._v);
	}

	T&& unwrap() requires std::movable<T> {
		if (_is_some) {
			return take()._v;
		}
		else {
			PANIC("Trying to unwrap Option::None");
		}
	}

	T&& unwrap_or(T v) {
		if (_is_some) {
			return take()._v;
		}
		else {
			return std::move(v);
		}
	}

	template<typename F>
	T&& unwrap_or_else(F els) requires std::movable<T> {
		if (_is_some) {
			return take()._v;
		}
		else {
			return std::move(els());
		}
	}

	Option<T*> as_ptr() {
		if (_is_some) {
			return Option<T*>::some(&_v);
		}
		return Option<T*>::none();
	}
	Option<const T*> as_ptr() const {
		if (_is_some) {
			return Option<const T*>::some(&_v);
		}
		return Option<const T*>::none();
	}
	template<typename F>
	void then_do(F func) requires std::movable<T> {
		if (_is_some) {
			func(take().unwrap_unchecked());
		}
	}
	
	template<typename U, typename F>
	Option<U> map(F map) requires std::movable<T> {
		if (_is_some) {
			return Option<U>::some(map(take().unwrap_unchecked()));
		}
		return Option<U>::none();
	}

	bool is_some() const {
		return _is_some;
	}
	bool is_none() const {
		return !_is_some;
	}

	Option<T> insert(T&& item) requires std::movable<T> {
		Option res(std::move(*this));
		_is_some = true;
		_v = std::move(item);
		return res;
	}

private:
	Option(const T& v) : _is_some(true), _v(v) {}

	void zero() {
		memset(__t, 0, sizeof(T));
	}

	bool _is_some;
	union {
		T _v;
		char __t[sizeof(T)];
	};
};


template<typename T>
Option<T> none() {
	return Option<T>::none();
}
template<typename T>
Option<T> some(const T& v) requires std::copyable<T> {
	return Option<T>::some(v);
}
template<typename T>
Option<T> some(T&& v) requires std::movable<T> {
	return Option<T>::some(std::move(v));
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Option<T>& o) {
	if (o.is_some()) {
		const T& t = o.as_ref().unwrap();
		return os << "some(" << t << ")";
	}
	else {
		return os << "none";
	}
}

#define IF_LET_SOME(arg, expr) \
arg ## __option__ ## __LINE__ = expr; \
if arg.is_some() && (true || auto arg = arg ## __option__ ## __LINE__.unwrap_unchecked())