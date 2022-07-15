#pragma once
#include <rstd/primitives.h>
#include <math/aab.h>
#include <rstd/panic.h>

template<usize DEPTH>
struct SparseOctree {
	static_assert(DEPTH != 0, "Octree can't have a depth of 0");
	SparseOctree(Aabb<f32> dimension) : __inner_dimension{ dimension }, __len{ 0 }, __flat_array{0} {}
	
	bool insert_absolute(Aabb<f32> abs, u32 index) {
		insert(this->__inner_dimension, abs, DEPTH, index);
		bool contained = this->__inner_dimension.intersects(abs);
		if (contained){
			auto aabbs = SparseOctree<DEPTH>::split_aabb(this->__inner_dimension);
			for (auto& aabb : aabbs) {
				insert(aabb, abs, DEPTH - 1, index);
			}
		}
		return contained;
	}

	bool insert_relative(Aabb<f32> rel, u32 index) {
		return false;
	}

	bool remove_by_index(u32 index) {
		return false;
	}
private:
	void insert(Aabb<f32>& dimensions, Aabb<f32>& collision, usize depth, u32 index) {
		if (dimensions.intersects(collision)) {
			if (depth == 0) {
				int x = 0;
			}
			else {
				auto aabbs = SparseOctree<DEPTH>::split_aabb(dimensions);
				for (auto& aabb : aabbs) {
					insert(aabb, collision, depth - 1, index);
				}
			}
		}
	}

	static std::array<Aabb<f32>, 8> split_aabb(const Aabb<f32>& aabb) {	
		auto offset = (aabb.max - aabb.min) / 2;
		auto aabb_b_d_l = Aabb<f32>{ aabb.min, aabb.min + offset };
		auto offset_x = Vec3<f32>{ offset.x, 0, 0 };
		auto aabb_b_d_r = Aabb<f32>{ aabb_b_d_l.min + offset_x, aabb_b_d_l.max + offset_x };
		auto offset_y = Vec3<f32>{ 0, offset.y, 0 };
		auto aabb_b_u_l = Aabb<f32>{ aabb_b_d_l.min + offset_y, aabb_b_d_l.max + offset_y };
		auto aabb_b_u_r = Aabb<f32>{ aabb_b_d_r.min + offset_y, aabb_b_d_r.max + offset_y };
		auto offset_z = Vec3<f32>{ 0, 0, offset.z };
		auto aabb_a_d_l = Aabb<f32>{ aabb_b_d_l.min + offset_z, aabb_b_d_l.max + offset_z };
		auto aabb_a_d_r = Aabb<f32>{ aabb_b_d_r.min + offset_z, aabb_b_d_r.max + offset_z };
		auto aabb_a_u_l = Aabb<f32>{ aabb_b_u_l.min + offset_z, aabb_b_u_l.max + offset_z };
		auto aabb_a_u_r = Aabb<f32>{ aabb_b_u_r.min + offset_z, aabb_b_u_r.max + offset_z };
		return { aabb_b_d_l, aabb_b_d_r, aabb_b_u_l, aabb_b_u_r, aabb_a_d_l, aabb_a_d_r, aabb_a_u_l, aabb_a_u_r };
	}

	Aabb<f32> __inner_dimension;
	usize __len;
	std::array<u32, DEPTH> __flat_array;
};