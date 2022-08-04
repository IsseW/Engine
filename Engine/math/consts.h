#pragma once
#include<rstd/primitives.h>
#include<algorithm>

namespace F32 {
	const f32 PI = 3.141592653589793238462643f;
	const f32 TAO = PI * 2.0f;
	const f32 TO_RAD = PI / 180.0f;
	const f32 TO_DEG = 180.0f / PI;
	const f32 INF = std::numeric_limits<f32>::infinity();
	const f32 NEG_INF = -INF;
}
namespace F64 {
	const f64 PI = 3.141592653589793238462643;
	const f64 TAO = PI * 2.0;
	const f64 TO_RAD = PI / 180.0;
	const f64 TO_DEG = 180.0 / PI;
	const f64 INF = std::numeric_limits<f64>::infinity();
	const f64 NEG_INF = -INF;
}