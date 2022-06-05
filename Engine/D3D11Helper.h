#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <iostream>

#include "PipelineHelper.h"

bool SetupD3D11(UINT width, UINT height, HWND window, Pipeline& pipeline);
