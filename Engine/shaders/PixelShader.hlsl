Texture2D ambient_tex : register(t0);
SamplerState ambient_sampler : register(s0);
Texture2D diffuse_tex : register(t1);
SamplerState diffuse_sampler : register(s1);
Texture2D specular_tex : register(t2);
SamplerState specular_sampler : register(s2);
Texture2D shinyness_tex : register(t3);
SamplerState shinyness_sampler : register(s3);

struct PixelShaderInput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 wpos : TEXCOORD2;
};

struct PixelShaderOutput {
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 normal;
    float4 position;
};

cbuffer GLOBALS : register(b0) {
    float4x4 view_matrix;
    float4x4 proj_matrix;
};

cbuffer OBJECT : register(b1) {
    float4x4 world_matrix;
};

cbuffer MATERIAL : register(b2) {
    float3 ambient_c;
    float3 diffuse_c;
    float3 specular_c;
    float shinyness_c;
};

PixelShaderOutput main(PixelShaderInput input) : SV_TARGET
{
    PixelShaderOutput output;

    output.ambient.xyz = ambient_c  * ambient_tex.Sample(ambient_sampler, input.uv).xyz;
    output.ambient.w = 1.0;
    output.diffuse.xyz = diffuse_c * diffuse_tex.Sample(diffuse_sampler, input.uv).xyz;
    output.diffuse.w = 1.0;
    output.specular.xyz = specular_c * specular_tex.Sample(specular_sampler, input.uv).xyz;
    output.specular.w = shinyness_c * shinyness_tex.Sample(specular_sampler, input.uv).x;

    output.normal = float4(input.normal, 1.0);
    output.position = float4(input.wpos, 1.0);

    return output;
}