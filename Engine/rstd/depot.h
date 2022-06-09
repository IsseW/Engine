#pragma once
#include"primitives.h"
#include"option.h"
#include"vector.h"

template<typename T>
struct Id {
	u32 idx() const { return _idx; }
	u32 gen() const { return _gen; }
private:
	u32 _idx;
	u32 _gen;
};
template<typename T>
struct Depot {
	bool is_empty() { return _len == 0; }
	usize len() { return _len; }
	bool contains(Id<T> id) {
		return _entries.get(id.idx())
			.map([&](const Entry& e) { return e.gen == id.gen() && e.item.is_some(); })
			.unwrap_or(std::move(false));
	}

	Option<const T&> get(Id<T> id) const {
		Option<Entry&> entry = _entries.get(id.idx());
		if (entry.is_some()) {
			Entry& e = entry.unwrap_unchecked();
			if (e.gen == id.gen) {
				return e.item.as_ref();
			}
		}
		return none<const T&>();
	}

	Option<T&> get(Id<T> id) {
		Option<Entry&> entry = _entries.get(id.idx());
		if (entry.is_some()) {
			Entry& e = entry.unwrap_unchecked();
			if (e.gen == id.gen) {
				return e.item.as_ref();
			}
		}
		return none<T&>();
	}

	Id<T> insert(T&& item) {
		if (_len < _entries.len()) {
			u32 i = 0;
			for (Entry& e : _entries) {
				if (e.item.is_none()) {
					e.item = some(std::move(item));
					ASSERT(e.gen < UINT32_MAX);
					e.gen++;
					_len += 1;
					return Id{
						i,
						e.gen
					};
				}
				++i;
			}
		}
		else {
			ASSERT(_entries.len() < UINT32_MAX - 1);
			auto id = Id{
				_entries.len(),
				0
			};
			_entries.push(std::move(Entry{
					0,
					some(std::move(item))
				}));
			_len += 1;
			return id;
		}
		PANIC("This shouldn't happen");
	}

	Option<T>&& remove(Id<T> id) {
		return _entries.get(id.idx()).and_then<Option<T>>([&](auto&& e) {
				if (e.gen == id.gen()) {
					_len -= 1;
					return e.item.take();
				}
				else {
					return none<Option<T>&&>();
				}
			});
	}
	
private:
	struct Entry {
		u32 gen;
		Option<T> item;
	};
	Vec<Entry> _entries;
	usize _len;
};