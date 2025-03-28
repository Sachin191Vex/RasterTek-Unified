// Filename: bitmapclass.cpp
#include "bitmapclass.h"
#include <fstream>
using namespace std;

// --------------------------------------------------------------------------------------------------------------------
BitmapClass::BitmapClass()
{
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
    m_Textures = nullptr;
}

BitmapClass::BitmapClass(const BitmapClass& other)
{
}

BitmapClass::~BitmapClass()
{
}

// --------------------------------------------------------------------------------------------------------------------
// if sprite_mode = true, filename will contain name of texture file to be loaded 
// othewise, file will be nam of the texture file and no animation is needed
bool BitmapClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, bool sprite_mode, char* filename, int renderX, int renderY)
{
    bool result;

    // In the Initialize function both the screen size and where the image gets rendered is stored.
    // These will be required for generating exact vertex locations during rendering.
    // Store the screen size.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Store where the bitmap should be rendered to.
    m_renderX = renderX;
    m_renderY = renderY;

    // Initialize the vertex and index buffer that hold the geometry for the bitmap quad.
    result = InitializeBuffers(device);
    if (!result) { return false; }

    // Initialize Animation related paramaters in sprite mode
    m_animate = (sprite_mode ? true : false);
    m_frameTime = 0;

    // Load the texture for this bitmap.
    result = LoadTextures(device, deviceContext, sprite_mode, filename);
    if (!result) { return false; }

    return true;
}

void BitmapClass::Shutdown()
{
    // Release the bitmap texture.
    ReleaseTextures();

    // Release the vertex and index buffers.
    ShutdownBuffers();

    return;
}

// --------------------------------------------------------------------------------------------------------------------
// Render puts the buffers of the 2D image on the video card. The UpdateBuffers function is called with the position parameters.
// If the position has changed since the last frame, it will then update the location of the vertices in the dynamic vertex buffer
// to the new location. If not, it will skip the UpdateBuffers function.After that the texture for the bitmap is set and the
// RenderBuffers function will prepare the final vertices / indices for rendering.
bool BitmapClass::Render(ID3D11DeviceContext* deviceContext)
{
    bool result;

    // Update the buffers if the position of the bitmap has changed from its original position.
    result = UpdateBuffers(deviceContext);
    if (!result) { return false; }

    // Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
    RenderBuffers(deviceContext);

    return true;
}

// The Update function takes in the frame time each frame. This will usually be around 16 - 17ms if
// you are running your program at 60fps. Each frame we add this time to the m_frameTime counter.
// If it reaches or passes the cycle time that was defined for this sprite, then we change the sprite
// to use the next texture in the array. We then reset the timer to start from zero again.
void BitmapClass::Update(float frameTime)
{
    // Increment the frame time each frame.
    m_frameTime += frameTime;

    // Check if the frame time has reached the cycle time.
    if (m_frameTime >= m_cycleTime) {
        // If it has then reset the frame time and cycle to the next sprite in the texture array.
        m_frameTime -= m_cycleTime;

        m_currentTexture++;

        // If we are at the last sprite texture then go back to the beginning of the texture array to the first texture again.
        if (m_currentTexture == m_textureCount) {
            m_currentTexture = 0;
        }
    }

    return;
}

// --------------------------------------------------------------------------------------------------------------------
// GetIndexCount returns the number of indexes for the 2D image. This will pretty much always be six.
int BitmapClass::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* BitmapClass::GetTexture()
{
    return m_Textures[m_currentTexture].GetTexture();
}

// --------------------------------------------------------------------------------------------------------------------
// InitializeBuffers is the function that is used to build the vertex and index buffer that will be used to draw the 2D image.
bool BitmapClass::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // The previous rendering location is first initialized to negative one. This will be an important variable that will
    // locate where it last drew this image. If the image location hasn't changed since last frame, then it won't modify
    // the dynamic vertex buffer which will save us some cycles.
    // Initialize the previous rendering position to negative one.
    m_prevPosX = -1;
    m_prevPosY = -1;

    // We set the vertices to six since we are making a square out of two triangles, so six points are needed. The indices will be the same.
    // Set the number of vertices in the vertex array.
    m_vertexCount = 6;

    // Set the number of indices in the index array.
    m_indexCount = m_vertexCount;

    // Create the vertex array.
    vertices = new VertexType[m_vertexCount];

    // Create the index array.
    indices = new unsigned long[m_indexCount];

    // Initialize vertex array to zeros at first.
    memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

    // Load the index array with data.
    for (i=0; i<m_indexCount; i++) { indices[i] = i; }

    // Here is the big change in comparison to the ModelClass. We are now creating a dynamic vertex buffer so we can modify the data inside
    // the vertex buffer each frame if we need to.
    // To make it dynamic we set Usage to D3D11_USAGE_DYNAMIC and CPUAccessFlags to D3D11_CPU_ACCESS_WRITE in the description.
    // Set up the description of the dynamic vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Now finally create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result)) { return false; }

    // We don't need to make the index buffer dynamic since the six indices will always point to the same six vertices
    // even though the coordinates of the vertex may change.
    // Set up the description of the index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result)) { return false; }

    // Release the arrays now that the vertex and index buffers have been created and loaded.
    delete [] vertices;
    vertices = 0;

    delete [] indices;
    indices = 0;

    return true;
}

