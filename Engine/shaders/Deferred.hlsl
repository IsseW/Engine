Texture2D albedo : register(t0);
Texture2D normal : register(t1);
Texture2D position : register(t2);
Texture2D light_info : register(t3);

Texture2D depth : register(t4);

Texture2DArray dir_shadows : register(t5);
Texture2DArray spot_shadows : register(t6);

SamplerState shadow_sampler : register(s0);

struct DirLight {
	float4x4 texture_mat;
	float3 pos;
	float3 dir;
	float3 color;
	float strength;
};

StructuredBuffer<DirLight> dir_lights : register(t7);

struct SpotLight {
	float4x4 texture_mat;
	float3 pos;
	float3 dir;
	float3 color;
	float strength;
};

StructuredBuffer<SpotLight> spot_lights : register(t8);

cbuffer Locals : register(b0) {
	float3 cam_pos;
	uint render_mode;
	uint num_dir;
	uint num_spot;
}

RWTexture2D<float4> output : register(u0);

void deferred(uint2 pos) {
	if (position[pos].w == 0.0) {
		output[pos] = albedo[pos];
	}
	else {
		int dir_light_count = 0;

		float3 p = position[pos].xyz;
		float3 n = normal[pos].xyz;

		float3 view_dir = normalize(cam_pos - p);

		float ambient = light_info[pos].x;
		float diffuse_c = light_info[pos].y;
		float specular_c = light_info[pos].z;
		float shinyness = light_info[pos].w;

		float3 diffuse;
		float3 specular;

		for (uint i = 0; i < num_dir; i++) {
			DirLight light = dir_lights[i];

			float4 shadow_uv = mul(light.texture_mat, float4(p, 1.0));
			shadow_uv /= shadow_uv.w;
			float shadow = dir_shadows.SampleLevel(shadow_sampler, float3(shadow_uv.xy, (float)i), 0, 0).x;

			float d = pow(1.0 - shadow, 0.5) * 2.0;
			output[pos] = float4(d, d, d, 1.0);

			diffuse += saturate(dot(n, -light.dir)) * light.color;

			// Using Blinn half angle modification for performance over correctness
			// TODO: Fix
			float3 h = normalize(view_dir - light.dir);
			specular += pow(saturate(dot(h, n)), 4.0) * light.color;
		}

		float3 lighting = ambient + diffuse * diffuse_c + specular * specular_c;
		float4 color = albedo[pos];
		output[pos] =  float4(color.xyz * lighting, color.w);
	}
}

// Redner modes
#define ALBEDO 1
#define DEPTH 2
#define NORMAL 3
#define POSITION 4

[numthreads(8, 8, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID) {
	uint2 pos = dispatch_id.xy;

	switch (render_mode) {
	case ALBEDO: {
		output[pos] = albedo[pos];
		break;
	}
	case DEPTH: {
		float d = pow((1.0 - depth[pos]).x, 0.5) * 2.0;
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