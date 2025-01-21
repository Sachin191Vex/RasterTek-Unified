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
    m_matrixBuffer = nullptr;
}

ShaderClass::ShaderClass(const ShaderClass& other)
{
}

ShaderClass::~ShaderClass()
{
}

// --------------------------------------------------------------------------------------------------------------------
bool ShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;
    wchar_t vsFilename[128];
    wchar_t psFilename[128];
    int error;

    // Set the filename of the vertex shader.
    error = wcscpy_s(vsFilename, 128, L"./shaders/color.vs");
    if(error != 0) { return false; }

    // Set the filename of the pixel shader.
    error = wcscpy_s(psFilename, 128, L"./shaders/color.ps");
    if (error != 0) { return false; }

    // Initialize the vertex and pixel shaders.
    result = InitializeShader(device, hwnd, vsFilename, psFilename);
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
bool ShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
                              XMMATRIX projectionMatrix)
{
    bool result;

    // Set the shader parameters that it will use for rendering.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
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
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);\
        } else {\
            char* tmp_char_name; WCHAR2CHAR(filename, tmp_char_name);\
            MessageBox(hwnd, tmp_char_name, "Missing Shader File", MB_OK);\
        }\
        return false;\
    }\
}

bool ShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    unsigned int numElements;

    // Initialize the pointers this function will use to null.
    errorMessage = nullptr;

    // Step 1: Compile shaders -------------------------------------------------------------------------------------------
    // Compile the vertex shader code.
    ID3D10Blob* vertexShaderBuffer = nullptr;
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                                &vertexShaderBuffer, &errorMessage);
    CHECK_AND_RETURN_COMPILE_RESULT(result, vsFilename);

    // Compile the pixel shader code.
    ID3D10Blob* pixelShaderBuffer = nullptr;
    result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                                &pixelShaderBuffer, &errorMessage);
    CHECK_AND_RETURN_COMPILE_RESULT(result, psFilename);

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
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "COLOR";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    // Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
                                       vertexShaderBuffer->GetBufferSize(), &m_layout);
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
    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result)) { return false; }

    return true;
}

void ShaderClass::ShutdownShader()
{
    // Release the matrix constant buffer.
    if(m_matrixBuffer) {
        m_matrixBuffer->Release();
        m_matrixBuffer = nullptr;
    }

    // Release the layout.
    if(m_layout) {
        m_layout->Release();
        m_layout = nullptr;
    }

    // Release the pixel shader.
    if(m_pixelShader) {
        m_pixelShader->Release();
        m_pixelShader = nullptr;
    }

    // Release the vertex shader.
    if(m_vertexShader) {
        m_vertexShader->Release();
        m_vertexShader = nullptr;
    }

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

// The SetShaderVariables function exists to make setting the global variables in the shader easier.
// The matrices used in this function are created inside the ApplicationClass, after which this function is called
// to send them from there into the vertex shader during the Render function call.
bool ShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
                                           XMMATRIX projectionMatrix)
{
    HRESULT result;
    unsigned int bufferNumber;

    // Transpose matrices before sending them into the shader, this is a requirement for DirectX 11.
    worldMatrix = XMMatrixTranspose(worldMatrix);
    viewMatrix = XMMatrixTranspose(viewMatrix);
    projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // Lock the constant buffer so it can be written to.
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) { return false; }

    // Get a pointer to the data in the constant buffer.
    MatrixBufferType* dataPtr;
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // Copy the matrices into the constant buffer.
    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Finanly set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

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

    // Step 3: Render the triangle.
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}

// --------------------------------------------------------------------------------------------------------------------