void BitmapClass::ShutdownBuffers()
{
    RT_RELEASE_ID3D11_PTR(m_indexBuffer);
    RT_RELEASE_ID3D11_PTR(m_vertexBuffer);
    return;
}

// --------------------------------------------------------------------------------------------------------------------
// The UpdateBuffers function is called each frame to update the contents of the dynamic vertex buffer to re-position
// the 2D bitmap image on the screen if need be.
bool BitmapClass::UpdateBuffers(ID3D11DeviceContext* deviceContent)
{
    float left, right, top, bottom;
    VertexType* vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* dataPtr;
    HRESULT result;

    // If the position we are rendering this bitmap to hasn't changed then don't update the vertex buffer.
    if ((m_prevPosX == m_renderX) && (m_prevPosY == m_renderY)) { return true; }

    // If the rendering location has changed then store the new position and update the vertex buffer.
    m_prevPosX = m_renderX;
    m_prevPosY = m_renderY;

    // Create the vertex array.
    vertices = new VertexType[m_vertexCount];

    // Calculate the screen coordinates of the left side of the bitmap.
    left = (float)((m_screenWidth / 2) * -1) + (float)m_renderX;

    // Calculate the screen coordinates of the right side of the bitmap.
    right = left + (float)m_bitmapWidth;

    // Calculate the screen coordinates of the top of the bitmap.
    top = (float)(m_screenHeight / 2) - (float)m_renderY;

    // Calculate the screen coordinates of the bottom of the bitmap.
    bottom = top - (float)m_bitmapHeight;

    // Load the vertex array with data.
    // First triangle.
    vertices[0].position = XMFLOAT3(left, top, 0.0f);  // Top left.
    vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
    vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

    vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
    vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

    // Second triangle.
    vertices[3].position = XMFLOAT3(left, top, 0.0f);  // Top left.
    vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[4].position = XMFLOAT3(right, top, 0.0f);  // Top right.
    vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

    vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
    vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

    // Lock the vertex buffer.
    result = deviceContent->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) { return false; }

    // Get a pointer to the data in the constant buffer.
    dataPtr = (VertexType*)mappedResource.pData;

    // Copy the data into the vertex buffer.
    memcpy(dataPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

    // Unlock the vertex buffer.
    deviceContent->Unmap(m_vertexBuffer, 0);

    // Release the pointer reference.
    dataPtr = 0;

    // Release the vertex array as it is no longer needed.
    delete [] vertices;
    vertices = 0;

    return true;
}

// The RenderBuffers function sets up the vertex and index buffers on the gpu to be drawn by the shader.
void BitmapClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset.
    stride = sizeof(VertexType);
    offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

// The following function loads the texture that will be used for drawing the 2D image.
bool BitmapClass::LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, bool sprite_mode, char* filename)
{
    bool result;
    ifstream fin;
    int i, j;

    m_textureCount = 1;
    if (sprite_mode) {
        // Open the sprite info data file.
        fin.open(filename);
        if (fin.fail()) { return false; }

        // Read in the number of textures.
        fin >> m_textureCount;

        if (m_textureCount == 1) { m_animate = false; }
    }

    // Create and initialize the texture object array
    m_Textures = new TextureClass[m_textureCount];

    if (m_animate) {
        char input;
        char textureFilename[128];

        // Read to start of next line.
        fin.get(input);

        // Read in each texture file name.
        for (i=0; i<m_textureCount; i++) {
            j = 0;
            fin.get(input);
            while (input != '\n') {
                textureFilename[j] = input;
                j++;
                fin.get(input);
            }
            textureFilename[j] = '\0';

            // Once you have the filename then load the texture in the texture array.
            result = m_Textures[i].Initialize(device, deviceContext, textureFilename);
            if (!result) { return false; }
        }

        // Read in the cycle time.
        fin >> m_cycleTime;

        // Convert the integer milliseconds to float representation.
        m_cycleTime = m_cycleTime * 0.001f;

        // Close the file.
        fin.close();
    } else {
        result = m_Textures[0].Initialize(device, deviceContext, filename);
        if (!result) { return false; }
    }

    // Set the starting texture in the cycle to be the first one in the list.
    m_currentTexture = 0;

    // Get the dimensions of the first texture and use that as the dimensions of the 2D sprite images.
    m_bitmapWidth = m_Textures[m_currentTexture].GetWidth();
    m_bitmapHeight = m_Textures[m_currentTexture].GetHeight();

    return true;
}

void BitmapClass::ReleaseTextures()
{
    RT_SHUTDOWN_OBJ_PTR_ARR(m_Textures, m_textureCount);
    return;
}

// The SetRenderLocation function allows you to change where the bitmap image is being rendered on the screen using 2D coordinates.
void BitmapClass::SetRenderLocation(int x, int y)
{
    m_renderX = x;
    m_renderY = y;
    return;
}

// --------------------------------------------------------------------------------------------------------------------
