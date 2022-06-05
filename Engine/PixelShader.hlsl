Texture2D simpleTexture : register(t0);
SamplerState simpleSampler : register(s0);

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 view_direction : TEXCOORD2;
    float3 light_dir: TEXCOORD3;
};

struct Light {
    float3 pos;
    float strength;
};

cbuffer PS_CONSTANT_BUFFER : register(b0) {
    Light light;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 color = simpleTexture.Sample(simpleSampler, input.uv);

    float ambient = 0.2;

    float shininess = 8.5;

    float diffuse = max(dot(input.normal, input.light_dir), 0.0);

    float3 reflect_dir = reflect(input.light_dir, input.normal);
    float specular = pow(max(dot(reflect_dir, input.view_direction), 0.0), shininess) * 80.0;

    return float4(color * (ambient + diffuse) + specular, 1.0f);
}