#pragma once
#include<memory>
#include<concepts>

template<typename T>
struct Box {
	Box(T&& val) : _inner{ std::make_unique<T>(val) } {}
	Box(Box&& other) : _inner{ std::move(other._inner) } {}

	static Box from_ptr(T* ptr) {
		Box box = Box{};
		box._inner = std::unique_ptr<T>{ ptr };
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
		this->_inner = std::move(other._inner);
		return *this;
	}
private:
	Box(): _inner{}{}
	std::unique_ptr<T> _inner;
};