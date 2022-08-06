struct VertexShaderInput {
    float4 position : POSITION;
};

struct VertexShaderOutput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer GLOBALS : register(b0) {
    float4x4 view_matrix;
    float4x4 proj_matrix;
    float3 cam_pos;
};

float4 transform_to_camera(float4 vec) {
    float4 view = mul(view_matrix, vec);
    float4 projected = mul(proj_matrix, view);
    return projected;
}

VertexShaderOutput main(VertexShaderInput input) {
    VertexShaderOutput output;
    output.pos = transform_to_camera(float4(input.position.xyz, 1.0));
    output.color = float4(1.0, 0, 0, 1.0) * (1.0 - input.position.w) + float4(0.0, 0.0, 1.0, 1.0) * input.position.w;
    return output;
}