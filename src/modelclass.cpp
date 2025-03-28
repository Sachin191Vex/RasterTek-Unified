// Filename: modelclass.cpp
#include "modelclass.h"

// --------------------------------------------------------------------------------------------------------------------
ModelClass::ModelClass()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_Texture = nullptr;
    m_model = nullptr;
}

ModelClass::ModelClass(const ModelClass& other)
{
}

ModelClass::~ModelClass()
{
}

// --------------------------------------------------------------------------------------------------------------------
bool ModelClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, CraftModel craftModel, char* modelFilename, char* textureFilename, bool useNormal)
{
    bool result;
    bool useTexture, useModelFile;

    useModelFile = ((strcmp(modelFilename,"") != 0) ? true : false);
    useTexture = ((strcmp(textureFilename, "") != 0) ? true : false);

    // Load in the model data.
    if (useModelFile) {
        result = LoadModel(modelFilename);
        if (!result) { return false; }
    }

    // Initialize the vertex and index buffers.
    result = InitializeBuffers(device, craftModel, useTexture, useNormal, useModelFile);
    if (!result) { return false; }

    // Load the texture for this model.
    if (useTexture) {
        result = LoadTexture(device, deviceContext, textureFilename);
        if (!result) { return false; }
    }

    return true;
}

void ModelClass::Shutdown()
{
    // Release the model texture.
    ReleaseTexture();

    // Shutdown the vertex and index buffers.
    ShutdownBuffers();

    // Release the model data.
    ReleaseModel();

    return;
}

// --------------------------------------------------------------------------------------------------------------------
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
    if (m_Texture == nullptr) { return nullptr; }
    return m_Texture->GetTexture();
}

