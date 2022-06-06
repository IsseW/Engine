#pragma once
#include "util.h"

struct GPUMesh {
	DXPointer<ID3D11Buffer> vertexBuffer;
};

struct GPUTexture {
	DXPointer<ID3D11Texture2D> texture;
	DXPointer<ID3D11ShaderResourceView> shaderResource;
	DXPointer<ID3D11SamplerState> samplerState;
};