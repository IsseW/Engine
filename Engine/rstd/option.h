#pragma once
#include<utility>
#include<iostream>
#include"panic.h"
#include<concepts>

template<typename T>
struct Option {
	static Option<T> none() {
		return Option();
	}
	static Option<T> some(const T& v) requires std::copyable<T> {
		return Option(v);
	}
	static Option<T> some(T&& v) requires std::movable<T> {
		return Option(v);
	}

	T&& unwrap() requires std::movable<T> {
		if (_is_some) {
			return std::move(_v);
		}
		else {
			PANIC("Trying to unwrap Option::None");
		}
	}

	T&& unwrap_or(T&& v) requires std::movable<T> {
		if (_is_some) {
			return std::move(_v);
		}
		else {
			return std::move(v);
		}
	}

	template<typename F>
	T&& unwrap_or_else(F els) requires std::movable<T> {
		if (_is_some) {
			return std::move(_v);
		}
		else {
			return std::move(els());
		}
	}

	Option<T&> as_ref() {
		if (_is_some) {
			return Option<T&>::none();
		}
		else {
			return Option<T&>::some(_v);
		}
	}
	Option<const T&> as_ref() const {
		if (_is_some) {
			return Option<const T&>::none();
		}
		else {
			return Option<const T&>::some(_v);
		}
	}
	
	template<typename U, typename F>
	Option<U> map(F map) const requires std::movable<T> {
		if (_is_some) {
			return map(std::move(_v));
		}
		else {
			return Option<U>::none();
		}
	}

	bool is_some() const {
		return _is_some;
	}
	bool is_none() const {
		return !_is_some;
	}

private:
	Option() : _is_some(false), __t(0) {}
	Option(const T& v) : _is_some(true), _v(v) {}
	Option(T&& v) : _is_some(true), _v(v) {}

	bool _is_some;
	union {
		T _v;
		char __t;
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
	return Option<T>::some(v);
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
