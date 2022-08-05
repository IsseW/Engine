struct PixelShaderInput {
    float4 position : SV_POSITION;
};

struct PixelShaderOutput {
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 normal;
    float4 position;
};

PixelShaderOutput main(PixelShaderInput input) : SV_TARGET
{
    PixelShaderOutput output;
    
    output.ambient = float4(1.0, 1.0, 1.0, 1.0);
    output.diffuse = float4(0.0, 0.0, 0.0, 1.0);
    output.specular = float4(0.0, 0.0, 0.0, 1.0);

    output.normal = float4(0.0, 0.0, 0.0, 1.0);
    output.position = float4(0.0, 0.0, 0.0, 0.0);

    return output;
}