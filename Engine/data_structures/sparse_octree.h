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
	std::array<T, 8> data{};

	~OctreeLeaf() final = default;
};

template<typename T, usize DEPTH>
struct SparseOctree {
	static_assert(DEPTH != 0, "Octree can't have a depth of 0");
	
	Result<EmptyTuple, OctreeError> insert(Vec3<f32> pos, T&& val) {
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

	Result<EmptyTuple, OctreeError> insert(Aabb<f32> bb, T&& val) {
		auto index = index_of_multiple(bb);
		if (index.is_some()) {
			auto idx = index.unwrap_unchecked();
			for (auto cur : idx) {
				insert(cur, std::move(val));
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


	Option<T*> at(Vec3<i32> pos) const {
		auto node_index_opt = index_of(pos);
		if (node_index_opt.is_none()) {
			return none<T, OctreeError>();
		}
		auto node_index = node_index_opt.unwrap_unchecked();
		return find(node_index);
	}
private:
	Result<T*, OctreeError> find_or_create(u64 node_index) {
		OctreeNode* node = _root.get();
		for (usize depth = DEPTH; depth > 0; --depth) {
			auto shift = depth * 3;
			usize bits = (node_index >> shift) & 0b111;
			auto branch = dynamic_cast<OctreeBranch*>(node);
			auto child = branch->children[bits].get();
			if (child != nullptr) {
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
		return ok<T*, OctreeError>(&leaf->data[bits]);
	}

	Option<T*> find(u64 node_index) const {
		const OctreeNode* node = _root.get();
		for (usize depth = DEPTH; depth > 0; --depth) {
			auto shift = depth * 3;
			auto bits = (node_index >> shift) & 0b111;
			auto branch = dynamic_cast<const OctreeBranch*>(node);
			node = branch->children[bits].get();
			if (node == nullptr) {
				return none<T>();
			}
		}
		auto bits = node_index & 0b111;
		auto leaf = dynamic_cast<const OctreeLeaf<T>*>(node);
		return some<T*>(&leaf->data[bits]);
	}

	Option<u64> index_of(Vec3<f32> pos) const {
		auto offset = pos - this->origin;
		auto len = this->len;
		auto absolute = offset.map<f32>([&](auto elem) {
			return elem / len;
		});
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
		auto ratio = [&](auto elem) {
			return elem / len;
		};
		auto absolute = Aabb<f32>{ bb.min.map<f32>(ratio), bb.max.map<f32>(ratio) };
		auto vec = Vec<u64>{};
		index_of_multiple_recurse(DEPTH, absolute, 0, vec);
		return some<Vec<u64>>(vec);
	}

	void index_of_multiple_recurse(usize depth, Aabb<f32> bb, u64 result, Vec<u64>& results) const {
		u64 val = 0;
		if (bb.min == Vec3<f32>{0, 0, 0} || bb.max == Vec3<f32>{1.0f,1.0f,1.0f}) {
			u64 upper = result << (depth + 1);
			for (u64 index = 0; index < upper; ++index) {
				results.push(upper | index);
			}
			return;
		}
		for (usize oct = 0; oct < 8; ++oct) {
			auto offset_max = bb.max.map<f32>([](auto elem) {
				return elem >= 0 ? 1.0f : -1.0f;
			});
			auto offset_min = bb.min.map<f32>([](auto elem) {
				return elem >= 0 ? 1.0f : -1.0f;
			});
			if(depth > 0){
				auto new_bb = Aabb<f32>{
					bb.max * 2 - offset_max,
					bb.min * 2 - offset_min
				};
				index_of_multiple_recurse(depth - 1, new_bb, (result << 3) | val, results);
			}
			else {

			}
		}
	}

	void insert(u64 node_index, T&& val) {
		auto elem = find_or_create(node_index).unwrap();
		*elem = std::move(val);
	}

	std::unique_ptr<OctreeBranch> _root = std::unique_ptr<OctreeBranch>{ new OctreeBranch{} };
	Vec3<f32> origin{0,0,0};
	f32 len;
};