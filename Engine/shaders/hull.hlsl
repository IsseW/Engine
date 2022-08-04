

cbuffer Globals : register(b0) {
    float4x4 view_matrix;
    float4x4 proj_matrix;
    float3 cam_pos;
};


struct HullInput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 wpos : TEXCOORD1;
};

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

float round_to(float val, float rounding) {
    return round(val / rounding) * rounding;
}

ConstantOutput patch_constant_function(const OutputPatch<HullOutput, 3> input_patch, uint patch_id : SV_PrimitiveID) {
    ConstantOutput output;

    float3 patch_zero_d = 1.0 / distance(input_patch[0].wpos.xyz, cam_pos);
    float3 patch_one_d = 1.0 / distance(input_patch[1].wpos.xyz, cam_pos);
    float3 patch_two_d = 1.0 / distance(input_patch[2].wpos.xyz, cam_pos);

    float t;
    float inside = 0.0;

    t = (patch_zero_d + patch_one_d) / 2.0;
    t = round_to(t, 0.01);
    output.edges[0] = t;
    inside += t;

    t = (patch_one_d + patch_two_d) / 2.0;
    t = round_to(t, 0.01);
    output.edges[1] = t;
    inside += t;

    t = (patch_two_d + patch_zero_d) / 2.0;
    t = round_to(t, 0.01);
    output.edges[2] = t;
    inside += t;

    inside /= 3.0;
    inside = round_to(inside, 0.01);

    output.inside = inside * 12.0;

    return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patch_constant_function")]
HullOutput main(InputPatch<HullInput, 3> patch, uint point_id : SV_OutputControlPointID, uint patch_id : SV_PrimitiveID) {
    HullOutput output;
    
    output.wpos = patch[point_id].wpos;
    output.position = patch[point_id].position;
    output.uv = patch[point_id].uv;
    output.normal = patch[point_id].normal;

    return output;
}