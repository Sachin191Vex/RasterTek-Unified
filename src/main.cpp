////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp : Defines the entry point for the application.
////////////////////////////////////////////////////////////////////////////////
#include "RasterTek.h"
#include "systemclass.h"
	
// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	SystemClass* system = nullptr;
	int result;

	system = new SystemClass;
	result = system->Initialize();
	if (result == RT_OK) {
		system->Run();
	}

	system->Shutdown();
	delete system;
	system = nullptr;
}
