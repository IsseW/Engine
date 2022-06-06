#pragma once

template<typename T, typename E>
struct Result {
	Result(T& v) : is_ok{true} {}
	Result(T&&) : is_ok{ true } {}
	Result(E& e) : is_ok{ false }
private:
	bool is_ok;
	union {
		T ok;
		E err;
	} _inner;
};