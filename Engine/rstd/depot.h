#pragma once
#include"primitives.h"
#include"option.h"
#include"vector.h"

template<typename T>
struct Id {
	Id(u32 idx, u32 gen) : _idx(idx), _gen(gen) {}
	u32 idx() const { return _idx; }
	u32 gen() const { return _gen; }
private:
	u32 _idx;
	u32 _gen;
};
template<typename T>
struct Depot {
	Depot() : _entries(), _free_entries() {}
	bool is_empty() { return len() == 0; }
	usize len() { return _entries.len() - _free_entries.len(); }
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
		Option<u32> free = _free_entries.pop();
		if (free.is_some()) {
			u32 idx = free.unwrap_unchecked();
			ASSERT(++_entries[idx].gen < UINT32_MAX);
			_entries[idx].item.insert(std::move(item));
			return Id<T>(idx, _entries[idx].gen);
		}
		else {
			ASSERT(_entries.len() < UINT32_MAX - 1);
			auto id = Id<T>(_entries.len(), 0);
			_entries.push(std::move(Entry{
					0,
					some(std::move(item))
				}));
			return id;
		}
		PANIC("This shouldn't happen");
	}

	Option<T>&& remove(Id<T> id) {
		return std::move(_entries.get(id.idx()).and_then<T>([&](Entry* e) {
				if (e->gen == id.gen()) {
					_free_entries.push(id.idx());
					return e->item.take();
				}
				else {
					return none<T>();
				}
			}));
	}

	template<typename F>
	void values(F f) const {
		for (const Entry& e : _entries) {
			if (e.item.is_some()) {
				f(e.item.as_ref().unwrap_unchecked());
			}
		}
	}

	template<typename F>
	void values(F f) {
		for (Entry& e : _entries) {
			if (e.item.is_some()) {
				f(e.item.as_ptr().unwrap_unchecked());
			}
		}
	}
	
private:
	struct Entry {
		u32 gen;
		Option<T> item;
	};
	Vec<Entry> _entries;
	Vec<u32> _free_entries;
};