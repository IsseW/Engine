

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

float calculate_lod(float3 wpos_0, float3 wpos_1) {

    float diameter = distance(wpos_0, wpos_1);
    float radius = diameter / 2.0;

    float3 radius_pos = (wpos_0 + wpos_1) / 2.0;
    radius_pos = mul(view_matrix, float4(radius_pos, 1.0)).xyz;

    float3 cam_dir = normalize(radius_pos - cam_pos);

    float3 sup_;
    float3 cam_dir_abs = abs(cam_dir);

    if (cam_dir_abs.y < cam_dir_abs.x && cam_dir_abs.z < cam_dir_abs.y) {
        sup_ = float3(-cam_dir.z, 0.0, cam_dir.x);
    }
    else if (cam_dir_abs.z < cam_dir_abs.x && cam_dir_abs.z < cam_dir_abs.y) {
        sup_ = float3(-cam_dir.y, cam_dir.x, 0.0);
    }
    else {
        sup_ = float3(0.0, -cam_dir.z, cam_dir.y);
    }

    float3 sup = normalize(sup_);
    float3 sdown = radius_pos - sup * radius;
    sup = radius_pos + sup * radius;

    float3 projection_up = mul(proj_matrix, float4(sup, 1.0)).xyz;
    float3 projection_down = mul(proj_matrix, float4(sup, 1.0)).xyz;

    float proj_sphere_diameter = distance(projection_up.xy, projection_down.xy);

    float tessellated_tri_size = 12.0;

    return proj_sphere_diameter * tessellated_tri_size;
}

ConstantOutput patch_constant_function(const OutputPatch<HullOutput, 3> input_patch, uint patch_id : SV_PrimitiveID) {
    ConstantOutput output;

    /*
    float3 patch_zero = input_patch[0].wpos.xyz;
    float3 patch_one = input_patch[1].wpos.xyz;
    float3 patch_two = input_patch[2].wpos.xyz;

    float t;
    float inside = 0.0;

    t = calculate_lod(patch_zero, patch_one);
    t = round_to(t, 0.01);
    output.edges[0] = t;
    inside += t;

    t = calculate_lod(patch_one, patch_two);
    t = round_to(t, 0.01);
    output.edges[1] = t;
    inside += t;

    t = calculate_lod(patch_two, patch_zero);
    t = round_to(t, 0.01);
    output.edges[2] = t;
    inside += t;

    inside /= 3.0;
    inside = round_to(inside, 0.01);

    output.inside = inside;
    */

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