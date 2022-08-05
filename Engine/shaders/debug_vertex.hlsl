struct VertexShaderInput {
    float4 position : POSITION;
};

struct VertexShaderOutput {
    float4 pos : SV_POSITION;
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
    float4 pos = transform_to_camera(input.position);
    VertexShaderOutput output;
    output.pos = pos;
    return output;
}