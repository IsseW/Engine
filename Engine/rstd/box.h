#pragma once
#include<memory>
#include<concepts>

template<typename T>
struct Box {
	Box() requires std::is_default_constructible<T>::value : _inner{} {}
	Box(T&& val) : _inner{ std::make_unique(val) } {}
	Box(Box&& other) : _inner{ other._inner } {}

	const T& get() const {
		return *this->_inner.get();
	}

	T& get() {
		return *this->_inner.get();
	}
private:
	std::unique_ptr<T> _inner;
};