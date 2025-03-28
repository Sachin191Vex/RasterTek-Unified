////////////////////////////////////////////////////////////////////////////////
// Filename: Shaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "RasterTek.h"
#include "shaderclass.h"

// --------------------------------------------------------------------------------------------------------------------
ShaderClass::ShaderClass()
{
    m_vertexShader = nullptr;
    m_pixelShader = nullptr;
    m_layout = nullptr;
    m_sampleState = nullptr;
    
    m_matrixBuffer = nullptr;
    m_textureConfigBuffer = nullptr;
    m_lightConfigBuffer = nullptr;
    m_lightDiffuseParamBuffer = nullptr;
    m_lightAmbientSpecularParamBuffer = nullptr;
    m_cameraBuffer = nullptr;
}

ShaderClass::ShaderClass(const ShaderClass& other)
{
}

ShaderClass::~ShaderClass()
{
}

// --------------------------------------------------------------------------------------------------------------------
bool ShaderClass::Initialize(ID3D11Device* device, HWND hwnd, bool useTexture, bool useAmbient, bool useDiffuse, bool useSpecular)
{
    bool result;

    auto useLighting = useAmbient || useDiffuse || useSpecular;
    result = SetShaderUsed(useTexture, useLighting);
    if (!result) { return false; }

    // Initialize the vertex and pixel shaders.
    result = InitializeShader(device, hwnd, useTexture, useAmbient, useDiffuse, useSpecular);
    if(!result) { return false; }

    return true;
}

void ShaderClass::Shutdown()
{
    // Shutdown the vertex and pixel shaders as well as the related objects.
    ShutdownShader();

    return;
}

// --------------------------------------------------------------------------------------------------------------------
bool ShaderClass::SetShaderUsed(bool useTexture, bool useLighting)
{
    bool status = true;

    if (!useTexture && !useLighting) {
        m_shader_info.type = SHADER_COLOR;
        m_shader_info.vs_shader_file = L"../shaders/color.vs";
        m_shader_info.vs_shader_name = "ColorVertexShader";
        m_shader_info.ps_shader_file = L"../shaders/color.ps";
        m_shader_info.ps_shader_name = "ColorPixelShader";
        m_shader_info.param_cnt = 2;
    }
    else if (useTexture && !useLighting) {
        m_shader_info.type = SHADER_TEXURE;
        m_shader_info.vs_shader_file = L"../shaders/texture.vs";
        m_shader_info.vs_shader_name = "TextureVertexShader";
        m_shader_info.ps_shader_file = L"../shaders/texture.ps";
        m_shader_info.ps_shader_name = "TexturePixelShader";
        m_shader_info.param_cnt = 2;
    }
    else if (useTexture && useLighting) {
        m_shader_info.type = SHADER_LIGHT;
        m_shader_info.vs_shader_file = L"../shaders/light.vs";
        m_shader_info.vs_shader_name = "LightVertexShader";
        m_shader_info.ps_shader_file = L"../shaders/light.ps";
        m_shader_info.ps_shader_name = "LightPixelShader";
        m_shader_info.param_cnt = 4;
    }
    else {
        // ToDo: support ligting shader with color and add command line options to override test lighting defaults
        status = false;
    }

    return(status);
}

ShaderInfo ShaderClass::GetShaderUsed()
{
    return m_shader_info;
}

// --------------------------------------------------------------------------------------------------------------------
bool ShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
                         XMMATRIX projMatrix, ID3D11ShaderResourceView* texture,
                         XMFLOAT3 cameraPos,
                         bool useAmbient,  XMFLOAT4 ambientCol,
                         bool useDiffuse, unsigned int numDiffuseLights, XMFLOAT4 diffuseCol[],
                         bool isLightPos, XMFLOAT3 lightPosDir[],
                         bool useSpecular, XMFLOAT4 specularCol, float specularPow)
{
    bool result;

    // Set the shader parameters that it will use for rendering.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projMatrix, texture,
                                 cameraPos,
                                 useAmbient, ambientCol,
                                 useDiffuse, numDiffuseLights, diffuseCol,
                                 isLightPos, lightPosDir,
                                 useSpecular, specularCol, specularPow);
    if (!result) { return false; }

    // Now render the prepared buffers with the shader.
    RenderShader(deviceContext, indexCount);

    return true;
}

