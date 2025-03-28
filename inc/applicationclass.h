#pragma once

#include "RasterTek.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "shaderclass.h"
#include "lightclass.h"
#include "bitmapclass.h"
#include "timerclass.h"

// GLOBALS
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

typedef struct ApplicationConfig {
    bool useTimer = false;
} ApplicationConfig;

class ApplicationClass {
public:
    ApplicationClass();
    ApplicationClass(const ApplicationClass&);
    ~ApplicationClass();

public:
    bool Initialize(int screenWidth, int screenHeight, HWND hwnd, ApplicationConfig &config);
    void Shutdown();

    bool Frame();

private:
    bool Render(float rotation);

    ApplicationConfig m_Config;
    D3DClass* m_Direct3D;
    CameraClass* m_Camera;
    ModelClass* m_Model;
    ShaderClass* m_Shader;
    BitmapClass* m_Bitmap;
    LightClass* m_Lights;
    TimerClass* m_Timer;
    int m_numDiffuseLights;
    bool m_isDiffuseLightPosGiven;   // Position of diffuse lights is specified, if true; otherise direction will be given. 
                                     // (May need to use position to calculate direction, may be wrt each vertex vor wrt world
};