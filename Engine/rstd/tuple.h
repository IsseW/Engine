#pragma once
#include<tuple>

template<typename ...T>
struct Tuple {
	Tuple(T... t) : _inner{t...} {}

	template<int index>
	constexpr auto get() {
		static_assert(index < sizeof...(T), "Trying to index tuple out of bounds.");
		return std::get<index>(_inner);
	}
private:
	std::tuple<T...> _inner;
};

template<typename ...T>
Tuple<T...> new_tuple(T... t) {
	return Tuple<T...>{t...};
}