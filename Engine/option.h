#pragma once
#include<utility>
#include"panic.h"

template<typename T>
struct Option {
	static Option<T> none() {
		return Option();
	}
	static Option<T> some(const T& v) {
		return Option(v);
	}
	static Option<T> some(T&& v) {
		return Option(v);
	}

	T&& unwrap() {
		if (_is_some) {
			return std::move(_v);
		}
		else {
			PANIC("Trying to unwrap Option::None");
		}
	}

	T&& unwrap_or(T&& v) {
		if (_is_some) {
			return std::move(_v);
		}
		else {
			return std::move(v);
		}
	}

	template<typename F>
	T&& unwrap_or_else(F els) {
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
	
	template<typename U, typename F>
	Option<U> map(F map) {
		if (_is_some) {
			return map(std::move(_v));
		}
		else {
			return Option<U>::none();
		}
	}

	bool is_some() {
		return _is_some;
	}
	bool is_none() {
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
Option<T> some(const T& v) {
	return Option<T>::some(v);
}
template<typename T>
Option<T> some(T&& v) {
	return Option<T>::some(v);
}