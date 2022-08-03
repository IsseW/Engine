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
		auto index = index_of(pos.map<i32>([&](auto val) {
			return (i32)val;
		}));
		if (index.is_some()) {
			auto idx = index.unwrap_unchecked();
			insert(idx, std::move(val));
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

	Option<u64> index_of(Vec3<i32> pos) const {
		auto offset = pos - this->origin;
		f32 len = (f32)this->len;
		auto absolute = offset.map<f32>([&](auto elem) {
			return (f32)elem / len;
		});
		return some<u64>(index_of_recurse(DEPTH, absolute, 0));
	}

	u64 index_of_recurse(usize depth, Vec3<f32> pos, u64 result) const {
		u64 val = 0;
		auto x = pos.x >= 0;
		auto y = pos.y >= 0;
		auto z = pos.z >= 0;
		auto index= (u32)x | ((u32)y << 1) | ((u32)z << 2);
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

	void insert(u64 node_index, T&& val) {
		auto elem = find_or_create(node_index).unwrap();
		*elem = std::move(val);
	}

	std::unique_ptr<OctreeBranch> _root = std::unique_ptr<OctreeBranch>{ new OctreeBranch{} };
	Vec3<i32> origin{0,0,0};
	u32 len;
};