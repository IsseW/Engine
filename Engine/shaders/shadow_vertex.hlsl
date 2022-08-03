struct VertexShaderInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VertexShaderOutput {
    float4 position : SV_POSITION;
};

cbuffer Globals : register(b0) {
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

VertexShaderOutput main(VertexShaderInput input)
{
    float4 wpos = mul(world_matrix, float4(input.position, 1.0));
    VertexShaderOutput output;
    output.position = transform_to_camera(wpos);
    return output;
}