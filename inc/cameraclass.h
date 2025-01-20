////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include "RasterTek.h"
#include <directxmath.h>
using namespace DirectX;

// Class name: CameraClass
// Camera class to let DirectX 11 know from where and also how we are viewing the scene. The camera class will keep
// track of where the camera is and its current rotation. It will use the position and rotation information
// to generate a view matrix which will be passed into the HLSL shader for rendering.
class CameraClass
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	void GetViewMatrix(XMMATRIX&);

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	XMMATRIX m_viewMatrix;
};

#endif