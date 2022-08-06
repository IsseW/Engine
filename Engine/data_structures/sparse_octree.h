#pragma once
#include <rstd/primitives.h>
#include <math/aab.h>
#include <rstd/panic.h>
#include <rstd/box.h>
#include <rstd/result.h>
#include <rstd/tuple.h>

enum class OctreeError {
	OutOfBounds,
	EmptyNode,
};

struct OctreeNode {
	OctreeNode() = default;

	virtual ~OctreeNode() = default;
};

struct OctreeBranch: OctreeNode {
	std::array<std::unique_ptr<OctreeNode>, 8> children{};

	~OctreeBranch() final = default;
};

template<typename T>
struct OctreeLeaf: OctreeNode {
	std::array<Vec<T>, 8> data{};

	~OctreeLeaf() final = default;
};

template<typename T, usize DEPTH>
struct SparseOctree {
	static_assert(DEPTH != 0, "Octree can't have a depth of 0");

	SparseOctree(Vec3<f32> origin, f32 len) : origin{ origin }, len{ len } {}

	Result<EmptyTuple, OctreeError> insert(Vec3<f32> pos, T val) {
		auto index = index_of(pos);
		if (index.is_some()) {
			auto idx = index.unwrap_unchecked();
			insert(idx, std::move(val));
			return ok<EmptyTuple, OctreeError>(EmptyTuple{});
		}
		else {
			return err<EmptyTuple, OctreeError>(OctreeError::OutOfBounds);
		}
	}

	Result<EmptyTuple, OctreeError> insert(Aabb<f32> bb, T val) {
		auto index = index_of_multiple(bb);
		if (index.is_some()) {
			auto idx = index.unwrap_unchecked();
			for (auto cur : idx) {
				insert(cur, val);
			}
			return ok<EmptyTuple, OctreeError>(EmptyTuple{});
		}
		else {
			return err<EmptyTuple, OctreeError>(OctreeError::OutOfBounds);
		}
	}

	Vec3<i32> min_incl() const {
		return Vec3<i32>{-(1 << DEPTH)};
	}

	Vec3<i32> min_excl() const {
		return Vec3<i32>{-(1 << DEPTH) - 1};
	}

	Vec3<i32> max_incl() const {
		return Vec3<i32>{(1 << DEPTH) - 1};
	}

	Vec3<i32> max_excl() const {
		return Vec3<i32>{1 << DEPTH};
	}

	const Vec<T>* at(Vec3<f32> pos) const {
		auto node_index_opt = index_of(pos);
		if (node_index_opt.is_none()) {
			return nullptr;
		}
		auto node_index = node_index_opt.unwrap_unchecked();
		return find(node_index);
	}

	Vec<T>* at(Vec3<f32> pos) {
		auto node_index_opt = index_of(pos);
		if (node_index_opt.is_none()) {
			return nullptr;
		}
		auto node_index = node_index_opt.unwrap_unchecked();
		return find(node_index);
	}

