#include "applicationclass.h"

// --------------------------------------------------------------------------------------------------------------------
ApplicationClass::ApplicationClass()
{
    m_Direct3D = nullptr;
    m_Camera = nullptr;
    m_Model = nullptr;
    m_Shader = nullptr;
    m_Bitmap = nullptr;
    m_Lights = nullptr;
    m_Lights = nullptr;
    m_numDiffuseLights = 0;
    m_isDiffuseLightPosGiven = false;
}

ApplicationClass::ApplicationClass(const ApplicationClass&)
{
}

ApplicationClass::~ApplicationClass()
{
}

// --------------------------------------------------------------------------------------------------------------------
#define SHOW_MSG_AND_RETURN(msg, title) {\
    MessageBox(hwnd, msg, title, MB_OK);\
    return false;\
}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
    bool result;
    char modelFilename[128];
    char textureFilename[128];
    char bitmapFilename[128];
    bool useTexture = false;
    bool useAmbient = false;
    bool useDiffuse = false;
    bool useSpecular = false;
    bool useLighting = false;
    bool useGeoRendering = false;
    bool use2DRendering = false;

    // Initilize variable
    strcpy(modelFilename, "");
    strcpy(textureFilename, "");

    // Step 1: Create the direct3d object. -------------------------------------------------------------------------------
    m_Direct3D = new D3DClass();
    result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) { SHOW_MSG_AND_RETURN("Could not initialize Direct3D", "Error"); }
    if (CHECK_RT_TEST_NUM(3)) { return true; }

    // Step 2: Create the camera object. ---------------------------------------------------------------------------------
    m_Camera = new CameraClass;

    // Set the initial position of the camera.
    m_Camera->SetPosition(0.0f, 0.0f, -5.0f);
    if (CHECK_RT_TEST_NUM(8) || CHECK_RT_TEST_NUM(9) || CHECK_RT_TEST_NUM(12)) {
        // Move the camera back another 5 units so that we can see both cubes easily.
        m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
    }
    if (CHECK_RT_TEST_NUM(11)) {
        m_Camera->SetPosition(0.0f, 2.0f, -12.0f);
    }

    // Step 3: Create and initialize the model object. -------------------------------------------------------------------
    // Set the file name of the model.
    m_Model = new ModelClass;

    // Set the name of the model and texture file that we will be loading.
    if (CHECK_RT_TEST_NUM(7) || CHECK_RT_TEST_NUM(8) || CHECK_RT_TEST_NUM(9)) { strcpy(modelFilename, "../data/models/cube.txt"); }
    if (CHECK_RT_TEST_NUM(10)) { strcpy(modelFilename, "../data/models/sphere.txt"); }
    if (CHECK_RT_TEST_NUM(11)) { strcpy(modelFilename, "../data/models/plane.txt"); }

    if (CHECK_RT_TEST_NUM(5) || CHECK_RT_TEST_NUM(6) || CHECK_RT_TEST_NUM(7) || CHECK_RT_TEST_NUM(8) || CHECK_RT_TEST_NUM(9) || CHECK_RT_TEST_NUM(10) || CHECK_RT_TEST_NUM(11) ||
        CHECK_RT_TEST_NUM(12)) {
        strcpy(textureFilename, "../data/textures/stone01.tga");
        useTexture = true;
    }

    if (CHECK_RT_TEST_NUM(6) || CHECK_RT_TEST_NUM(7) || CHECK_RT_TEST_NUM(8) || CHECK_RT_TEST_NUM(9) || CHECK_RT_TEST_NUM(10) || CHECK_RT_TEST_NUM(11)) { useAmbient = true; useDiffuse = true; }
    if (CHECK_RT_TEST_NUM(10)) { useAmbient = true;  useSpecular = true; }

    if (CHECK_RT_TEST_NUM(12)) { useGeoRendering = false; use2DRendering = true; }

    CraftModel craftModel = TRI_FULLCOL;
    if (CHECK_RT_TEST_NUM(1)) { craftModel = TRI_RED; }
    if (CHECK_RT_TEST_NUM(2)) { craftModel = TRI_REDINC; }
    result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), craftModel, modelFilename, textureFilename, useDiffuse);
    if (!result) { SHOW_MSG_AND_RETURN("Could not initialize the model object.", "Error"); }

    // Step 4: Create and initialize the shader object.
    m_Shader = new ShaderClass;

    result = m_Shader->Initialize(m_Direct3D->GetDevice(), hwnd, useTexture, useAmbient, useDiffuse, useSpecular);
    if (!result) { SHOW_MSG_AND_RETURN("Could not initialize the shader object.", "Error"); }

    // Step 5: Create and initialize the light object. -------------------------------------------------------------------
    useLighting = useAmbient || useDiffuse || useSpecular;
    m_numDiffuseLights = 0;
    if (useLighting) {
        // The color of the light is set to white and the light direction is set to point down the positive Z axis.
        m_numDiffuseLights = 1; m_isDiffuseLightPosGiven = false;
        if (CHECK_RT_TEST_NUM(11)) { m_numDiffuseLights = 4; }

        // Create and initialize the light objects array.
        m_Lights = new LightClass[m_numDiffuseLights];

        // Set Ambient light properties - color
        m_Lights[0].SetAmbientColor(0.0f, 0.0f, 0.0f, 1.0f);
        if (useAmbient) {
            m_Lights[0].SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
        }

        // Set Diffuse light properties - color, direction and/or posotion (for all lights used in a test)
        m_Lights[0].SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
        m_Lights[0].SetDirection(0.0f, 0.0f, 1.0f);
        if (CHECK_RT_TEST_NUM(9)) {
            m_Lights[0].SetDirection(1.0f, 0.0f, 0.0f);
        }
        if (CHECK_RT_TEST_NUM(10)) {
            m_Lights[0].SetDirection(1.0f, 0.0f, 1.0f);
        }
        if (CHECK_RT_TEST_NUM(11)) {
            m_isDiffuseLightPosGiven = true;

            // Manually set the color and position of each light.
            m_Lights[0].SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);  // Red
            m_Lights[0].SetPosition(-3.0f, 1.0f, 3.0f);

            m_Lights[1].SetDiffuseColor(0.0f, 1.0f, 0.0f, 1.0f);  // Green
            m_Lights[1].SetPosition(3.0f, 1.0f, 3.0f);

            m_Lights[2].SetDiffuseColor(0.0f, 0.0f, 1.0f, 1.0f);  // Blue
            m_Lights[2].SetPosition(-3.0f, 1.0f, -3.0f);

            m_Lights[3].SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);  // White
            m_Lights[3].SetPosition(3.0f, 1.0f, -3.0f);
        }

        // Set Ambient light properties - color, power
        m_Lights[0].SetSpecularColor(0.0f, 0.0f, 0.0f, 1.0f);
        m_Lights[0].SetSpecularPower(0.0f);
        if (useSpecular) {
            m_Lights[0].SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
            m_Lights[0].SetSpecularPower(32.0f);
        }
    }

    // Step 6: Create and initialize the bitmap object needed for 2D rendering. ------------------------------------------
    if (use2DRendering) {
        m_Bitmap = new BitmapClass;

        strcpy(bitmapFilename, textureFilename);  // "../data/textures/stone01.tga"
        result = m_Bitmap->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, bitmapFilename, 50, 50);
        if (!result) { SHOW_MSG_AND_RETURN("Could not initialize the bitmap object.", "Error"); }
    }

    return true;
}