// --------------------------------------------------------------------------------------------------------------------
// If the shader failed to compile it should have writen something to the error message.
// If there was  nothing in the error message then it simply could not find the shader file itself.
#define CHECK_AND_RETURN_COMPILE_RESULT(result, filename) {\
    if (FAILED(result)) {\
        if (errorMessage) {\
            OutputShaderErrorMessage(errorMessage, hwnd, filename);\
        } else {\
            char* tmp_char_name; WCHAR2CHAR(filename, tmp_char_name);\
            MessageBox(hwnd, tmp_char_name, "Missing Shader File", MB_OK);\
        }\
        return false;\
    }\
}

#define CREATE_CBUFFER(cbuffer, cbuffer_type) {\
    D3D11_BUFFER_DESC cBufferDesc;\
    cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;\
    cBufferDesc.ByteWidth = sizeof(cbuffer_type); /* Check here that size is multiple of 16 */\
    assert((cBufferDesc.ByteWidth % 16) == 0); \
    cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;\
    cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;\
    cBufferDesc.MiscFlags = 0;\
    cBufferDesc.StructureByteStride = 0;\
    /* Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.*/\
    result = device->CreateBuffer(&cBufferDesc, NULL, &cbuffer);\
    if (FAILED(result)) { return false; }\
}

bool ShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, bool useTexture, bool useAmbient, bool useDiffuse, bool useSpecular)
{
    HRESULT result;

    // Initialize paramaters
    auto useLighting = useAmbient || useDiffuse || useSpecular;

    // Initialize the pointers this function will use to null.
    ID3D10Blob* errorMessage = nullptr;

    // Step 1: Compile shaders -------------------------------------------------------------------------------------------
    auto shader_info = GetShaderUsed();
#if _DEBUG
    // UINT compilerFlag1 = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_DEBUG;  // To debug using Visual Studio Graphics Debug
    UINT compilerFlag1 = D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;  // To debug using RenderDoc
#else
    UINT compilerFlag1 = D3D10_SHADER_ENABLE_STRICTNESS;
#endif

    // Compile the vertex shader code.
    ID3D10Blob* vertexShaderBuffer = nullptr;
    result = D3DCompileFromFile(shader_info.vs_shader_file, NULL, NULL, shader_info.vs_shader_name, "vs_5_0", compilerFlag1, 0,
                                &vertexShaderBuffer, &errorMessage);
    CHECK_AND_RETURN_COMPILE_RESULT(result, shader_info.vs_shader_file);

    // Compile the pixel shader code.
    ID3D10Blob* pixelShaderBuffer = nullptr;
    result = D3DCompileFromFile(shader_info.ps_shader_file, NULL, NULL, shader_info.ps_shader_name, "ps_5_0", compilerFlag1, 0,
                                &pixelShaderBuffer, &errorMessage);
    CHECK_AND_RETURN_COMPILE_RESULT(result, shader_info.ps_shader_file);

    // Step 2: Create shader objects/sblobs ------------------------------------------------------------------------------
    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result)) { return false; }

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result)) { return false; }

    // Step 3: Define inputs to vertex shader ----------------------------------------------------------------------------
    // Create the vertex input layout description.
    // This setup needs to match the VertexType stucture in the ModelClass and in the shader.
    unsigned int param_num = 0;
    D3D11_INPUT_ELEMENT_DESC* polygonLayout = new D3D11_INPUT_ELEMENT_DESC[shader_info.param_cnt];
    polygonLayout[param_num].SemanticName = "POSITION";
    polygonLayout[param_num].SemanticIndex = 0;
    polygonLayout[param_num].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[param_num].InputSlot = 0;
    polygonLayout[param_num].AlignedByteOffset = 0;
    polygonLayout[param_num].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[param_num].InstanceDataStepRate = 0;
    param_num++;

    if ((shader_info.type == SHADER_COLOR) || (shader_info.type == SHADER_LIGHT)) {
        polygonLayout[param_num].SemanticName = "COLOR";
        polygonLayout[param_num].SemanticIndex = 0;
        polygonLayout[param_num].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        polygonLayout[param_num].InputSlot = 0;
        polygonLayout[param_num].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        polygonLayout[param_num].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        polygonLayout[param_num].InstanceDataStepRate = 0;
        param_num++;
    }

    if ((shader_info.type == SHADER_TEXURE) || (shader_info.type == SHADER_LIGHT)) {
        polygonLayout[param_num].SemanticName = "TEXCOORD";
        polygonLayout[param_num].SemanticIndex = 0;
        polygonLayout[param_num].Format = DXGI_FORMAT_R32G32_FLOAT;
        polygonLayout[param_num].InputSlot = 0;
        polygonLayout[param_num].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        polygonLayout[param_num].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        polygonLayout[param_num].InstanceDataStepRate = 0;
        param_num++;
    }

    if (shader_info.type == SHADER_LIGHT) {
        polygonLayout[param_num].SemanticName = "NORMAL";
        polygonLayout[param_num].SemanticIndex = 0;
        polygonLayout[param_num].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        polygonLayout[param_num].InputSlot = 0;
        polygonLayout[param_num].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        polygonLayout[param_num].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        polygonLayout[param_num].InstanceDataStepRate = 0;
        param_num++;
    }

    // Get a count of the elements in the layout.
    // numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    result = device->CreateInputLayout(polygonLayout, shader_info.param_cnt, vertexShaderBuffer->GetBufferPointer(),
                                       vertexShaderBuffer->GetBufferSize(), &m_layout);
    delete[] polygonLayout;
    if (FAILED(result)) { return false; }

    // Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Step 4: Define buffer to pass constats to the shader --------------------------------------------------------------
    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    // The buffer usage needs to be set to dynamic since we will be updating it each frame.
    // The bind flags indicate that this buffer will be a constant buffer.
    // The CPU access flags need to match up with the usage so it is set to D3D11_CPU_ACCESS_WRITE.
    CREATE_CBUFFER(m_matrixBuffer, MatrixBufferType);
    if (useTexture) { CREATE_CBUFFER(m_textureConfigBuffer, TextureConfigBufferType); }

    // Step 5: Setup the sampler state description and then can be passed to the pixel shader after. ---------------------
    // The most important element of the texture sampler description is Filter. Filter will determine how it decides
    // which pixels will be used or combined to create the final look of the texture on the polygon face. 
    // AddressU and AddressV are set to Wrap which ensures that the coordinates stay between 0.0f and 1.0f.
    // Anything outside of that wraps around and is placed between 0.0f and 1.0f.
    // All other settings for the sampler state description are defaults.
    if (useTexture) {
        // Create a texture sampler state description.
        D3D11_SAMPLER_DESC samplerDesc;
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.BorderColor[0] = 0;
        samplerDesc.BorderColor[1] = 0;
        samplerDesc.BorderColor[2] = 0;
        samplerDesc.BorderColor[3] = 0;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        // Create the texture sampler state.
        result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
        if (FAILED(result)) { return false; }
    }

    // Step 6: Setup the light constant buffer description which will handle the diffuse light color and light direction.
    // Pay attention to the size of the constant buffers, if they are not multiples of 16 you need to pad extra space on
    // to the end of them or the CreateBuffer function will fail. In this case the constant buffer is 28 bytes with 4 bytes 
    // padding to make it 32.
    // Setup the description of the light dynamic constant buffer that is in the pixel shader.
    // Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
    if (useLighting) {
        CREATE_CBUFFER(m_lightConfigBuffer, LightConfigBufferType);
        CREATE_CBUFFER(m_lightDiffuseParamBuffer, LightDiffuseParamBufferType);
        CREATE_CBUFFER(m_lightAmbientSpecularParamBuffer, LightAmbientSpecularParamBufferType);

        // Setup the description of the camera dynamic constant buffer that is in the vertex shader use for specualr lighting calculations
        if (useSpecular) { CREATE_CBUFFER(m_cameraBuffer, CameraBufferType); }
    }

    return true;
}

