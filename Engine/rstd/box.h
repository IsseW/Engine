#pragma once
#include<memory>
#include<concepts>

template<typename T>
struct Box {
	Box() requires std::is_default_constructible<T>::value : _inner{} {}
	Box(T&& val) : _inner{ std::make_unique(val) } {}
	Box(Box&& other) : _inner{ other._inner } {}

	static Box from_ptr(T* ptr) {
		auto box = Box{};
		box._inner = std::unique_ptr{ ptr };
		return box;
	}

	const T& get() const {
		return *this->_inner.get();
	}

	T& get() {
		return *this->_inner.get();
	}

	const T& operator->() const {
		return this->get();
	}

	T& operator->() {
		return this->get();
	}

	Box& operator=(Box&& other) {
		this->_inner = other._inner;
		return *this;
	}
private:
	std::unique_ptr<T> _inner;
};