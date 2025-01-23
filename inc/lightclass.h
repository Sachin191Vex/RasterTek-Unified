// Filename: lightclass.h
#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_

// INCLUDES
#include "RasterTek.h"
#include <directxmath.h>
using namespace DirectX;

// Class name: LightClass
class LightClass
{
public:
    LightClass();
    LightClass(const LightClass&);
    ~LightClass();

    void SetAmbientColor(float red, float green, float blue, float alpha);
    void SetDiffuseColor(float red, float green, float blue, float alpha);
    void SetDirection(float x, float y, float z);
    void SetSpecularColor(float, float, float, float);
    void SetSpecularPower(float);

    XMFLOAT4 GetAmbientColor();
    XMFLOAT4 GetDiffuseColor();
    XMFLOAT3 GetDirection();
    XMFLOAT4 GetSpecularColor();
    float GetSpecularPower();

private:
    XMFLOAT4 m_ambientColor;
    XMFLOAT4 m_diffuseColor;
    XMFLOAT3 m_direction;
    XMFLOAT4 m_specularColor;
    float m_specularPower;
};

#endif