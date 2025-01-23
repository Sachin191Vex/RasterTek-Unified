// Filename: Shaderclass.h
#ifndef _ShaderCLASS_H_
#define _ShaderCLASS_H_

#include "RasterTek.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
using namespace DirectX;
using namespace std;

// Class name: ShaderClass
class ShaderClass
{
private:
    // Here is the definition of the cBuffer type that will be used with the vertex shader. This typedef must be exactly
    // the same as the one in the vertex shader as the model data needs to match the typedefs in the shader for proper rendering.
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

    struct LightBufferType
    {
        XMFLOAT4 ambientColor;
        XMFLOAT4 diffuseColor;
        XMFLOAT3 lightDirection;
        float specularPower;
        XMFLOAT4 specularColor;

        bool useTexture;
        bool useAmbientLight;
        bool useDiffuseLight;
        bool useSpecularLight;
        XMFLOAT3 tmp;				// Added extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
    };

    struct CameraBufferType
    {
        XMFLOAT3 cameraPosition;
        bool calcViewDirection;
        bool padding[3];			// Padding to make it multiple of 16
    };
public:
    ShaderClass();
    ShaderClass(const ShaderClass&);
    ~ShaderClass();

    bool Initialize(ID3D11Device* device, HWND hwnd, bool useTexture, bool useNormal, bool useSpecular);
    void Shutdown();
    bool Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
                XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
                XMFLOAT3 lightDirection,
                bool useAmbientLight, XMFLOAT4 ambientColor,
                bool useDiffuseLight, XMFLOAT4 diffuseColor,
                bool useSpecularLight, XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower);

private:
    bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, bool useTexture, bool useNormal, bool useSpecular);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);

    bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
                             XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
                             XMFLOAT3 lightDirection,
                             bool useAmbientLight, XMFLOAT4 ambientColor,
                             bool useDiffuseLight, XMFLOAT4 diffuseColor,
                             bool useSpecularLight, XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;

    ID3D11Buffer* m_matrixBuffer;
    ID3D11Buffer* m_lightBuffer;
    ID3D11Buffer* m_cameraBuffer;
};

#endif