// Filename: modelclass.h
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include "RasterTek.h"
#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

#include "textureclass.h"

// Class name: ModelClass
class ModelClass
{
private:
    struct VertexTypeColor
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    struct VertexTypeTexture
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
    };

    struct VertexTypeTextureLight
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
        XMFLOAT3 normal;
    };

public:
    ModelClass();
    ModelClass(const ModelClass&);
    ~ModelClass();

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* textureFilename);
    void Shutdown();
    void Render(ID3D11DeviceContext* deviceContext);

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

private:
    bool InitializeBuffers(ID3D11Device* device);
    void ShutdownBuffers();
    void RenderBuffers(ID3D11DeviceContext* deviceContext);
    bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename);
    void ReleaseTexture();

private:
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    int m_vertexCount, m_indexCount;
    TextureClass* m_Texture;
};

#endif