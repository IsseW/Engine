#pragma once
#include<utility>
#include"panic.h"

template<typename T>
struct Option {
	Option() : _is_some(false) { }
	Option(T& v) : _is_some(true), _v(v) { }
	Option(T&& v) : _is_some(true), _v(v) { }

	T&& unwrap() {
		if (_is_some) {
			return std::move(_v);
		}
		else {
			PANIC("Trying to unwrap Option::None");
		}
	}

	Option<T&> as_ref() {
		return Option(_v);
	}
private:
	bool _is_some;
	T _v;
};