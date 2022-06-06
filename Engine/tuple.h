#pragma once
#include<tuple>

template<typename ...T>
struct Tuple {
	template<int index>
	constexpr auto get() {
		static_assert(index < sizeof...(T), "Trying to index tuple out of bounds.");
		return std::get<index>(_inner);
	}
private:
	std::tuple<T...> _inner;
};