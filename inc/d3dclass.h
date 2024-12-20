#pragma once
#include "RasterTek.h"

class D3DClass {
public:
	D3DClass();
	D3DClass(const D3DClass& from);
	~D3DClass();

	bool Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);
	void Shutdown();
};