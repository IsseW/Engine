SamplerState tex_sampler : register(s0);

Texture2D ambient_tex : register(t0);
Texture2D diffuse_tex : register(t1);
Texture2D specular_tex : register(t2);
Texture2D normal_tex : register(t3);

TextureCube reflection_tex : register(t4);

struct PixelShaderInput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 wpos : TEXCOORD1;
    float3 obj_pos : TEXCOORD2;
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
    float4 normal_c;
    float shininess_c;
};

PixelShaderOutput main(PixelShaderInput input) : SV_TARGET {
    PixelShaderOutput output;

    float2 uv = input.uv;
    float3 wpos = input.wpos;

    float3 n = input.normal;

    float3 p_dx = ddx(wpos.xyz);
    float3 p_dy = ddy(wpos.xyz);
    float2 uv_dx = ddx(uv);
    float2 uv_dy = ddy(uv);

    float3 t = normalize(uv_dy.y * p_dx - uv_dx.y * p_dy);
    float3 b = normalize(uv_dy.x * p_dx - uv_dx.x * p_dy); // sign inversion
    float3 x = cross(n, t);

    t = normalize(cross(x, n));

    x = cross(b, n);
    b = normalize(cross(x, n));

    float3x3 tbn = float3x3(t, b, n);

    float3 normal = normalize(normal_c.xyz * normal_tex.Sample(tex_sampler, uv).xyz * 2.0 - 1.0);

    output.normal = float4(normal.x * t + normal.y * b + normal.z * n, 1.0);
    output.position = float4(wpos, shininess_c);

    float3 reflection = reflection_tex.Sample(tex_sampler, normalize(output.normal)).xyz;

    output.ambient.xyz = ambient_c.xyz * ambient_tex.Sample(tex_sampler, uv).xyz + reflection / 3.0;
    output.ambient.w = 1.0;
    output.diffuse.xyz = diffuse_c.xyz * reflection * diffuse_tex.Sample(tex_sampler, uv).xyz;
    output.diffuse.w = 1.0;
    output.specular.xyz = specular_c.xyz * specular_tex.Sample(tex_sampler, uv).xyz;
    output.specular.w = 1.0;

    return output;
}