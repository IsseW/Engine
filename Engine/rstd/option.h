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

	Option(Option&& o) requires std::movable<T> {
		*this = o.take();
	}

	Option&& take() requires std::movable<T> {
		Option res = std::move(*this);
		_is_some = false;
		return std::move(res);
	}

	template<typename U, typename F>
	Option<U> and_then(F f) requires std::movable<T> {
		if (is_some()) {
			return f(take());
		}
		else {
			return none();
		}
	}

	T&& unwrap_unchecked() requires std::movable<T> {
		return std::move(_v);
	}

	T&& unwrap() requires std::movable<T> {
		if (_is_some) {
			return take()._v;
		}
		else {
			PANIC("Trying to unwrap Option::None");
		}
	}

	T&& unwrap_or(T&& v) requires std::movable<T> {
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

#define IF_LET_SOME(arg, expr) \
arg ## __option__ ## __LINE__ = expr; \
if arg.is_some() && (true || auto arg = arg ## __option__ ## __LINE__.unwrap_unchecked())