#include "applicationclass.h"

// --------------------------------------------------------------------------------------------------------------------
ApplicationClass::ApplicationClass()
{
    m_Direct3D = nullptr;
    m_Camera = nullptr;
    m_Model = nullptr;
    m_Shader = nullptr;
    m_Light = nullptr;
}

ApplicationClass::ApplicationClass(const ApplicationClass&)
{
}

ApplicationClass::~ApplicationClass()
{
}

// --------------------------------------------------------------------------------------------------------------------
#define SHOW_MSG_AND_REURN(msg, title) {\
    MessageBox(hwnd, msg, title, MB_OK);\
    return false;\
}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
    char textureFilename[128];
    bool result;

    // Step 1: Create the direct3d object. -------------------------------------------------------------------------------
    m_Direct3D = new D3DClass();
    result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) { SHOW_MSG_AND_REURN("Could not initialize Direct3D", "Error");}
    if (CHECK_RT_TEST_NUM(3)) { return true; }

    // Step 2: Create the camera object. ---------------------------------------------------------------------------------
    m_Camera = new CameraClass;

    // Set the initial position of the camera.
    m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

    // Step 3: Create and initialize the model object. -------------------------------------------------------------------
    m_Model = new ModelClass;

    // Set the name of the texture file that we will be loading.
    strcpy_s(textureFilename, "../data/textures/stone01.tga");

    result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), textureFilename);
    if (!result) { SHOW_MSG_AND_REURN("Could not initialize the model object.", "Error"); }

    // Create and initialize the shader object.
    m_Shader = new ShaderClass;

    result = m_Shader->Initialize(m_Direct3D->GetDevice(), hwnd);
    if (!result) { SHOW_MSG_AND_REURN("Could not initialize the shader object.", "Error"); }

    // Create and initialize the light object.
    // The color of the light is set to white and the light direction is set to point down the positive Z axis.
    m_Light = new LightClass;

    m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetDirection(0.0f, 0.0f, 1.0f);

    return true;
}

void ApplicationClass::Shutdown()
{
    // Release the light object.
    if (m_Light) {
        delete m_Light;
        m_Light = nullptr;
    }

    // Release the color shader object.
    if (m_Shader) {
        m_Shader->Shutdown();
        delete m_Shader;
        m_Shader = nullptr;
    }

    // Release the model object.
    if (m_Model) {
        m_Model->Shutdown();
        delete m_Model;
        m_Model = nullptr;
    }

    // Release the camera object.
    if (m_Camera) {
        delete m_Camera;
        m_Camera = nullptr;
    }

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
    static float rotation = 0.0f;
    bool result;

    // Update the rotation variable each frame.
    rotation -= 0.0174532925f * 0.1f;
    if (rotation < 0.0f) { rotation += 360.0f; }

    // Render the graphics scene.
    result = Render(rotation);
    if (!result) { return false; }

    return true;
}

// --------------------------------------------------------------------------------------------------------------------
// It still begins with clearing the scene except that it is cleared to black. After that it calls the Render function
// for the camera object to create a view matrix based on the camera's location that was set in the Initialize function.
// Once the view matrix is created, we get a copy of it from the camera class.
// We also get copies of the world and projection matrix from the D3DClass object.
// We then call the ModelClass::Render function to put the green triangle model geometry on the graphics pipeline. W
// ith the vertices now prepared we call the color shader to draw the vertices using the model information and the three
// matrices for positioning each vertex. The green triangle is now drawn to the back buffer. With that the scene is
// complete and we call EndScene to display it to the screen.
bool ApplicationClass::Render(float rotation)
{
    XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
    bool result;

    if (CHECK_RT_TEST_NUM(3) == true) {
        // Clear the buffers to begin the scene - gray
        m_Direct3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

        // Present the rendered scene to the screen.
        m_Direct3D->EndScene();

        return true;
    }

    // Step 1: Clear the back buffer ----------------------------------------------------------------------------------
    // Clear the buffers to begin the scene - black
    m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // Step 2: Peset the render frame ------------------------------------------------------------------------------------
    // 2-a: Generate the view matrix based on the camera's position.
    // m_Camera->SetPosition(0.0f, 0.0f, -15.0f);
    m_Camera->Render();

    // 2-b: Get the world, view, and projection matrices from the camera and d3d objects.
    m_Direct3D->GetWorldMatrix(worldMatrix);
    m_Camera->GetViewMatrix(viewMatrix);
    m_Direct3D->GetProjectionMatrix(projectionMatrix);

    // Here we rotate the world matrix by the rotation value so that when we render the triangle using this updated world matrix it will spin the triangle by the rotation amount.
    if (CHECK_RT_TEST_NUM(6) == true) {
        // Rotate the world matrix by the rotation value so that the triangle will spin.
        worldMatrix = XMMatrixRotationY(rotation);
    }

    // 2-c: Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
    m_Model->Render(m_Direct3D->GetDeviceContext());

    // 2-d: Render the model using the color shader.
    if (CHECK_RT_TEST_NUM(4)) { result = m_Shader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, nullptr, m_Light->GetDirection(), m_Light->GetDiffuseColor()); }
    if (CHECK_RT_TEST_NUM(5) || CHECK_RT_TEST_NUM(6)) { result = m_Shader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture(), m_Light->GetDirection(), m_Light->GetDiffuseColor()); }
    if (!result) { return false; }

    // Step 3: Present the rendered scene to the screen. -----------------------------------------------------------------
    m_Direct3D->EndScene();

    return true;
}

// --------------------------------------------------------------------------------------------------------------------