	template<typename F>
	Vec<const Vec<T>*> collect(F f) const {
		Vec<const Vec<T>*> collector{};
		collect_inner(f, &collector, this->_root.get(), DEPTH, this->origin, this->len);
		return collector;
	}
	template<typename F>
	Vec<Vec<T>*> collect(F f) {
		Vec<Vec<T>*> collector{};
		collect_inner(f, &collector, this->_root.get(), DEPTH, this->origin, this->len);
		return collector;
	}
private:
	template<typename F>
	void collect_inner(F f, Vec<const Vec<T>*>* collector, OctreeNode* cur_node, usize depth, Vec3<f32> origin, f32 len) const {
		if (!cur_node) {
			return;
		}
		Vec3<f32> origins[8];
		f32 new_len = len / 2;
		usize index = 0;
		for (usize z = 0; z <= 1; ++z) {
			for (usize y = 0; y <= 1; ++y) {
				for (usize x = 0; x <= 1; ++x) {
					Vec3<f32> offset = Vec3<f32>{
						(f32)x * 2 - 1,
						(f32)y * 2 - 1,
						(f32)z * 2 - 1,
					};
					origins[index++] = origin + offset * new_len;
				}
			}
		}
		if (depth != 0) {
			auto branch = dynamic_cast<OctreeBranch*>(cur_node);
			for (usize oct = 0; oct < 8; ++oct) {
				auto oct_origin = origins[oct];
				if (f(oct_origin, new_len)) {
					collect_inner(f, collector, branch->children[oct].get(), depth - 1, oct_origin, new_len); 
				}
			}
		}
		else {
			auto leaf = dynamic_cast<OctreeLeaf<T>*>(cur_node);
			for (usize oct = 0; oct < 8; ++oct) {
				if (f(origins[oct], new_len)) {
					collector->push(&leaf->data[oct]);
				}
			}
		}
	}
	

	template<typename F>
	void collect_inner(F f, Vec<Vec<T>*>* collector, OctreeNode* cur_node, usize depth, Vec3<f32> origin, f32 len) {
		if (!cur_node) {
			return;
		}
		Vec3<f32> origins[8];
		f32 new_len = len / 2;
		usize index = 0;
		for (usize z = 0; z <= 1; ++z) {
			for (usize y = 0; y <= 1; ++y) {
				for (usize x = 0; x <= 1; ++x) {
					Vec3<f32> offset = Vec3<f32>{
						(f32)x * 2 - 1,
						(f32)y * 2 - 1,
						(f32)z * 2 - 1,
					};
					origins[index++] = origin + offset * new_len;
				}
			}
		}
		if (depth != 0) {
			auto branch = dynamic_cast<OctreeBranch*>(cur_node);
			for (usize oct = 0; oct < 8; ++oct) {
				auto oct_origin = origins[oct];
				if (f(oct_origin, new_len)) {
					collect_inner(f, collector, branch->children[oct].get(), depth - 1, oct_origin, new_len);
				}
			}
		}
		else {
			auto leaf = dynamic_cast<OctreeLeaf<T>*>(cur_node);
			for (usize oct = 0; oct < 8; ++oct) {
				if (f(origins[oct], new_len)) {
					collector->push(&leaf->data[oct]);
				}
			}
		}
	}


	Vec<T>* find_or_create(u64 node_index) {
		OctreeNode* node = _root.get();
		for (usize depth = DEPTH; depth > 0; --depth) {
			auto shift = depth * 3;
			usize bits = (node_index >> shift) & 0b111;
			auto branch = dynamic_cast<OctreeBranch*>(node);
			if (auto child = branch->children[bits].get(); child != nullptr) {
				node = child;
			}
			else {
				auto ptr = std::unique_ptr<OctreeNode>{ depth == 1 ? (OctreeNode*)new OctreeLeaf<T> : (OctreeNode*)new OctreeBranch };
				node = ptr.get();
				branch->children[bits] = std::move(ptr);
			}
		}
		usize bits = node_index & 0b111;
		auto leaf = dynamic_cast<OctreeLeaf<T>*>(node);
		return &leaf->data[bits];
	}

	const Vec<T>* find(u64 node_index) const {
		const OctreeNode* node = _root.get();
		for (usize depth = DEPTH; depth > 0; --depth) {
			usize shift = depth * 3;
			usize bits = (node_index >> shift) & 0b111;
			auto branch = dynamic_cast<const OctreeBranch*>(node);
			node = branch->children[bits].get();
			if (node == nullptr) {
				return nullptr;
			}
		}
		usize bits = node_index & 0b111;
		auto leaf = dynamic_cast<const OctreeLeaf<T>*>(node);
		return &leaf->data[bits];
	}

