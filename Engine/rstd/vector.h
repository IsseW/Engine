#pragma once
#include<vector>
#include<concepts>
#include"option.h"

template<typename T>
struct Vec {
	Vec() : _internal{} {}
	Vec(T elem, usize len) requires std::copyable<T> : _internal{} {
		for (usize i = 0; i < len; ++i) {
			this->_internal.push_back(elem);
		}
	}
	usize len() const { return _internal.size(); }

	bool is_empty() { return len() == 0; }

	void push(T&& t) {
		_internal.push_back(std::move(t));
	}

	void insert(T elem, usize index) {
		_internal.insert(elem, index);
	}

	Option<T> pop() {
		if (!is_empty()) {
			return some<T>(std::move(_internal.pop_back()));
		}
		else {
			return none<T>();
		}
	}

	Option<T*> get(usize i) {
		if (i < len()) {
			return some(&_internal[i]);
		}
		else {
			return none<T*>();
		}
	}

	Option<const T*> get(usize i) const {
		if (i < len()) {
			return some(&_internal[i]);
		}
		else {
			return none<const T*>();
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
	auto begin() const {
		return _internal.begin();
	}
	auto end() {
		return _internal.end();
	}
	auto end() const {
		return _internal.end();
	}


private:
	std::vector<T> _internal;
};