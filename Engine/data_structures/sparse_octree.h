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

	virtual ~OctreeNode() = 0;
};

struct OctreeBranch: OctreeNode {
	std::array<Option<Box<OctreeNode>>, 8> children;

	~OctreeBranch() final;
};

template<typename T>
struct OctreeLeaf: OctreeNode {
	std::array<T, 8> data{};

	~OctreeLeaf() final;
};

template<typename T, usize DEPTH>
struct SparseOctree {
	static_assert(DEPTH != 0, "Octree can't have a depth of 0");
	
	Result<EmptyTuple, OctreeError> insert(Vec3<i32> pos, T&& val) {
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


	Result<T, OctreeError> at(Vec3<i32> pos) const requires std::copyable<T> {
		auto node_index_opt = index_of(pos);
		if (node_index_opt.is_none()) {
			return err<T, OctreeError>(OctreeError::OutOfBounds);
		}
		auto node_index = node_index_opt.unwrap_unchecked();
		auto elem = find(node_index);
		if(elem.is_none()) {
			return OctreeError::EmptyNode;
		}
		T val = elem.unwrap();
		auto result = ok<T, OctreeError>(val);
		return result;
	}
private:
	Result<T*, OctreeError> find_or_create(u64 node_index) {
		OctreeNode* node = &_root.get();
		for (usize depth = DEPTH; depth > 0; --depth) {
			auto shift = depth * 3;
			auto bits = (node_index >> shift) & 0b111;
			auto branch = dynamic_cast<OctreeBranch*>(node);
			auto option = branch->children[bits].as_ptr();
			if (option.is_some()) {
				node = &option.unwrap_unchecked()->get();
			}
			else {
				auto box = Box<OctreeNode>::from_ptr(depth == 1 ? (OctreeNode*)new OctreeLeaf<T> : (OctreeNode*)new OctreeBranch);
				node = &box.get();
				branch->children[bits] = some<Box<OctreeNode>>(std::move(box));
			}
		}
		auto bits = node_index & 0b111;
		auto leaf = dynamic_cast<OctreeLeaf<T>*>(node);
		return ok<T*, OctreeError>(&leaf->data[bits]);
	}

	Option<T*> find(u64 node_index) const {
		const OctreeNode* node = &_root.get();
		for (usize depth = DEPTH; depth > 0; --depth) {
			auto shift = depth * 3;
			auto bits = (node_index >> shift) & 0b111;
			auto branch = dynamic_cast<const OctreeBranch*>(node);
			auto option = branch->children[bits].as_ptr();
			if (option.is_some()) {
				node = &option.unwrap_unchecked()->get();
			}
			else {
				return none<T>();
			}
		}
		auto bits = node_index & 0b111;
		auto leaf = dynamic_cast<const OctreeLeaf<T>*>(node);
		return some<T*>(&leaf->data[bits]);
	}

	Option<u64> index_of(Vec3<i32> pos) const {
		some<u64>(0);
	}

	void insert(u64 node_index, T&& val) {
		auto elem = find_or_create(node_index).unwrap();
		*elem = std::move(val);
	}

	Box<OctreeBranch> _root = Box<OctreeBranch>{};
	Vec3<u64> origin;
};