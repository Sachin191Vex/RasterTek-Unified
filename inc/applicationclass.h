#pragma once

#include "RasterTek.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "shaderclass.h"
#include "lightclass.h"

// GLOBALS
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

class ApplicationClass {
public:
    ApplicationClass();
    ApplicationClass(const ApplicationClass&);
    ~ApplicationClass();

public:
    bool Initialize(int screenWidth, int screenHeight, HWND hwnd);
    void Shutdown();

    bool Frame();

private:
    bool Render(float rotation);

    D3DClass* m_Direct3D;
    CameraClass* m_Camera;
    ModelClass* m_Model;
    ShaderClass* m_Shader;
    LightClass* m_Lights;
    int m_numDiffuseLights;
    bool m_isDiffuseLightPosGiven;   // Position of diffuse lights is specified, if true; otherise direction will be given. 
                                     // (May need to use position to calculate direction, may be wrt each vertex vor wrt world
};