// Filename: modelclass.cpp
#include "modelclass.h"

// --------------------------------------------------------------------------------------------------------------------
ModelClass::ModelClass()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_Texture = nullptr;
}

ModelClass::ModelClass(const ModelClass& other)
{
}

ModelClass::~ModelClass()
{
}

// --------------------------------------------------------------------------------------------------------------------
bool ModelClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* textureFilename)
{
    bool result;

    // Initialize the vertex and index buffers.
    result = InitializeBuffers(device);
    if (!result) { return false; }

    // Load the texture for this model.
    result = LoadTexture(device, deviceContext, textureFilename);
    if (!result) { return false; }

    return true;
}

void ModelClass::Shutdown()
{
    // Release the model texture.
    ReleaseTexture();

    // Shutdown the vertex and index buffers.
    ShutdownBuffers();

    return;
}

// This function to put the vertex and index buffers on the graphics pipeline so the color shader will be able to render them.
void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
    // Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
    RenderBuffers(deviceContext);

    return;
}

// --------------------------------------------------------------------------------------------------------------------
int ModelClass::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* ModelClass::GetTexture()
{
    return m_Texture->GetTexture();
}

// --------------------------------------------------------------------------------------------------------------------
bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
    VertexTypeColor* verticesColor;
    VertexTypeTexture* verticesTexture;
    unsigned long* indices;
    HRESULT result;

    // Set the number of vertices in the vertex array.
    m_vertexCount = 3;

    // Set the number of indices in the index array.
    m_indexCount = 3;

    // Step 1: Fill both the vertex and index array -------------------------------------------------------------------
    // The points are created in the clockwise order of drawing them. If you do this counter clockwise it will think the triangle is facing
    // the opposite direction and not draw it due to back face culling.
    // Always remember that the order in which you send your vertices to the GPU is very important.
    // The color is set here as well since it is part of the vertex description.
    // Load the vertex array with data.
    if (CHECK_RT_TEST_NUM(4)) {
        // Create the vertex array.
        verticesColor = new VertexTypeColor[m_vertexCount];
        if (!verticesColor) { return false; }

        verticesColor[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
        verticesColor[0].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

        verticesColor[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
        verticesColor[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

        verticesColor[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
        verticesColor[2].color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
    }
    if (CHECK_RT_TEST_NUM(5)) {
        // Create the vertex array.
        verticesTexture = new VertexTypeTexture[m_vertexCount];
        if (!verticesTexture) { return false; }

        verticesTexture[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
        verticesTexture[0].texture = XMFLOAT2(0.0f, 1.0f);

        verticesTexture[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
        verticesTexture[1].texture = XMFLOAT2(0.5f, 0.0f);

        verticesTexture[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
        verticesTexture[2].texture = XMFLOAT2(1.0f, 1.0f);
    }

    // Create and load the index array with data.
    indices = new unsigned long[m_indexCount];
    if (!indices) { return false; }

    indices[0] = 0;  // Bottom left.
    indices[1] = 1;  // Top middle.
    indices[2] = 2;  // Bottom right.

    // Step 2: Create the vertex buffer and index buffer. ----------------------------------------------------------------
    // First fill out a description of the buffer. In the description the ByteWidth (size of the buffer) and the BindFlags (type of buffer) 
    // are what you need to ensure are filled out correctly.
    // Second, fill out a subresource pointer which will point to either your vertex or index array you previously created.
    // With the description and subresource pointer you can call CreateBuffer using the D3D device and it will return a pointer to your new buffer.
    // Set up the description of the static vertex buffer.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    if (CHECK_RT_TEST_NUM(4)) { vertexBufferDesc.ByteWidth = sizeof(VertexTypeColor) * m_vertexCount; }
    if (CHECK_RT_TEST_NUM(5)) { vertexBufferDesc.ByteWidth = sizeof(VertexTypeTexture) * m_vertexCount; }
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    D3D11_SUBRESOURCE_DATA vertexData;
    if (CHECK_RT_TEST_NUM(4)) { vertexData.pSysMem = verticesColor; }
    if (CHECK_RT_TEST_NUM(5)) { vertexData.pSysMem = verticesTexture; }
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result)) { return false; }

    // Set up the description of the static index buffer.
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a  pointer to the index data.
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result)) { return false; }

    // Release the arrays now that the vertex and index buffers have been created and loaded.
    if (CHECK_RT_TEST_NUM(4)) { delete[] verticesColor; verticesColor = nullptr; }
    if (CHECK_RT_TEST_NUM(5)) { delete[] verticesTexture; verticesTexture = nullptr; }

    delete [] indices;
    indices = 0;

    return true;
}

void ModelClass::ShutdownBuffers()
{
    // Release the index buffer.
    if(m_indexBuffer) {
        m_indexBuffer->Release();
        m_indexBuffer = nullptr;
    }

    // Release the vertex buffer.
    if(m_vertexBuffer) {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }

    return;
}

// --------------------------------------------------------------------------------------------------------------------
// LoadTexture is a new private function that will create the texture object and then initialize it with the input file name provided.
// This function is called during initialization.
bool ModelClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    bool result;

    // Create and initialize the texture object.
    m_Texture = new TextureClass;

    result = m_Texture->Initialize(device, deviceContext, filename);
    if (!result) { return false; }

    return true;
}

// The ReleaseTexture function will release the texture object that was created and loaded during the LoadTexture function.
void ModelClass::ReleaseTexture()
{
    // Release the texture object.
    if (m_Texture) {
        m_Texture->Shutdown();
        delete m_Texture;
        m_Texture = nullptr;
    }

    return;
}

// --------------------------------------------------------------------------------------------------------------------
// THe purpose of this function is to set the vertex buffer and index buffer as active on the input assembler in the GPU.
// Once the GPU has an active vertex buffer it can then use the shader to render that buffer.
// This function also defines how those buffers should be drawn such as triangles, lines, fans, and so forth.
void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset.
	if (CHECK_RT_TEST_NUM(4)) { stride = sizeof(VertexTypeColor); }
	if (CHECK_RT_TEST_NUM(5)) { stride = sizeof(VertexTypeTexture); }
	offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

// --------------------------------------------------------------------------------------------------------------------