#define RELEASE_ID3D11_PTR(ID3D11_ptr) {\
    if (ID3D11_ptr) {\
        ID3D11_ptr->Release();\
        ID3D11_ptr = nullptr;\
    }\
}

void ShaderClass::ShutdownShader()
{
    // Release the created constant buffers.
    RELEASE_ID3D11_PTR(m_cameraBuffer);

    RELEASE_ID3D11_PTR(m_lightAmbientSpecularParamBuffer);
    RELEASE_ID3D11_PTR(m_lightDiffuseParamBuffer);
    RELEASE_ID3D11_PTR(m_lightConfigBuffer);

    RELEASE_ID3D11_PTR(m_textureConfigBuffer);
    RELEASE_ID3D11_PTR(m_matrixBuffer);

    // Release the created sampler state, input layout and shader buffers
    RELEASE_ID3D11_PTR(m_sampleState);
    RELEASE_ID3D11_PTR(m_layout);
    RELEASE_ID3D11_PTR(m_pixelShader);
    RELEASE_ID3D11_PTR(m_vertexShader);

    return;
}

// --------------------------------------------------------------------------------------------------------------------
void ShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long long bufferSize, i;
    ofstream fout;

    // Get a pointer to the error message text buffer.
    compileErrors = (char*)(errorMessage->GetBufferPointer());

    // Get the length of the message.
    bufferSize = errorMessage->GetBufferSize();

    // Open a file to write the error message to.
    fout.open("shader-error.txt");

    // Write out the error message.
    for(i=0; i<bufferSize; i++) {
        fout << compileErrors[i];
    }

    // Close the file.
    fout.close();

    // Release the error message.
    errorMessage->Release();
    errorMessage = 0;

    // Pop a message up on the screen to notify the user to check the text file for compile errors.
    char* tmp_char_name; WCHAR2CHAR(shaderFilename, tmp_char_name);
    MessageBox(hwnd, "Error compiling shader.  Check shader-error.txt for message.", tmp_char_name, MB_OK);

    return;
}

