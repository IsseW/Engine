#pragma once
#include <rstd/primitives.h>
#include <math/aab.h>
#include <rstd/panic.h>

template<usize DEPTH>
struct SparseOctree {
	SparseOctree(Aabb<f32> dimension) : __inner_dimension{ dimension }, __len{0} {}
	
	void insert_absolute(Aabb<f32> abs, u32 index) {

	}

	void insert_relative(Aabb<f32> rel, u32 index) {

	}

	void remove_by_index(u32 index) {
		
	}


private:
	Aabb<f32> __inner_dimension;
	usize __len;
	u32 __flat_array[DEPTH];
};