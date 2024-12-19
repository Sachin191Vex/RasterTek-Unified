#pragma once

#include "RasterTek.h"
#include "inputclass.h"
#include "applicationclass.h"

// FUNCTION PROTOTYPES
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize(); // Do all object initialization in this function
	void Run();
	void Shutdown();   // Do all object cleanup in this function

	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

private:
	bool Frame();

	bool InitializeWindows(int& screenWidth, int& screenHeight);
	void ShutdownWindows();

private:
	LPCSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_Input;
	ApplicationClass* m_Application;
};

// GLOBALS
static SystemClass* ApplicationHandle = nullptr;
