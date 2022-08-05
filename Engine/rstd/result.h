#pragma once
#include<utility>
#include"option.h"
#include"panic.h"
#include<cstdint>
#include<iostream>
#include<concepts>


template<typename T, typename E>
struct Result {
	Result(E&& e) {
		this->_is_ok = false;
		this->_err = std::move(e);
	}
	Result(Result&& r) noexcept {
		this->_is_ok = r._is_ok;
		if (r._is_ok){
			this->_ok = std::move(r._ok);
		}
		else {
			this->_err = std::move(r._err);
		}
		r._is_ok = false;
	}
	~Result() {
		if (is_ok()) {
			// std::cout << "Bye bye " << typeid(T).name() << std::endl;
			_ok.~T();
		}
		else {
			_err.~E();
		}
	}
	static Result ok(const T& v) requires std::copyable<T> {
		Result<T, E> res{};
		res._ok = v;
		res._is_ok = true;
		return res;
	}
	static Result ok(T&& v) requires std::movable<T> {
		Result<T, E> res{};
		res._ok = v;
		res._is_ok = true;
		return res;
	}
	static Result err(const E& v) requires std::copyable<T> {
		Result<T, E> res{};
		res._err = v;
		res._is_ok = false;
		return res;
	}
	static Result err(E&& v) requires std::movable<T> {
		Result<T, E> res{};
		res._err = v;
		res._is_ok = false;
		return res;
	}

	T unwrap() {
		if (is_ok()) {
			return this->_ok;
		} else {
			PANIC("Trying to unwrap err.");
		}
	}

	E unwrap_err() {
		if (is_err()) {
			return this->_err;
		}
		else {
			PANIC("Trying to unwrap err from ok.");
		}
	}
	E unwrap_err_unchecked() {
		return this->_err;
	}

	Option<T> ok() const {
		if (this->is_ok()) {
			Option<T>::some(this->_ok);
		}
		else {
			Option<T>::none();
		}
	}

	Result<T*, E*> as_ptr() {
		if (this->is_ok()) {
			return Result<T*, E*>::ok(&this->_ok);
		}
		else {
			return Result<T*, E*>::err(&this->_err);
		}
	}

	Result<const T*, const E*> as_ptr() const {
		if (this->is_ok()) {
			return Result<const T*, const E*>::ok(&this->_ok);
		}
		else {
			return Result<const T*, const E*>::err(&this->_err);
		}
	}

	template<typename U, typename F>
	Result<U, E> map(F f) const {
		if (this->is_ok()) {
			return Result<U, E>::ok(f(this->_ok));
		}
		else {
			return Result<U, E>::err(this->_err);
		}

	}

	Result<const T, E> as_const() const {
		this->map<const T>([](const auto ok) {
			return ok;
		});
	}

	bool is_ok() const {
		return this->_is_ok;
	}
	bool is_err() const {
		return !this->_is_ok;
	}
private:
	Result() {
		_is_ok = false;
		_t = 0;
	}

	bool _is_ok;
	union {
		T _ok;
		E _err;
		char _t;
	};
};

template<typename T, typename E>
Result<T, E> ok(const T& v) requires std::copyable<T> {
	return Result<T, E>::ok(v);
}
template<typename T, typename E>
Result<T, E> ok(T&& v) requires std::movable<T> {
	return Result<T, E>::ok(std::move(v));
}

template<typename T, typename E>
Result<T, E> err(const E& e) requires std::copyable<T> {
	return Result<T, E>::err(e);
}

template<typename T, typename E>
Result<T, E> err(E&& e) requires std::movable<T> {
	return Result<T, E>::err(std::move(e));
}


template<typename T, typename E>
std::ostream& operator<<(std::ostream& os, const Result<T, E>& o) {
	if (o.is_ok()) {
		const T& t = o.as_ref().unwrap();
		return os << "ok(" << t << ")";
	}
	else {
		const E& e = o.as_ref().unwrap_err();
		return os << "err(" << e << ")";
	}
}

// Tries to unwrap a result, otherwise return the error.
#define TRY(x, result) {\
	auto x ## __res = std::move(result); \
	if ((x ## __res).is_err()) { return std::move(x ## __res.unwrap_err_unchecked()); } \
	x = (x ## __res).unwrap(); \
}