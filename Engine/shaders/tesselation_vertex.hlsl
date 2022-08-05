struct VertexShaderInput {
    float3 position : POSITION;
    float3 normal : NORMAL0;
    float3 tan : TANGENT0;
    float3 uv : TEXCOORD0;
};

struct VertexShaderOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float3 tan : TANGENT0;
    float3 uv : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input) {
    VertexShaderOutput output;
    output.position = float4(input.position, 1.0);
    output.normal = input.normal;
    output.uv = input.uv;
    output.tan = input.tan;
    return output;
}