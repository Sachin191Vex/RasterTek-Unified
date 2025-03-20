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

#define MAX_DIFFUSE_LIGHTS 4

// Class name: ShaderClass
enum ShaderType { SHADER_COLOR, SHADER_TEXURE, SHADER_LIGHT };
typedef struct ShaderInfo {
    ShaderType type;
    WCHAR* vs_shader_file;
    char* vs_shader_name;
    WCHAR* ps_shader_file;
    char* ps_shader_name;
    unsigned int param_cnt;
} ShaderInfo;

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

    struct TextureConfigBufferType
    {
        unsigned int useTexture;
        XMFLOAT3 tmp;                  // Extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
    };

    struct LightConfigBufferType
    {
        unsigned int useAmbientLight;
        unsigned int useDiffuseLight;
        unsigned int useSpecularLight;
        unsigned int numDiffuseLights;
        // float padding;              // No padding needed
    };

    struct LightPosBufferType
    {
        XMFLOAT4 diffuseLightPosDir[MAX_DIFFUSE_LIGHTS];
        unsigned int numDiffuseLights;
        unsigned int isLightPos;
        unsigned int padding[2];
    };

    struct LightParamBufferType
    {
        // Paramaters for ambient light
        XMFLOAT4 ambientColor;

        // Paramaters for specular light
        XMFLOAT4 specularColor;

        // Paramaters for diffuse light
        XMFLOAT4 diffuseColor[MAX_DIFFUSE_LIGHTS];
        float specularPower;
        float padding[3];                  // Extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
    };

    struct CameraBufferType
    {
        XMFLOAT3 cameraPosition;
        unsigned int calcViewDirection;
    };

public:
    ShaderClass();
    ShaderClass(const ShaderClass&);
    ~ShaderClass();

    bool Initialize(ID3D11Device* device, HWND hwnd, bool useTexture, bool useAmbient, bool useDiffuse, bool useSpecular);
    void Shutdown();
    bool Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
                XMMATRIX projMatrix, ID3D11ShaderResourceView* texture,
                bool isLightPos, unsigned int numDiffuseLights, XMFLOAT3 lightPosDir[],
                bool useAmbient, XMFLOAT4 ambientCol,
                bool useDiffuse, XMFLOAT4 diffuseCol[],
                bool useSpecular, XMFLOAT3 cameraPos, XMFLOAT4 specularCol, float specularPow);

private:
    bool InitializeShader(ID3D11Device* device, HWND hwnd, bool useTexture, bool useAmbient, bool useDiffuse, bool useSpecular);

    bool SetShaderUsed(bool useTexture, bool useLighting);
    ShaderInfo GetShaderUsed();

    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);

    bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
                             XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
                             bool isLightPos, unsigned int numDiffuseLights, XMFLOAT3 lightPosDir[],
                             bool useAmbient, XMFLOAT4 ambientCol,
                             bool useDiffuse, XMFLOAT4 diffuseCol[],
                             bool useSpecular, XMFLOAT3 cameraPos, XMFLOAT4 specularCol, float specularPow);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
    ShaderInfo m_shader_info;

    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;

    ID3D11Buffer* m_matrixBuffer;
    ID3D11Buffer* m_textureConfigBuffer;
    ID3D11Buffer* m_lightConfigBuffer;
    ID3D11Buffer* m_lightPosBuffer;
    ID3D11Buffer* m_lightParamBuffer;
    ID3D11Buffer* m_cameraBuffer;
};

#endif