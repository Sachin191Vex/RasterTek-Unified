#include "applicationclass.h"

// --------------------------------------------------------------------------------------------------------------------
ApplicationClass::ApplicationClass()
{
	m_Direct3D = nullptr;
}

ApplicationClass::ApplicationClass(const ApplicationClass&)
{
}

ApplicationClass::~ApplicationClass()
{
}

// --------------------------------------------------------------------------------------------------------------------
bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
	m_Direct3D = new D3DClass();
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result) {
		MessageBox(hwnd, "Could not initialize Direct3D", "Error", MB_OK);
		return false;
	}
	return true;
}

void ApplicationClass::Shutdown()
{
	// Release the Direct3D object.
	if (m_Direct3D) {
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = nullptr;
	}
	return;
}

// --------------------------------------------------------------------------------------------------------------------
bool ApplicationClass::Frame()
{
	bool result;

	// Render the graphics scene.
	result = Render();
	if (!result) { return false; }

    return true;
}

// --------------------------------------------------------------------------------------------------------------------
bool ApplicationClass::Render()
{
	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

    return true;
}

// --------------------------------------------------------------------------------------------------------------------
