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
	std::array<Option<Box<OctreeNode>>, 8> nodes;
	
	~OctreeBranch() final;
};

template<typename T>
struct OctreeLeaf: OctreeNode {
	std::array<T, 8> data;

	~OctreeLeaf() final;
};

template<typename T, usize DEPTH>
struct SparseOctree {
	static_assert(DEPTH != 0, "Octree can't have a depth of 0");
	
	Result<EmptyTuple, OctreeError> insert(Vec3<i32> pos, T&& val) {
		ensure_space(pos);
		auto index = index_of(pos);
		insert(index, std::move(val));
	}

	Tuple<Vec3<i32>, Vec3<i32>> min_bounds() const {
		TODO;
	}

	Tuple<Vec3<i32>, Vec3<i32>> max_bounds() const {
		TODO;
	}

	Result<T*, OctreeError> at(Vec3<i32> pos) {
		u32 lim = bounds_test(pos);
		if (lim == 0) {
			return err<T*, OctreeError>(OctreeError::OutOfBounds);
		}
		auto node_index = index_of(pos);
		auto elem = find(node_index);
		if (elem.is_none()) {
			return OctreeError::EmptyNode;
		}
		auto result = ok<T*, OctreeError>(elem.unwrap());
		return result;
	}

	Result<const T*, OctreeError> at(Vec3<i32> pos) const {
		u32 lim = bounds_test(pos);
		if(lim == 0){
			return err<const T*, OctreeError>(OctreeError::OutOfBounds);
		}
		auto node_index = index_of(pos);
		auto elem = find(node_index);
		if(elem.is_none()) {
			return OctreeError::EmptyNode;
		}
		const auto val = elem.unwrap();
		auto result = ok<const T*, OctreeError>(val);
		return result;
	}
private:
	T* find_or_create(u64 node_index) {}

	Option<T*> find(u64 node_index) const {}

	u64 index_of(Vec3<i32> pos) const {}

	void ensure_space(Vec3<i32> pos) {}

	void insert(u64 node_index, T&& val) {}

	void grow(u32 lim) {}

	void grow_once() {}

	u32 bounds_test(Vec3<i32> v) const{}

	//Box<OctreeBranch> _root = Box<OctreeBranch>{};
};