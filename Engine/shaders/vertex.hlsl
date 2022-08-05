struct VertexShaderInput {
    float3 position : POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct VertexShaderOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 wpos : TEXCOORD1;
    float3 obj_pos : TEXCOORD2;
};

cbuffer GLOBALS : register(b0) {
    float4x4 view_matrix;
    float4x4 proj_matrix;
    float3 cam_pos;
};

cbuffer LOCALS : register(b1) {
    float4x4 world_matrix;
};

float4 transform_to_camera(float4 vec) {
    float4 view = mul(view_matrix, vec);
    float4 projected = mul(proj_matrix, view);
    return projected;
}

float4 transform(float4 vec) {
    float4 world = mul(world_matrix, vec);
    return transform_to_camera(world);
}

VertexShaderOutput main(VertexShaderInput input)
{
    float4 wpos = mul(world_matrix, float4(input.position, 1.0));
    VertexShaderOutput output;
    output.obj_pos = input.position.xyz;
    output.wpos = wpos.xyz;
    output.position = transform_to_camera(wpos);
    output.uv = input.uv;
    output.normal = normalize(mul(world_matrix, float4(input.normal, 0.0)).xyz);

    return output;
}