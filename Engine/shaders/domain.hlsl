

struct ConstantOutput {
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

struct HullOutput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 wpos : TEXCOORD1;
};

struct DomainOutput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 wpos : TEXCOORD1;
};



[domain("tri")]
DomainOutput main(ConstantOutput input, float3 uvw: SV_DomainLocation, const OutputPatch<HullOutput, 3> patch) {
	DomainOutput output;

	output.position = uvw.x * patch[0].position + uvw.y * patch[1].position + uvw.z * patch[2].position;
	output.uv = uvw.x * patch[0].uv + uvw.y * patch[1].uv + uvw.z * patch[2].uv;
	output.normal = uvw.x * patch[0].normal + uvw.y * patch[1].normal + uvw.z * patch[2].normal;
	output.wpos = uvw.x * patch[0].wpos + uvw.y * patch[1].wpos + uvw.z * patch[2].wpos;

	return output;
}