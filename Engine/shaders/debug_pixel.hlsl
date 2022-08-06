struct PixelShaderInput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

struct PixelShaderOutput {
    float4 color;
};

PixelShaderOutput main(PixelShaderInput input) : SV_TARGET
{
    PixelShaderOutput output;
    output.color = input.color;

    return output;
}