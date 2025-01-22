// Filename: modelclass.h
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include "RasterTek.h"
#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

#include "textureclass.h"
#include <fstream>
using namespace std;

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

    struct ModelType
    {
        float x, y, z;
        float tu, tv;
        float nx, ny, nz;
    };
public:
    ModelClass();
    ModelClass(const ModelClass&);
    ~ModelClass();

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* modelFilename, char* textureFilename, bool useNormal);
    void Shutdown();
    void Render(ID3D11DeviceContext* deviceContext);

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

    bool LoadModel(char*);
    void ReleaseModel();

private:
    bool InitializeBuffers(ID3D11Device* device, bool useTexture, bool useNormal, bool useModelFile);
    void ShutdownBuffers();
    void RenderBuffers(ID3D11DeviceContext* deviceContext);
    bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename);
    void ReleaseTexture();

    void StoreVertexBufferStride(unsigned int value) { m_vertexBufferStride = value; }
    unsigned int GetVertexBufferStride() { return m_vertexBufferStride; }

private:
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    int m_vertexCount, m_indexCount;
    TextureClass* m_Texture;

    ModelType* m_model;
    unsigned int m_vertexBufferStride;
};

#endif