#define RELEASE_APP_OBJ(obj)  { if (obj) {delete obj; obj = nullptr;} }
#define SHUTDOWN_APP_OBJ(obj) { if (obj) { obj->Shutdown(); delete obj; obj = nullptr;} }
#define RELEASE_APP_OBJ_ARR(arr) { if (arr) { delete[] arr; arr = nullptr;} }

void ApplicationClass::Shutdown()
{
    RELEASE_APP_OBJ_ARR(m_Lights);
    SHUTDOWN_APP_OBJ(m_Bitmap);
    SHUTDOWN_APP_OBJ(m_Shader);
    SHUTDOWN_APP_OBJ(m_Model);
    RELEASE_APP_OBJ(m_Lights);
    SHUTDOWN_APP_OBJ(m_Direct3D);

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

// Tutorisal 8 notes: In this tutorial we will do two transformation examples with the 3D model from the last tutorial.
// The first transformation will be a rotation and a translation.
// The second transformation will be a scale, rotation, and a translation.
//
// We you don't need to send them all into the shader. Instead, you can just multiply all the different
// transform matrices together and it will create a single matrix which will do all of the combined transformations!
//
// Key point: However, note that the order of multiplying matrices together is crucial; if you do it in the wrong order, you will place
// the model somewhere you probably weren't expecting it to be. The correction order of multiplying matrices for the three
// main types of transformation we are performing in this tutorial is: 1 - Scale, 2 - Rotation, 3 - Translation, or SRT for a shorter abbreviation.
//
// To help remember this order it is easiest by just thinking about the model in the 3D world.
// It first starts by being located at the origin of our 3D scene at 0, 0, 0. Now before moving it anywhere we first scale it
// and make it the overall size we wish it to be.
// Once it is scaled, we then rotate it so that it will be oriented in the direction we want it to face.
// Now that it is both scaled and rotated, we can move it to the final location in the 3D world by using translation.

bool ApplicationClass::Render(float rotation)
{
    bool result;
    XMMATRIX worldMatrix, viewMatrix, projectionMatrix, rotateMatrix, translateMatrix, scaleMatrix, srMatrix;
    XMMATRIX viewMatrixDefault, orthoMatrix;
    bool useGeoRendering = false;
    bool use2DRendering = false;

    if (CHECK_RT_TEST_NUM(12)) { useGeoRendering = false; use2DRendering = true; }

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

    // Step 2: Reset the render frame ---------------------------------------------------------------------------------

    // 2-a: Generate the view matrix based on the camera's position.
    m_Camera->Render();
    // m_Camera->SetPosition(0.0f, 0.0f, -15.0f);

    // Object #1 base object ==========================================================================================
    // 2-b: Get the world, view, and projection matrices from the camera and d3d objects.
    m_Direct3D->GetWorldMatrix(worldMatrix);
    m_Camera->GetViewMatrix(viewMatrix);                   // use for Geometry rendering
    m_Camera->GetViewMatrixDefault(viewMatrixDefault);     // use for 2D rendering will - view should not change with camera
    m_Direct3D->GetProjectionMatrix(projectionMatrix);     // Used for Geometry rendering
    m_Direct3D->GetOrthoMatrix(orthoMatrix);               // Used for 2D Rendeirng 

    // Here we rotate the world matrix by the rotation value so that when we render the triangle using this updated world matrix it will spin the triangle by the rotation amount.
    if (CHECK_RT_TEST_NUM(6) || CHECK_RT_TEST_NUM(7) || CHECK_RT_TEST_NUM(9) || CHECK_RT_TEST_NUM(10)) {
        // Rotate the world matrix by the rotation value so that the triangle will spin.
        worldMatrix = XMMatrixRotationY(rotation);
    }
    if (CHECK_RT_TEST_NUM(8)) {
        // Key ont: Multiply in SRT order as described above
        rotateMatrix = XMMatrixRotationY(rotation);  // Build the rotation matrix.
        translateMatrix = XMMatrixTranslation(-2.0f, 0.0f, 0.0f);  // Build the translation matrix.

        // Multiply them together to create the final world transformation matrix.
        worldMatrix = XMMatrixMultiply(rotateMatrix, translateMatrix);
    }
    // 2-c: Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
    m_Model->Render(m_Direct3D->GetDeviceContext());

    // 2-d: Render the model using the color shader.
    bool useAmbientLight = false;
    bool useDiffuseLight = false;
    bool useSpecularLight = false;
    bool useLighting = false;
    XMFLOAT4 ambientColor(0.0, 0.0, 0.0, 1.0);
    XMFLOAT4 diffuseColor[MAX_DIFFUSE_LIGHTS];
    XMFLOAT4 specularColor(0.0, 0.0, 0.0, 1.0);
    float specularPower = 0.0;
    XMFLOAT3 lightPosDir[MAX_DIFFUSE_LIGHTS];

    if (CHECK_RT_TEST_NUM(9) || CHECK_RT_TEST_NUM(10) || CHECK_RT_TEST_NUM(11)) { useAmbientLight = true; }
    if (CHECK_RT_TEST_NUM(6) || CHECK_RT_TEST_NUM(7) || CHECK_RT_TEST_NUM(8) || CHECK_RT_TEST_NUM(9) || CHECK_RT_TEST_NUM(10) || CHECK_RT_TEST_NUM(11)) { useDiffuseLight = true; }
    if (CHECK_RT_TEST_NUM(10)) { useSpecularLight = true; }

    useLighting = useAmbientLight || useDiffuseLight || useSpecularLight;
    if (useLighting) {
        // Get the ambient light paramaters.
        ambientColor = m_Lights->GetAmbientColor();

        // Get the diffuse light paramaters.
        for (auto i = 0; i < m_numDiffuseLights; i++) {
            // Create the diffuse color array from the four light colors.
            diffuseColor[i] = m_Lights[i].GetDiffuseColor();

            if (m_isDiffuseLightPosGiven) {
                // Create the light position array from the four light positions.
                auto position = m_Lights[i].GetPosition();
                lightPosDir[i].x = position.x;
                lightPosDir[i].y = position.y;
                lightPosDir[i].z = position.z;
            }
            else {
                lightPosDir[i] = m_Lights[i].GetDirection();
            }
        }

        // Get the specular light paramaters.
        specularColor = m_Lights->GetSpecularColor();
        specularPower = m_Lights->GetSpecularPower();
    }

    ID3D11ShaderResourceView* texture = m_Model->GetTexture();
    result = m_Shader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, texture,
                              m_Camera->GetPosition(),
                              useAmbientLight, ambientColor,
                              useDiffuseLight, m_numDiffuseLights, diffuseColor,
                              m_isDiffuseLightPosGiven, lightPosDir,
                              useSpecularLight, specularColor, specularPower);
    if (!result) { return false; }

    if (CHECK_RT_TEST_NUM(8)) {
        // Object #2 Additonal object ========================================================================================
        scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);          // Build the scaling matrix.
        rotateMatrix = XMMatrixRotationY(rotation);               // Build the rotation matrix.
        translateMatrix = XMMatrixTranslation(2.0f, 0.0f, 0.0f);  // Build the translation matrix.

        // Multiply the scale, rotation, and translation matrices together to create the final world transformation matrix.
        srMatrix = XMMatrixMultiply(scaleMatrix, rotateMatrix);
        worldMatrix = XMMatrixMultiply(srMatrix, translateMatrix);

        // Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
        m_Model->Render(m_Direct3D->GetDeviceContext());

        // Render the model using the light shader.
        result = m_Shader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
                                  m_Model->GetTexture(),
                                  m_Camera->GetPosition(),
                                  useAmbientLight, ambientColor,
                                  useDiffuseLight, m_numDiffuseLights, diffuseColor,
                                  m_isDiffuseLightPosGiven, lightPosDir,
                                  useSpecularLight, specularColor, specularPower);
        if (!result) { return false; }
    }

    // 2D REndeirng usign bitmap
    if (use2DRendering) {
        // Turn off the Z buffer to begin all 2D rendering.
        m_Direct3D->TurnZBufferOff();

        // Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
        result = m_Bitmap->Render(m_Direct3D->GetDeviceContext());
        if (!result) { return false; }

        // Once the vertex / index buffers are prepared we draw them using the texture shader.
        // Notice we send in the orthoMatrix instead of the projectionMatrix for rendering 2D.
        // Due note also that if your view matrix is changing you will need to create a default one for 2D rendering and use it instead of the regular view matrix.
        // Render the bitmap with the texture shader.
        result = m_Shader->Render(m_Direct3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrixDefault, orthoMatrix,
                                  m_Bitmap->GetTexture(),
                                  m_Camera->GetPosition(),
                                  useAmbientLight, ambientColor,
                                  useDiffuseLight, m_numDiffuseLights, diffuseColor,
                                  m_isDiffuseLightPosGiven, lightPosDir,
                                  useSpecularLight, specularColor, specularPower);
        if (!result) { return false; }

        // After all the 2D rendering is done we turn the Z buffer back on for the next round of 3D rendering.
        m_Direct3D->TurnZBufferOn();
    }

    // Step 3: Present the rendered scene to the screen. -----------------------------------------------------------------
    m_Direct3D->EndScene();

    return true;
}

// --------------------------------------------------------------------------------------------------------------------
