// Filename: bitmapclass.h
#ifndef _BITMAPCLASS_H_
#define _BITMAPCLASS_H_

// INCLUDES
#include "RasterTek.h"
#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

#include "textureclass.h"

// BitmapClass will be used to represent an individual 2D image that needs to be rendered to the screen.
// For every 2D image you have you will need a new BitmapClass for each.
// Note that this class is just the ModelClass re-written to handle 2D images instead of 3D objects.
// Class name: BitmapClass
class BitmapClass
{
private:
    // Each bitmap image is still a polygon object that gets rendered similar to 3D objects.
    // For 2D images we just need a position vector and texture coordinates.
    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
    };

public:
    BitmapClass();
    BitmapClass(const BitmapClass&);
    ~BitmapClass();

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, char* textureFilename, int renderX, int renderY);
    void Shutdown();
    bool Render(ID3D11DeviceContext* deviceContext);

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

    void SetRenderLocation(int x, int y);

private:
    bool InitializeBuffers(ID3D11Device* device);
    void ShutdownBuffers();
    bool UpdateBuffers(ID3D11DeviceContext* deviceContent);
    void RenderBuffers(ID3D11DeviceContext* deviceContent);

    bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename);
    void ReleaseTexture();

private:
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    // The BitmapClass will need to maintain some extra information that a 3D model wouldn't such as the screen size,
    // the bitmap size, and the last place it was rendered. We have added extra private variables here to track that extra information.
    int m_vertexCount, m_indexCount, m_screenWidth, m_screenHeight, m_bitmapWidth, m_bitmapHeight, m_renderX, m_renderY, m_prevPosX, m_prevPosY;
    TextureClass* m_Texture;
};

#endif