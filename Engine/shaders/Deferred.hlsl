
struct Light {
	float4x4 view_mat;
	float4x4 proj_mat;
	float3 pos;
	float3 dir;
	float3 color;
	uint light_type;
	float cutoff;
};

Texture2D ambient_map : register(t0);
Texture2D diffuse_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal : register(t3);
Texture2D position : register(t4);
Texture2D depth : register(t5);
Texture2DArray shadows : register(t6);
SamplerState shadow_sampler : register(s0);
StructuredBuffer<Light> lights : register(t7);

cbuffer ObjectData : register(b0) {
	float3 cam_pos;
	uint render_mode;
	uint num_lights;
}

RWTexture2D<float4> output : register(u0);

#define DIR_LIGHT 0
#define SPOT_LIGHT 1

void deferred(uint2 pos) {
	if (position[pos].w == 0.0) {
		output[pos] = float4(0.2, 0.2, 0.2, 1.0);
	}
	else {

		float3 p = position[pos].xyz;
		float3 n = normal[pos].xyz;

		float3 view_dir = normalize(cam_pos - p);

		float3 ambient_c = ambient_map[pos].xyz;
		float3 diffuse_c = diffuse_map[pos].xyz;
		float3 specular_c = specular_map[pos].xyz;
		float shinyness = specular_map[pos].w;

		float3 diffuse;
		float3 specular;

		for (uint i = 0; i < num_lights; i++) {

			float bias = 0.001f;

			Light light = lights[i];

			float4 shadow_uv = mul(light.proj_mat, mul(light.view_mat, float4(p, 1.0)));
			shadow_uv /= shadow_uv.w;
			float shadow = shadows.SampleLevel(
				shadow_sampler,
				float3(0.5 + shadow_uv.xy * float2(0.5, -0.5), (float)i),
				0, 0
			).x + bias;

			float d = pow((abs(shadow_uv.z - shadow)), 0.5) * 2.0;
			// output[pos] = float4(d, d, d, 1.0);

			if (shadow_uv.z < shadow) {
				if (light.light_type == DIR_LIGHT) {
					diffuse += saturate(dot(n, -light.dir)) * light.color;
					// Using Blinn half angle modification for performance over correctness
					// TODO: Fix maybe
					float3 h = normalize(view_dir - light.dir);
					specular += pow(saturate(dot(h, n)), shinyness) * light.color;
				}
				else if (light.light_type == SPOT_LIGHT) {
					float3 ldir = normalize(p - light.pos);
					if (dot(light.dir, ldir) > light.cutoff) {
						diffuse += saturate(dot(n, -ldir)) * light.color;

						float3 h = normalize(view_dir - ldir);
						specular += pow(saturate(dot(h, n)), shinyness) * light.color;
					}
				}
			}
		}

		output[pos] =  float4(ambient_c + diffuse * diffuse_c + specular * specular_c, 1.0);
	}
}

// Redner modes
#define AMBIENT 1
#define DIFFUSE 2
#define SPECULAR 3
#define DEPTH 4
#define NORMAL 5
#define POSITION 6

[numthreads(8, 8, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID) {
	uint2 pos = dispatch_id.xy;

	switch (render_mode) {
	case AMBIENT: {
		output[pos] = ambient_map[pos];
		break;
	}
	case DIFFUSE: {
		output[pos] = diffuse_map[pos];
		break;
	}
	case SPECULAR: {
		output[pos] = specular_map[pos];
		break;
	}
	case DEPTH: {
		float d = pow((1.0 - depth[pos].x), 0.5) * 2.0;
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