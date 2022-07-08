Texture2D albedo : register(t0);
Texture2D normal : register(t1);
Texture2D position : register(t2);
Texture2D depth : register(t3);

#define ALBEDO 1
#define DEPTH 2
#define NORMAL 3
#define POSITION 4

cbuffer locals : register(b0) {
	uint render_mode;
}

RWTexture2D<float4> output : register(u0);

void deferred(uint2 pos) {
	if (position[pos].w == 0.0) {
		output[pos] = albedo[pos];
	}
	else {
		output[pos] = albedo[pos];
	}
}

[numthreads(8, 8, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID) {
	uint2 pos = dispatch_id.xy;

	switch (render_mode) {
	case ALBEDO: {
		output[pos] = albedo[pos];
		break;
	}
	case DEPTH: {
		float d = (1.0 - depth[pos]).x;
		output[pos] = float4(d, d, d, 1.0);
		break;
	}
	case NORMAL: {
		output[pos] = normal[pos];
		break;
	}
	case POSITION: {
		output[pos] = position[pos];
		break;
	}
	default:
		deferred(pos);
		break;
	}
}