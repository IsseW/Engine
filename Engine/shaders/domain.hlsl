

cbuffer Globals : register(b0) {
	float4x4 view_matrix;
	float4x4 proj_matrix;
	float3 cam_pos;
};

cbuffer LOCALS : register(b1) {
	float4x4 world_matrix;
};

struct ConstantOutput {
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

struct HullOutput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

struct DomainOutput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float3 wpos : TEXCOORD1;
	float3 obj_pos : TEXCOORD2;
};


[domain("tri")]
DomainOutput main(ConstantOutput input, float3 uvw: SV_DomainLocation, const OutputPatch<HullOutput, 3> patch) {
	DomainOutput output;

	float4 p = uvw.x * patch[0].position + uvw.y * patch[1].position + uvw.z * patch[2].position;

	float4 t0 = p - float4(patch[0].normal * dot((p - patch[0].position).xyz, patch[0].normal), 0.0);
	float4 t1 = p - float4(patch[1].normal * dot((p - patch[1].position).xyz, patch[1].normal), 0.0);
	float4 t2 = p - float4(patch[2].normal * dot((p - patch[2].position).xyz, patch[2].normal), 0.0);

	float4 tp = uvw.x * t0 + uvw.y * t1 + uvw.z * t2;
	float a = 0.75;
	float4 position = (1.0 - a) * p + a * tp;
	output.obj_pos = position.xyz;
	float4 wpos = mul(world_matrix, position);

	output.wpos = wpos.xyz;

	output.position = mul(proj_matrix, mul(view_matrix, wpos));

	float2 uv = uvw.x * patch[0].uv + uvw.y * patch[1].uv + uvw.z * patch[2].uv;
	output.uv = uv;
	float3 normal = uvw.x * patch[0].normal + uvw.y * patch[1].normal + uvw.z * patch[2].normal;
	output.normal = normalize(mul(world_matrix, float4(normal, 0.0)).xyz);

	return output;
}