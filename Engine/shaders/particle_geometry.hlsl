
cbuffer GLOBALS : register(b0) {
    float4x4 view_matrix;
    float4x4 proj_matrix;
    float3 cam_pos;
};

cbuffer EXTRA_GLOBALS : register(b1) {
    float2 start_size;
}

struct VertexOut {
    float4 wpos : SV_POSITION;
};

struct PixelIn {
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 wpos : TEXCOORD1;
    float3 obj_pos : TEXCOORD2;
};

float4 transform_to_camera(float4 vec) {
    float4 view = mul(view_matrix, vec);
    float4 projected = mul(proj_matrix, view);
    return projected;
}

[maxvertexcount(6)]
void main(point VertexOut input[1], inout TriangleStream<PixelIn> out_stream) {
    float3 pos = input[0].wpos.xyz;
    float life_time = input[0].wpos.w;


    float2 size = start_size * life_time;

    float3 normal = normalize(cam_pos - pos);
    float3 axis1;
    float3 axis2;
    float3 up = float3(0.0, 1.0, 0.0);

    if (dot(up, normal) == 0.0) {
        axis1 = float3(1.0, 0.0, 0.0);
        axis2 = float3(0.0, 0.0, 1.0);
    }
    else {
        axis1 = normalize(cross(normal, up));
        axis2 = normalize(cross(normal, axis1));
    }
    axis1 *= size.x;
    axis2 *= size.y;

    PixelIn output;
    output.normal = normal;

    output.wpos = pos + axis1 + axis2;
    output.position = transform_to_camera(float4(output.wpos, 1.0));
    output.uv = float2(1.0, 1.0);
    output.obj_pos = float3(1.0, 1.0, 0.0);
    out_stream.Append(output);

    output.wpos = pos + axis1 - axis2;
    output.position = transform_to_camera(float4(output.wpos, 1.0));
    output.uv = float2(1.0, 0.0);
    output.obj_pos = float3(1.0, -1.0, 0.0);
    out_stream.Append(output);

    output.wpos = pos - axis1 - axis2;
    output.position = transform_to_camera(float4(output.wpos, 1.0));
    output.uv = float2(0.0, 0.0);
    output.obj_pos = float3(-1.0, -1.0, 0.0);
    out_stream.Append(output);


    output.wpos = pos + axis1 + axis2;
    output.position = transform_to_camera(float4(output.wpos, 1.0));
    output.uv = float2(1.0, 1.0);
    output.obj_pos = float3(1.0, 1.0, 0.0);
    out_stream.Append(output);

    output.wpos = pos - axis1 - axis2;
    output.position = transform_to_camera(float4(output.wpos, 1.0));
    output.uv = float2(0.0, 0.0);
    output.obj_pos = float3(-1.0, -1.0, 0.0);
    out_stream.Append(output);

    output.wpos = pos - axis1 + axis2;
    output.position = transform_to_camera(float4(output.wpos, 1.0));
    output.uv = float2(0.0, 1.0);
    output.obj_pos = float3(-1.0, 1.0, 0.0);
    out_stream.Append(output);
    

    /*
    PixelIn v0;
    v0.wpos = pos + axis1 + axis2;
    v0.position = transform_to_camera(float4(v0.wpos, 1.0));
    v0.uv = float2(1.0, 1.0);
    v0.normal = normal;

    PixelIn v1;
    v1.wpos = pos + axis1 - axis2;
    v1.position = transform_to_camera(float4(v1.wpos, 1.0));
    v1.uv = float2(1.0, 0.0);
    v1.normal = normal;

    PixelIn v2;
    v2.wpos = pos - axis1 - axis2;
    v2.position = transform_to_camera(float4(v2.wpos, 1.0));
    v2.uv = float2(0.0, 0.0);
    v2.normal = normal;

    PixelIn v3;
    v3.wpos = pos - axis1 + axis2;
    v3.position = transform_to_camera(float4(v3.wpos, 1.0));
    v3.uv = float2(0.0, 1.0);
    v3.normal = normal;

    out_stream.Append(v0);
    out_stream.Append(v1);
    out_stream.Append(v2);
    out_stream.Append(v0);
    out_stream.Append(v2);
    out_stream.Append(v3);
    */
}