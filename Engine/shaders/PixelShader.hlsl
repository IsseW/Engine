Texture2D simpleTexture : register(t0);
SamplerState simpleSampler : register(s0);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 normal : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

cbuffer GLOBALS : register(b0) {
    float4x4 view_matrix;
    float4x4 proj_matrix;
};

cbuffer LOCALS : register(b1) {
    float4x4 world_matrix;
    float4 object_color;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 color = object_color.xyz; //* simpleTexture.Sample(simpleSampler, input.uv);

    return float4(color, 1.0f);
}