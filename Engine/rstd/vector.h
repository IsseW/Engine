#pragma once
#include<vector>
#include"option.h"

template<typename T>
struct Vec {
	usize len() { return _internal.size(); }

	bool is_empty() { return len() == 0; }

	void push(T&& t) {
		_internal.push_back(std::move(t));
	}

	Option<T> pop() {
		if (!is_empty()) {
			return some<T>(_internal.pop_back());
		}
		else {
			return none<T>();
		}
	}

	Option<T&> get(usize i) {
		if (i < len()) {
			return some<T&>(_internal[i]);
		}
		else {
			return none<T&>();
		}
	}

	Option<const T&> get(usize i) const {
		if (i < len()) {
			return some<const T&>(_internal[i]);
		}
		else {
			return none<const T&>();
		}
	}

	T& operator[](usize i) {
		return _internal[i];
	}
	const T& operator[](usize i) const {
		return _internal[i];
	}

	auto begin() {
		return _internal.begin();
	}
	auto end() {
		return _internal.end();
	}


private:
	std::vector<T> _internal;
};