#include "WindowHelper.h"
#include <iostream>
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND SetupWindow(HINSTANCE instance, UINT width, UINT height, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"Cool Rasterizer";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND window = CreateWindowEx(0, CLASS_NAME, L"Very cool", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, width, height, nullptr, nullptr, instance, nullptr);

	if (window == nullptr) {
		std::cerr << "HWND was nullptr, last error: " << GetLastError() << std::endl;
		return nullptr;
	}
	ShowWindow(window, nCmdShow);
	return window;
}