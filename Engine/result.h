#pragma once
#include<utility>
#include"option.h"
#include"panic.h"

template<typename T, typename E>
struct Result {
	static Result<T, E> ok(const T& v) {
		Result<T, E> res{};
		res._ok = v;
		res._is_ok = true;
		return res;
	}
	static Result<T, E> ok(T&& v) {
		Result<T, E> res{};
		res._ok = v;
		res._is_ok = true;
		return res;
	}
	static Result<T, E> err(const E& v) {
		Result<T, E> res{};
		res._err = v;
		res._is_ok = false;
		return res;
	}
	static Result<T, E> err(E&& v) {
		Result<T, E> res{};
		res._err = v;
		res._is_ok = false;
		return res;
	}

	T&& unwrap() {
		if (this->_is_ok) {
			std::move(this->_ok);
		} else {
			PANIC("Trying to unwrap err.");
		}
	}

	Result<T&, E&> as_ref() {
		if (this->is_ok()) {
			return Result<T&, E&>::ok(this->_ok);
		}
		else {
			return Result<T&, E&>::err(this->_err);
		}

	}

	template<typename U, typename F>
	Result<U, E> map(F f) {
		if (this->is_ok()) {
			return Result<U, E>::ok(f(this->_ok));
		}
		else {
			return Result<U, E>::err(this->_err);
		}

	}

	bool is_ok() {
		return this->_is_ok;
	}
private:
	Result() {
		_is_ok = true;
		_t = 0;
	}

	bool _is_ok;
	union {
		T _ok;
		E _err;
		char _t;
	};
};