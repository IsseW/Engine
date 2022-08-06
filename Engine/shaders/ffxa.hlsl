
static const float EDGE_THRESHOLD_MIN = 0.0312;
static const float EDGE_THRESHOLD_MAX = 0.125;
static const float SUBPIXEL_QUALITY = 0.75;

static const uint ITERATIONS = 12;
static const float QUALITY[12] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };

RWTexture2DArray<unorm float4> output : register(u0);

float4 sample_at(float2 pos) {
	float4 c00 = output[uint3((uint)floor(pos.x), (uint)floor(pos.y), 0)];
	float4 c01 = output[uint3((uint)ceil(pos.x), (uint)floor(pos.y), 0)];
	float4 c10 = output[uint3((uint)floor(pos.x), (uint)ceil(pos.y), 0)];
	float4 c11 = output[uint3((uint)ceil(pos.x), (uint)ceil(pos.y), 0)];
	return lerp(lerp(c00, c01, frac(pos.x)), lerp(c10, c11, frac(pos.x)), frac(pos.y));
}

float rgb2luma(float3 rgb) {
	return sqrt(dot(rgb, float3(0.299, 0.587, 0.114)));
}

[numthreads(8, 8, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID) {
	uint3 id = dispatch_id.xyz;
	float4 color_center = output[id];

	float luma_center = rgb2luma(color_center.xyz);

	float luma_down = rgb2luma(output[id - uint3(0, 1, 0)].xyz);
	float luma_up = rgb2luma(output[id + uint3(0, 1, 0)].xyz);
	float luma_left = rgb2luma(output[id - uint3(1, 0, 0)].xyz);
	float luma_right = rgb2luma(output[id + uint3(1, 0, 0)].xyz);

	float luma_min = min(luma_center, min(min(luma_down, luma_up), min(luma_left, luma_right)));
	float luma_max = max(luma_center, max(max(luma_down, luma_up), max(luma_left, luma_right)));

	float luma_range = luma_max - luma_min;

	float4 color = float4(0.0, 0.0, 0.0, 1.0);
	if (luma_range < max(EDGE_THRESHOLD_MIN, luma_max * EDGE_THRESHOLD_MAX)) {
		color = color_center;
	}
	else {
		float luma_down_left = rgb2luma(output[id - uint3(1, 1, 0)].xyz);
		float luma_up_right = rgb2luma(output[id + uint3(1, 1, 0)].xyz);
		float luma_up_left = rgb2luma(output[id + uint3(-1, 1, 0)].xyz);
		float luma_down_right = rgb2luma(output[id + uint3(1, -1, 0)].xyz);

		float luma_down_up = luma_down + luma_up;
		float luma_left_right = luma_left + luma_right;

		float luma_left_corners = luma_down_left + luma_up_left;
		float luma_down_corners = luma_down_left + luma_down_right;
		float luma_right_corners = luma_down_right + luma_up_right;
		float luma_up_corners = luma_up_left + luma_up_right;

		float edge_horizontal = abs(luma_left_corners - 2.0 * luma_left) +
								2.0 * abs(luma_down_up - 2.0 * luma_center) +
								abs(luma_right_corners - 2.0 * luma_right);

		float edge_vertical =	abs(luma_up_corners - 2.0 * luma_up) +
								2.0 * abs(luma_left_right - 2.0 * luma_center) +
								abs(luma_down_corners - 2.0 * luma_down);

		bool is_horizontal = (edge_horizontal >= edge_vertical);

		float luma1 = is_horizontal ? luma_down : luma_left;
		float luma2 = is_horizontal ? luma_up : luma_right;

		float gradient1 = luma1 - luma_center;
		float gradient2 = luma2 - luma_center;

		bool is1steepest = abs(gradient1) >= abs(gradient2);

		float gradient_scaled = 0.25 * max(abs(gradient1), abs(gradient2));

		// float step_length = is_horizontal ? inverse_screen_size.y : inverse_screen_size.x;

		float luma_local_average = 0.5 * (is1steepest ? luma1 : luma2) + luma_center;
		float step = is1steepest ? -1.0 : 1.0;
		float2 in_pos = (float2)id;
		float2 cur_pos = in_pos;
		if (is_horizontal) {
			cur_pos.x += step * 0.5;
		}
		else {
			cur_pos.y += step * 0.5;
		}

		float2 offset = is_horizontal ? float2(1.0, 0.0) : float2(0.0, 1.0);

		float2 pos1 = cur_pos - offset;
		float2 pos2 = cur_pos + offset;

		float luma_end1 = rgb2luma(sample_at(pos1).xyz) - luma_local_average;
		float luma_end2 = rgb2luma(sample_at(pos2).xyz) - luma_local_average;

		bool reached1 = abs(luma_end1) >= gradient_scaled;
		bool reached2 = abs(luma_end2) >= gradient_scaled;
		bool reached_both = reached1 && reached2;

		if (!reached1) {
			pos1 -= offset;
		}
		if (!reached2) {
			pos2 += offset;
		}

		if (!reached_both) {

			for (uint i = 2; i < ITERATIONS; i++) {
				if (!reached1) {
					luma_end1 = rgb2luma(sample_at(pos1).xyz) - luma_local_average;
				}
				if (!reached2) {
					luma_end2 = rgb2luma(sample_at(pos2).xyz) - luma_local_average;
				}

				reached1 = abs(luma_end1) >= gradient_scaled;
				reached2 = abs(luma_end2) >= gradient_scaled;
				reached_both = reached1 && reached2;


				if (!reached1) {
					pos1 -= offset * QUALITY[i];
				}
				if (!reached2) {
					pos2 += offset * QUALITY[i];
				}

				if (reached_both) { break; }
			}
		}

		float distance1 = is_horizontal ? (in_pos.x - pos1.x) : (in_pos.y - pos1.y);
		float distance2 = is_horizontal ? (pos2.x - in_pos.x) : (pos2.y - in_pos.y);

		bool is_direction1 = distance1 < distance2;
		float distance_final = min(distance1, distance2);

		float edge_thickness = (distance1 + distance2);

		float pixel_offset = -distance_final / edge_thickness + 0.5;

		bool is_luma_center_smaller = luma_center < luma_local_average;

		bool correct_variation = ((is_direction1 ? luma_end1 : luma_end2) < 0.0) != is_luma_center_smaller;

		float final_offset = correct_variation ? pixel_offset : 0.0;

		// Subpixel aliasing
		float luma_average = (1.0 / 12.0) * (2.0 * (luma_down_up + luma_left_right) + luma_left_corners + luma_right_corners);

		float sub_pixel_offset1 = clamp(abs(luma_average - luma_center) / luma_range, 0.0, 1.0);
		float sub_pixel_offset2 = (3.0 - 2.0 * sub_pixel_offset1) * sub_pixel_offset1 * sub_pixel_offset1;

		float sub_pixel_offset_final = sub_pixel_offset2 * sub_pixel_offset2 * SUBPIXEL_QUALITY;

		final_offset = max(final_offset, sub_pixel_offset_final);

		float2 final_pos = in_pos;
		if (is_horizontal) {
			final_pos.y += final_offset * step;
		}
		else {
			final_pos.x += final_offset * step;
		}
		float4 final = sample_at(final_pos);
		color = final;//float4(saturate(0.5 + in_pos - final_pos), 0.0, 1.0);
	}
	GroupMemoryBarrier();
	output[id] = color;
}