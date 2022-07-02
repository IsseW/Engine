struct VertexShaderInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
};

cbuffer GLOBALS : register(b0) {
    float4x4 view_matrix;
    float4x4 proj_matrix;
};

cbuffer LOCALS : register(b1) {
    float4x4 world_matrix;
    float4 object_color;
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
    VertexShaderOutput output;
    output.position = transform(float4(input.position, 1.0));
    output.normal = normalize(transform(float4(input.normal, 0.0)).xyz);
    output.uv = input.uv;
    return output;
}