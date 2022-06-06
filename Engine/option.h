#pragma once
#include<optional>

template<typename T>
struct Option {
private:
	std::option<T> _inner;
};