struct VertexShaderInput {
    float4 position : POSITION;
};

struct VertexShaderOutput {
    float4 wpos : SV_POSITION;
};

cbuffer LOCALS : register(b1) {
    float4x4 world_matrix;
};

VertexShaderOutput main(VertexShaderInput input)
{
    float4 pos = mul(world_matrix, float4(input.position.xyz, 1.0));
    VertexShaderOutput output;
    output.wpos = float4(pos.xyz, input.position.w);
    return output;
}