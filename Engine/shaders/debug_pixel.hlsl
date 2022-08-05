struct PixelShaderInput {
    float4 pos : SV_POSITION;
};

struct PixelShaderOutput {
    float4 colour;
};

PixelShaderOutput main(PixelShaderInput input) : SV_TARGET
{
    PixelShaderOutput output;
    output.colour = float4(1.0, 0, 0, 1);

    return output;
}