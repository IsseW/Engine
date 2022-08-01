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
    float4x4 texture_matrix;
};

cbuffer LOCALS : register(b1) {
    float4x4 world_matrix;
};


VertexShaderOutput main(VertexShaderInput input)
{
    float4 wpos = mul(world_matrix, float4(input.position, 1.0));
    VertexShaderOutput output;
    output.position = mul(texture_matrix, wpos);
    return output;
}