// --------------------------------------------------------------------------------------------------------------------
bool ModelClass::InitializeBuffers(ID3D11Device* device, CraftModel crafModel, bool useTexture, bool useNormal, bool useModelFile)
{
    VertexTypeColor* verticesColor;
    VertexTypeTexture* verticesTexture;
    VertexTypeTextureLight* verticesTextureLight;
    unsigned long* indices;
    unsigned long stride;
    HRESULT result;

    // Step 1: Fill both the vertex and index array -------------------------------------------------------------------
    // The points are created in the clockwise order of drawing them. If you do this counter clockwise it will think the triangle is facing
    // the opposite direction and not draw it due to back face culling.
    // Always remember that the order in which you send your vertices to the GPU is very important.
    // The color is set here as well since it is part of the vertex description.
    // Load the vertex array with data.

    // If model file is not used, model data will be hardcoded during initialization
    // If model files is used, data should loaded before calling this functon
    if (!useModelFile) {
        // Set the number of vertices in the vertex array.
        m_vertexCount = 3;

        // Set the number of indices in the index array.
        m_indexCount = 3;

        // Create and load the index array with data.
        indices = new unsigned long[m_indexCount];
        if (!indices) { return false; }

        indices[0] = 0;  // Bottom left.
        indices[1] = 1;  // Top middle.
        indices[2] = 2;  // Bottom right.

        // Create vertices array and assigned values
        if (!useTexture && !useNormal) {
            // Create the vertex array.
            stride = sizeof(VertexTypeColor);
            verticesColor = new VertexTypeColor[m_vertexCount];
            if (!verticesColor) { return false; }

            verticesColor[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
            verticesColor[0].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

            verticesColor[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
            verticesColor[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
            if (crafModel == TRI_RED) { verticesColor[1].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); }
            if (crafModel == TRI_REDINC) { verticesColor[1].color = XMFLOAT4(0.3f, 0.0f, 0.0f, 1.0f); }


            verticesColor[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
            verticesColor[2].color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
            if (crafModel == TRI_RED) { verticesColor[2].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); }
            if (crafModel == TRI_REDINC) { verticesColor[2].color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); }
        } else if (useTexture && !useNormal) {
            // Create the vertex array.
            stride = sizeof(VertexTypeTexture);
            verticesTexture = new VertexTypeTexture[m_vertexCount];
            if (!verticesTexture) { return false; }

            verticesTexture[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
            verticesTexture[0].texture = XMFLOAT2(0.0f, 1.0f);

            verticesTexture[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
            verticesTexture[1].texture = XMFLOAT2(0.5f, 0.0f);

            verticesTexture[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
            verticesTexture[2].texture = XMFLOAT2(1.0f, 1.0f);
        } else if (useTexture && useNormal) {
            // Create the vertex array.
            stride = sizeof(VertexTypeTextureLight);
            verticesTextureLight = new VertexTypeTextureLight[m_vertexCount];
            if (!verticesTextureLight) { return false; }

            // Each vertex now has normals associated with it for lighting calculations. 
            // The normal is a line that is perpendicular to the face of the polygon so that the exact direction the face is pointing can be calculated.
            // For simplicity purposes I set the normal for each vertex along the Z axis by setting each Z component to -1.0f which makes the normal point towards the viewer.
            verticesTextureLight[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
            verticesTextureLight[0].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // Red vertex
            verticesTextureLight[0].texture = XMFLOAT2(0.0f, 1.0f);
            verticesTextureLight[0].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

            verticesTextureLight[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);    // Top middle.
            verticesTextureLight[1].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // Red vertex
            verticesTextureLight[1].texture = XMFLOAT2(0.5f, 0.0f);
            verticesTextureLight[1].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

            verticesTextureLight[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);   // Bottom right.
            verticesTextureLight[2].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // Red vertex
            verticesTextureLight[2].texture = XMFLOAT2(1.0f, 1.0f);
            verticesTextureLight[2].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
        } else {
            return false;
        }
    }
    else {
        // Create and load the index array with data.
        indices = new unsigned long[m_indexCount];
        if (!indices) { return false; }

        // Create the vertex array.
        stride = sizeof(VertexTypeTextureLight);
        verticesTextureLight = new VertexTypeTextureLight[m_vertexCount];
        if (!verticesTextureLight) { return false; }

        // Load the vertex array and index array with data.
        for (int i = 0; i < m_vertexCount; i++) {
            verticesTextureLight[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
            verticesTextureLight[i].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);       // Red vertex
            verticesTextureLight[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
            verticesTextureLight[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

            indices[i] = i;
        }
    }

    // Store stride value as it will be equired when we senf VertextDat to pipeline durng every Render pass
    StoreVertexBufferStride(stride);

    // Step 2: Create the vertex buffer and index buffer. ----------------------------------------------------------------
    // First fill out a description of the buffer. In the description the ByteWidth (size of the buffer) and the BindFlags (type of buffer) 
    // are what you need to ensure are filled out correctly.
    // Second, fill out a subresource pointer which will point to either your vertex or index array you previously created.
    // With the description and subresource pointer you can call CreateBuffer using the D3D device and it will return a pointer to your new buffer.
    // Set up the description of the static vertex buffer.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    if (!useTexture && !useNormal) { vertexBufferDesc.ByteWidth = sizeof(VertexTypeColor) * m_vertexCount; }
    else if (useTexture && !useNormal) { vertexBufferDesc.ByteWidth = sizeof(VertexTypeTexture) * m_vertexCount; }
    else if (useTexture && useNormal) { vertexBufferDesc.ByteWidth = sizeof(VertexTypeTextureLight) * m_vertexCount; }
    else { return false; }
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    D3D11_SUBRESOURCE_DATA vertexData;
    if (!useTexture && !useNormal) { vertexData.pSysMem = verticesColor; }
    else if (useTexture && !useNormal) { vertexData.pSysMem = verticesTexture; }
    else if (useTexture && useNormal) { vertexData.pSysMem = verticesTextureLight; }
    else { return false; }
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
    if (!useTexture && !useNormal) { delete[] verticesColor; verticesColor = nullptr; }
    else if (useTexture && !useNormal) { delete[] verticesTexture; verticesTexture = nullptr; }
    else if (useTexture && useNormal) { delete[] verticesTextureLight; verticesTextureLight = nullptr; }
    else { return false; }

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
    RT_SHUTDOWN_OBJ_PTR(m_Texture);

    return;
}

// --------------------------------------------------------------------------------------------------------------------
// LoadModel function which handles loading the model data from the text file into the m_model array variable.
// It opens the text file and reads in the vertex count first. After reading the vertex count it creates the ModelParamType
// array and then reads each line into the array.Both the vertex count and index count are now set in this function.
bool ModelClass::LoadModel(char* filename)
{
    ifstream fin;
    char input;
    int i;

    // Open the model file.
    fin.open(filename);

    // If it could not open the file then exit.
    if (fin.fail()) { return false; }

    // Read up to the value of vertex count.
    fin.get(input);
    while (input != ':') { fin.get(input); }

    // Read in the vertex count.
    fin >> m_vertexCount;

    // Set the number of indices to be the same as the vertex count.
    m_indexCount = m_vertexCount;

    // Create the model using the vertex count that was read in.
    m_model = new ModelParamType[m_vertexCount];

    // Read up to the beginning of the data.
    fin.get(input);
    while (input != ':') { fin.get(input); }
    fin.get(input);
    fin.get(input);

    // Read in the vertex data.
    for (i = 0; i < m_vertexCount; i++) {
        fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
        fin >> m_model[i].tu >> m_model[i].tv;
        fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
    }

    // Close the model file.
    fin.close();

    return true;
}

// The ReleaseModel function handles deleting the model data array.
void ModelClass::ReleaseModel()
{
    if (m_model) {
        delete[] m_model;
        m_model = nullptr;
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
    offset = 0;
    stride = GetVertexBufferStride();

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

// --------------------------------------------------------------------------------------------------------------------