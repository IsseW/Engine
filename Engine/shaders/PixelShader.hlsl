Texture2D tex : register(t0);
SamplerState tex_sampler : register(s0);

struct PixelShaderInput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 wpos : TEXCOORD2;
};

struct PixelShaderOutput {
    float4 albedo;
    float4 normal;
    float4 position;
};

cbuffer GLOBALS : register(b0) {
    float4x4 view_matrix;
    float4x4 proj_matrix;
};

cbuffer LOCALS : register(b1) {
    float4x4 world_matrix;
    float4 object_color;
};

PixelShaderOutput main(PixelShaderInput input) : SV_TARGET
{

    PixelShaderOutput output;
    output.albedo = object_color * tex.Sample(tex_sampler, input.uv);
    output.normal = float4(input.normal, 1.0);
    output.position = float4(input.wpos, 1.0);

    return output;
}