	Vec<T>* find(u64 node_index) {
		OctreeNode* node = _root.get();
		for (usize depth = DEPTH; depth > 0; --depth) {
			usize shift = depth * 3;
			usize bits = (node_index >> shift) & 0b111;
			auto branch = dynamic_cast<OctreeBranch*>(node);
			node = branch->children[bits].get();
			if (node == nullptr) {
				return nullptr;
			}
		}
		usize bits = node_index & 0b111;
		auto leaf = dynamic_cast<OctreeLeaf<T>*>(node);
		return &leaf->data[bits];
	}

	Option<u64> index_of(Vec3<f32> pos) const {
		auto offset = pos - this->origin;
		auto len = this->len;
		bool out_of_range = false;
		auto absolute = offset.map<f32>([&](auto elem) {
			auto val = elem / len;
			out_of_range |= val < -1 || val > 1;
			return val;
		});
		if (out_of_range) {
			return none<u64>();
		}
		return some<u64>(index_of_recurse(DEPTH, absolute, 0));
	}

	u64 index_of_recurse(usize depth, Vec3<f32> pos, u64 result) const {
		auto x = (u64)(pos.x >= 0);
		auto y = (u64)(pos.y >= 0);
		auto z = (u64)(pos.z >= 0);
		u64 val = x | (y << 1) | (z << 2);
		auto offset = pos.map<f32>([](auto elem){
			return elem >= 0 ? 1.0f : -1.0f;
		});
		if(depth > 0){
			return index_of_recurse(depth - 1, pos * 2 - offset, (result << 3) | val);
		}
		else {
			return (result << 3) | val;
		}
	}

	Option<Vec<u64>> index_of_multiple(Aabb<f32> bb) const {
		auto len = this->len;
		bool out_of_range = false;
		auto ratio = [&](auto elem) {
			auto val = elem / len;
			out_of_range |= val < -1 || val > 1;
			return val;
		};
		auto absolute = Aabb<f32>{ bb.min.map<f32>(ratio), bb.max.map<f32>(ratio) };
		if (out_of_range) {
			return none<Vec<u64>>();
		}
		auto vec = Vec<u64>{};
		index_of_multiple_recurse(DEPTH, absolute, 0, vec);
		return some<Vec<u64>>(vec);
	}

	void index_of_multiple_recurse(usize depth, Aabb<f32> bb, u64 result, Vec<u64>& results) const {
		auto min_cmp = (bb.min/2 + 1).as<usize>();
		auto max_cmp = (bb.max/2 + 1).as<usize>();
		if (depth > 0) {
			for (usize x = min_cmp.x; x <= max_cmp.x; ++x) {
				for (usize y = min_cmp.y; y <= max_cmp.y; ++y) {
					for (usize z = min_cmp.z; z <= max_cmp.z; z++) {
						usize val = x | (y << 1) | (z << 2);
						auto offset = Vec3<f32>{ x,y,z }*2 - 1;
						auto new_bb = Aabb<f32>{ (bb.min*2  - offset).clamp(-1.0f, 1.0f), (bb.max * 2 - offset).clamp(-1.0f, 1.0f)};
						index_of_multiple_recurse(depth - 1, new_bb, (result << 3) | val, results);
					}
				}
			}
		}
		else {
			for (usize x = min_cmp.x; x <= max_cmp.x; ++x) {
				for (usize y = min_cmp.y; y <= max_cmp.y; ++y) {
					for (usize z = min_cmp.z; z <= max_cmp.z; z++) {
						usize val = x | (y << 1) | (z << 2);
						results.push((result << 3) | val);
					}
				}
			}
		}
	}

	void insert(u64 node_index, T val) {
		auto elem = find_or_create(node_index);
		if (elem != nullptr) {
			elem->push(val);
		}
	}

	std::unique_ptr<OctreeBranch> _root = std::unique_ptr<OctreeBranch>{ new OctreeBranch{} };
	Vec3<f32> origin{0,0,0};
	f32 len;
};