#include <iostream>
#include <Windows.h>
#include <d3d11.h>
#include <chrono>

#include "WindowHelper.h"
#include "D3D11Helper.h"
#include "PipelineHelper.h"

DirectX::XMFLOAT4X4 GetMatrix(float t) {
	DirectX::XMFLOAT3 scaling_vector = {
		1.0f,
		1.0f,
		1.0f,
	};
	auto scaling = DirectX::XMLoadFloat3(&scaling_vector);
	DirectX::XMFLOAT3 rotation_origin_vector = {
		0.0f,
		0.0f,
		0.0f,
	};
	auto rotation_origin = DirectX::XMLoadFloat3(&rotation_origin_vector);
	
	
	auto rotation = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, -t - DirectX::XM_PIDIV2, 0.0f);
	DirectX::XMFLOAT3 translation_vector = {
		cos(t),
		0.0,
		2.5f + sin(t),
	};
	auto translation = DirectX::XMLoadFloat3(&translation_vector);
	auto matrix = DirectX::XMMatrixAffineTransformation(scaling, rotation_origin, rotation, translation);
	DirectX::XMFLOAT4X4 result;

	DirectX::XMStoreFloat4x4(&result, matrix);

	return result;
}

void Render(Pipeline& pipeline, float dt)
{
	float clearColour[4] = { 0, 0, 0, 0 };
	pipeline.context->ClearRenderTargetView(pipeline.rtv, clearColour);
	pipeline.context->ClearDepthStencilView(pipeline.dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	D3D11_MAPPED_SUBRESOURCE resource;
	static float t = 0.0f;
	t += dt / 1000.0f;
	if (FAILED(pipeline.context->Map(pipeline.vsConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource))) {
		std::cerr << "Map failed" << std::endl;
	}
	else {
		VS_CONSTANT_BUFFER buffer;
		buffer.projection_matrix = CreateProjection(pipeline);
		buffer.view_matrix = VIEW_MATRIX;
		buffer.world_matrix = GetMatrix(t);
		buffer.light = LIGHT;
		memcpy(resource.pData, &buffer, sizeof(buffer));
		pipeline.context->Unmap(pipeline.vsConstantBuffer, 0);
	}

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	pipeline.context->IASetVertexBuffers(0, 1, pipeline.vertexBuffer.get(), &stride, &offset);
	pipeline.context->IASetInputLayout(pipeline.inputLayout);
	pipeline.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pipeline.context->VSSetShader(pipeline.vShader, nullptr, 0);
	pipeline.context->RSSetViewports(1, &pipeline.viewport);
	pipeline.context->PSSetShader(pipeline.pShader, nullptr, 0);
	pipeline.context->OMSetRenderTargets(1, pipeline.rtv.get(), pipeline.dsView);

	pipeline.context->Draw(6, 0);
	pipeline.swapChain->Present(0, 0);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) 
{
	const UINT WIDTH = 1024;
	const UINT HEIGHT = 576;
	HWND window = SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow);
	if (window) {

		AllocConsole();
		FILE* console;
		freopen_s(&console, "CONOUT$", "w", stderr);

		Pipeline pipeline = {};
		
		if (!SetupD3D11(WIDTH, HEIGHT, window, pipeline))
		{
			std::cerr << "Failed to setup d3d11!" << std::endl;
			return -1;
		}

		if (!SetupPipeline(pipeline))
		{
			std::cerr << "Failed to setup pipeline!" << std::endl;
			return -1;
		}

		MSG msg = { };
		auto last_frame = std::chrono::high_resolution_clock::now();
		while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT) {
			auto now = std::chrono::high_resolution_clock::now();
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			float dt = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(now - last_frame).count();
			Render(pipeline, dt);
			last_frame = now;
		}
		if (console) {
			fclose(console);
		}
	}
	else {
		std::cerr << "Failed to setup window!" << std::endl;
		return -1;
	}
	return 0;
}