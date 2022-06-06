#include "PipelineHelper.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <fstream>
#include <string>
#include <iostream>

bool LoadShaders(Pipeline& pipeline, std::string& vShaderByteCode)
{
	std::string shaderData;
	std::ifstream reader;
	reader.open("VertexShader.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		std::cerr << "Could not open VS file!" << std::endl;
		return false;
	}

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());

	if (FAILED(pipeline.device->CreateVertexShader(shaderData.c_str(), shaderData.length(), nullptr, pipeline.vShader.get())))
	{
		std::cerr << "Failed to create vertex shader!" << std::endl;
		return false;
	}

	vShaderByteCode = shaderData;
	shaderData.clear();
	reader.close();
	reader.open("PixelShader.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		std::cerr << "Could not open PS file!" << std::endl;
		return false;
	}

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());

	if (FAILED(pipeline.device->CreatePixelShader(shaderData.c_str(), shaderData.length(), nullptr, pipeline.pShader.get())))
	{
		std::cerr << "Failed to create pixel shader!" << std::endl;
		return false;
	}

	return true;
}

bool CreateInputLayout(Pipeline& pipeline, const std::string& vShaderByteCode)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[3] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	HRESULT hr = pipeline.device->CreateInputLayout(inputDesc, 3, vShaderByteCode.c_str(), vShaderByteCode.length(), pipeline.inputLayout.get());

	return !FAILED(hr);
}


bool CreateVertexBuffer(Pipeline& pipeline)
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(MESH);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = MESH;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = pipeline.device->CreateBuffer(&bufferDesc, &data, pipeline.vertexBuffer.get());
	return !FAILED(hr);
}

DirectX::XMFLOAT4X4 CreateProjection(Pipeline& pipeline) {
	float aspectRatio = pipeline.viewport.Width / pipeline.viewport.Height;
	auto perspective = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60.0f), aspectRatio, 0.1f, 10.0f);
	DirectX::XMFLOAT4X4 result;
	DirectX::XMStoreFloat4x4(&result, perspective);
	return result;
}

bool CreateVSConstantBuffer(Pipeline& pipeline) {
	VS_CONSTANT_BUFFER vsConstData;
	
	vsConstData.view_matrix = VIEW_MATRIX;
	vsConstData.world_matrix = WORLD_MATRIX;
	vsConstData.projection_matrix = CreateProjection(pipeline);
	vsConstData.light = LIGHT;

	// Fill in a buffer description.
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &vsConstData;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;


	// Create the buffer.
	HRESULT hr = pipeline.device->CreateBuffer(&cbDesc, &initData,
		pipeline.vsConstantBuffer.get());

	if (FAILED(hr))
		return false;

	// Set the buffer.
	pipeline.context->VSSetConstantBuffers(0, 1, pipeline.vsConstantBuffer.get());
	return true;
}

bool CreatePSConstantBuffer(Pipeline& pipeline) {
	PS_CONSTANT_BUFFER vsConstData;

	vsConstData.light = LIGHT;

	// Fill in a buffer description.
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(PS_CONSTANT_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &vsConstData;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;


	// Create the buffer.
	HRESULT hr = pipeline.device->CreateBuffer(&cbDesc, &initData,
		pipeline.psConstantBuffer.get());

	if (FAILED(hr))
		return false;

	// Set the buffer.
	pipeline.context->PSSetConstantBuffers(0, 1, pipeline.psConstantBuffer.get());
	return true;
}

bool CreateTexture(Pipeline& pipeline) {
	int width, height, channels;
	auto image = stbi_load("image.png", &width, &height, &channels, 4);
	if (!image) {
		std::cerr << "Image is NULLPTR" << std::endl;
		return false;
	}


	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = NULL;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = image;
	data.SysMemPitch = width * 4;
	data.SysMemSlicePitch = 0;

	HRESULT result = pipeline.device->CreateTexture2D(&desc, &data, pipeline.texture.get());

	if (FAILED(result)) return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC resource_desc;
	resource_desc.Texture2D = { 0, 1 };
	resource_desc.Format = desc.Format;
	resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	result = pipeline.device->CreateShaderResourceView(pipeline.texture, &resource_desc, pipeline.shaderResource.get());

	if (FAILED(result)) return false;

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.MaxAnisotropy = 4;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;


	result = pipeline.device->CreateSamplerState(&sampler_desc, pipeline.samplerState.get());
	if (FAILED(result)) return false;
	pipeline.context->PSSetShaderResources(0, 1, pipeline.shaderResource.get());
	pipeline.context->PSSetSamplers(0, 1, pipeline.samplerState.get());
	free(image);
	return true;
}

bool SetupPipeline(Pipeline& pipeline)
{
	std::string vShaderByteCode;
	if (!LoadShaders(pipeline, vShaderByteCode))
	{
		std::cerr << "Error loading shaders!" << std::endl;
		return false;
	}
	if (!CreateInputLayout(pipeline, vShaderByteCode))
	{
		std::cerr << "Error creating input layout!" << std::endl;
		return false;
	}

	if (!CreateVertexBuffer(pipeline))
	{
		std::cerr << "Error creating vertex buffer!" << std::endl;
		return false;
	}

	if (!CreateVSConstantBuffer(pipeline)) {
		std::cerr << "Error creating vertex shader constant buffer" << std::endl;
		return false;
	}

	if (!CreatePSConstantBuffer(pipeline)) {
		std::cerr << "Error creating pixel shader constant buffer" << std::endl;
		return false;
	}

	if (!CreateTexture(pipeline)) {
		std::cerr << "Error creating loading and creating texture" << std::endl;
		return false;
	}

	return true;
}