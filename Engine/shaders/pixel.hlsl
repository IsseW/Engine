SamplerState tex_sampler : register(s0);

Texture2D ambient_tex : register(t0);
Texture2D diffuse_tex : register(t1);
Texture2D specular_tex : register(t2);

struct PixelShaderInput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 wpos : TEXCOORD1;
    float3 obj_pos : TEXCOORD2;
    float3x3 tbn : MATRIX;
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
    float3 cam_pos;
};

cbuffer OBJECT : register(b1) {
    float4x4 world_matrix;
};

cbuffer MATERIAL : register(b2) {
    float4 ambient_c;
    float4 diffuse_c;
    float4 specular_c;
    float shininess_c;
};

PixelShaderOutput main(PixelShaderInput input) : SV_TARGET
{
    PixelShaderOutput output;

    output.ambient.xyz = ambient_c.xyz  * ambient_tex.Sample(tex_sampler, input.uv).xyz;
    output.ambient.w = 1.0;
    output.diffuse.xyz = diffuse_c.xyz * diffuse_tex.Sample(tex_sampler, input.uv).xyz;
    output.diffuse.w = 1.0;
    output.specular.xyz = specular_c.xyz * specular_tex.Sample(tex_sampler, input.uv).xyz;
    output.specular.w = 1.0;

    output.normal = float4(input.normal, 1.0);
    output.position = float4(input.wpos, shininess_c);

    return output;
}