#define PRE_CBUFFER_UPDATE(cbuffer, data_type, data_ptr) {\
    /* Lock the constant buffer so it can be written to. */\
    result = deviceContext->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);\
    if (FAILED(result)) { return false; }\
    /* Get a pointer to the data in the constant buffer.*/\
    data_ptr = (data_type*)mappedResource.pData;\
}
#define POST_CBUFFER_UPDATE(cbuffer, vs_or_ps_SetConstantBuffers_func, cbuff_num_var) {\
    /* Unlock the constant buffer.*/\
    deviceContext->Unmap(cbuffer, 0);\
    /* Now set the camera cbuffer in the approapriate shader with the updated values.*/\
    deviceContext->vs_or_ps_SetConstantBuffers_func(cbuff_num_var++, 1, &cbuffer);\
}

// The SetShaderVariables function exists to make setting the global variables in the shader easier.
// The matrices used in this function are created inside the ApplicationClass, after which this function is called
// to send them from there into the vertex shader during the Render function call.
bool ShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
                                      XMMATRIX projMatrix, ID3D11ShaderResourceView* texture,
                                      XMFLOAT3 cameraPos,
                                      bool useAmbient, XMFLOAT4 ambientCol,
                                      bool useDiffuse, unsigned int numDiffuseLights, XMFLOAT4 diffuseCol[],
                                      bool isLightPos, XMFLOAT3 lightPosDir[],
                                      bool useSpecular, XMFLOAT4 specularCol, float specularPow)
{
    HRESULT result;
    bool useTexture = false;
    bool useLighting = false;
    unsigned int VS_bufferNum = 0;
    unsigned int PS_bufferNum = 0;
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    // Step 0: Initialize local varaibles
    if (texture != nullptr) { useTexture = true; }
    useLighting = useAmbient || useDiffuse || useSpecular;

    // Step 1: Transpose matrices before sending them into the shader, this is a requirement for DirectX 11.
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projMatrix = XMMatrixTranspose(projMatrix);

    // Step 2: Copy updated metrics (constant buffers) for VS shader to use
    MatrixBufferType* dataPtr;
    PRE_CBUFFER_UPDATE(m_matrixBuffer, MatrixBufferType, dataPtr);
    // Copy the matrices into the constant buffer.
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projMatrix;
    POST_CBUFFER_UPDATE(m_matrixBuffer, VSSetConstantBuffers, VS_bufferNum);

    // Step 3: Set shader resource view if specified
    if (useTexture) {
        deviceContext->PSSetShaderResources(0, 1, &texture);

        // Update Texture config paramaters (constant buffers) for PS shader to use
        TextureConfigBufferType* dataPtr;
        PRE_CBUFFER_UPDATE(m_textureConfigBuffer, TextureConfigBufferType, dataPtr);
        // Copy the camera position into the constant buffer.
        dataPtr->useTexture = true;
        dataPtr->paddingTCB = XMFLOAT3(0.0f, 0.0f, 0.0f);
        POST_CBUFFER_UPDATE(m_textureConfigBuffer, PSSetConstantBuffers, PS_bufferNum);
    }

    // Step 4: Set up the light constant buffer is setup the same way as the matrix constant buffer.
    // We first lock the buffer and get a pointer to it. After that we set the diffuse color and light direction using that pointer.
    // Once the data is set, we unlock the buffer and then set it in the pixel shader.
    // Note that we use the PSSetConstantBuffers function instead of VSSetConstantBuffers since this is a pixel shader buffer we are setting.
    if (useLighting) {
        // Update Diffsue lighting paramaters (constant buffers) for VS shader to use
        LightDiffuseParamBufferType* dataPtr;
        PRE_CBUFFER_UPDATE(m_lightDiffuseParamBuffer, LightDiffuseParamBufferType, dataPtr);
        dataPtr->numDiffuseLights = numDiffuseLights;
        dataPtr->isDiffuseLightPos = isLightPos;
        for (auto i=0; i<numDiffuseLights; i++) {
            dataPtr->diffuseLightPosDir[i].x = lightPosDir[i].x;
            dataPtr->diffuseLightPosDir[i].y = lightPosDir[i].y;
            dataPtr->diffuseLightPosDir[i].z = lightPosDir[i].z;
            dataPtr->diffuseColor[i] = diffuseCol[i];
        }
        POST_CBUFFER_UPDATE(m_lightDiffuseParamBuffer, VSSetConstantBuffers, VS_bufferNum);

        // Update Lighting config paramaters (constant buffers) for PS shader to use
        LightConfigBufferType* dataPtr2;
        PRE_CBUFFER_UPDATE(m_lightConfigBuffer, LightConfigBufferType, dataPtr2);
        // Copy the lighting configuration variables into the constant buffer.
        dataPtr2->useAmbientLight = useAmbient;
        dataPtr2->useDiffuseLight = useDiffuse;
        dataPtr2->useSpecularLight = useSpecular;
        POST_CBUFFER_UPDATE(m_lightConfigBuffer, PSSetConstantBuffers, PS_bufferNum);

        // Update diffuse lighting paramaters (constant buffers) for PS shader to use
        PRE_CBUFFER_UPDATE(m_lightDiffuseParamBuffer, LightDiffuseParamBufferType, dataPtr);
        POST_CBUFFER_UPDATE(m_lightDiffuseParamBuffer, PSSetConstantBuffers, PS_bufferNum);

        // Update Lighting paramaters (constant buffers) for PS shader to use
        LightAmbientSpecularParamBufferType* dataPtr3;
        PRE_CBUFFER_UPDATE(m_lightAmbientSpecularParamBuffer, LightAmbientSpecularParamBufferType, dataPtr3);
        // Copy the camera position into the constant buffer.
        // Copy the lighting variables into the constant buffer.
        dataPtr3->ambientColor = ambientCol;
        dataPtr3->specularColor = specularCol;
        dataPtr3->specularPower = specularPow;
        POST_CBUFFER_UPDATE(m_lightAmbientSpecularParamBuffer, PSSetConstantBuffers, PS_bufferNum);

        if (useSpecular) {
            // Update Camera paramaters (constant buffers) for PS shader to use
            CameraBufferType* dataPtr;
            PRE_CBUFFER_UPDATE(m_cameraBuffer, CameraBufferType, dataPtr);
            // Copy the camera position into the constant buffer.
            dataPtr->cameraPosition = cameraPos;
            dataPtr->calcViewDirection = true;
            POST_CBUFFER_UPDATE(m_cameraBuffer, VSSetConstantBuffers, VS_bufferNum);
        }
    }

    return true;
}

// The render function sets the shader parameters and then draws the prepared model vertices using the shader.
// The first step in this function is to set our input layout to active in the input assembler. This lets the GPU
// know the format of the data in the vertex buffer.
// The second step is to set the vertex shader and pixel shader we will be using to render this vertex buffer.
// The third step is to issue a draw call
void ShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // Step 1: Set the vertex input layout.
    deviceContext->IASetInputLayout(m_layout);

    // Step 2: Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // Step 3: Set the sampler state in the pixel shader.
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // Step 4: Render the triangle.
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}

// --------------------------------------------------------------------------------------------------------------------
