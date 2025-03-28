// Filename: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "RasterTek.h"
#include "textureclass.h"

// --------------------------------------------------------------------------------------------------------------------
TextureClass::TextureClass()
{
    m_targaData = nullptr;
    m_texture = nullptr;
    m_textureView = nullptr;
}

TextureClass::TextureClass(const TextureClass& other)
{
}

TextureClass::~TextureClass()
{
}

// --------------------------------------------------------------------------------------------------------------------
bool TextureClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    bool result;
    HRESULT hResult;
    unsigned int rowPitch;

    // Load the targa image data into memory.
    result = LoadTarga32Bit(filename);
    if(!result) { return false; }

    // Setup the description of the texture.
    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Height = m_height;
    textureDesc.Width = m_width;
    textureDesc.MipLevels = 0;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    // Create the empty texture.
    hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
    if (FAILED(hResult)) { return false; }

    // Set the row pitch of the targa image data.
    rowPitch = (m_width * 4) * sizeof(unsigned char);

    // Copy the targa image data into the texture.
    // Here we use UpdateSubresource to actually do the copying of the Targa data array into the DirectX texture.
    // We could have used Map and Unmap to copy our texture data. And in fact, using Map and Unmap is generally a lot quicker than using UpdateSubresource,
    // however both loading methods have specific purposes and you need to choose correctly which one to use for performance reasons.
    // The recommendation is that you use Map and Unmap for data that is going to be reloaded each frame or on a very regular basis.
    // And you should use UpdateSubresource for something that will be loaded once or that gets loaded rarely during loading sequences.
    // The reason being is that UpdateSubresource puts the data into higher speed memory that gets cache retention preference since 
    // it knows you aren't going to remove or reload it anytime soon.
    // We let DirectX also know by using D3D11_USAGE_DEFAULT when we are going to load using UpdateSubresource.
    // And Map and Unmap will put the data into memory locations that will not be cached as DirectX is expecting that data to be overwritten shortly.
    // And that is why we use D3D11_USAGE_DYNAMIC to notify DirectX that this type of data is transient.
    deviceContext->UpdateSubresource(m_texture, 0, NULL, m_targaData, rowPitch, 0);

    // Setup the shader resource view description.
    // After the texture is loaded, we create a shader resource view which allows us to have a pointer to set the texture in shaders.
    // In the description we also set two important Mipmap variables which will give us the full range of Mipmap levels for high quality texture rendering at any distance.
    // Once the shader resource view is created, we call GenerateMips and it creates the Mipmaps for us, however if you want you can load your own Mipmap levels in manually
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    // Create the shader resource view for the texture.
    hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
    if (FAILED(hResult)) { return false; }

    // Generate mipmaps for this texture.
    deviceContext->GenerateMips(m_textureView);

    // Release the targa image data now that the image data has been loaded into the texture.
    delete [] m_targaData;
    m_targaData = nullptr;

    return true;
}

void TextureClass::Shutdown()
{
    RT_RELEASE_ID3D11_PTR(m_textureView);
    RT_RELEASE_ID3D11_PTR(m_texture);
    RT_RELEASE_OBJ_PTR_ARR(m_targaData);
    return;
}

// --------------------------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* TextureClass::GetTexture()
{
    return m_textureView;
}

// Targa images are stored upside down and need to be flipped before using. So here we will open the file, read it into an array,
// and then take that array data and load it into the m_targaData array in the correct order.
// Note we are purposely only dealing with 32-bit Targa files that have alpha channels, this function will reject Targa's that are saved as 24-bit.
bool TextureClass::LoadTarga32Bit(char* filename)
{
    int error, bpp, imageSize, index, i, j, k;
    FILE* filePtr;
    unsigned int count;
    TargaHeader targaFileHeader;
    unsigned char* targaImage;

    // Open the targa file for reading in binary.
    error = fopen_s(&filePtr, filename, "rb");
    if(error != 0) { return false; }

    // Read in the file header.
    count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
    if(count != 1) { return false; }

    // Get the important information from the header.
    m_height = (int)targaFileHeader.height;
    m_width = (int)targaFileHeader.width;
    bpp = (int)targaFileHeader.bpp;

    // Check that it is 32 bit and not 24 bit.
    if(bpp != 32) { return false; }

    // Calculate the size of the 32 bit image data.
    imageSize = m_width * m_height * 4;

    // Allocate memory for the targa image data.
    targaImage = new unsigned char[imageSize];

    // Read in the targa image data.
    count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
    if(count != imageSize) { return false; }

    // Close the file.
    error = fclose(filePtr);
    if(error != 0) { return false; }

    // Allocate memory for the targa destination data.
    m_targaData = new unsigned char[imageSize];

    // Initialize the index into the targa destination data array.
    index = 0;

    // Initialize the index into the targa image data.
    k = (m_width * m_height * 4) - (m_width * 4);

    // Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down and also is not in RGBA order.
    for(j=0; j<m_height; j++) {
        for(i=0; i<m_width; i++) {
            m_targaData[index + 0] = targaImage[k + 2];  // Red.
            m_targaData[index + 1] = targaImage[k + 1];  // Green.
            m_targaData[index + 2] = targaImage[k + 0];  // Blue
            m_targaData[index + 3] = targaImage[k + 3];  // Alpha

            // Increment the indexes into the targa data.
            k += 4;
            index += 4;
        }

        // Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
        k -= (m_width * 8);
    }

    // Release the targa image data now that it was copied into the destination array.
    delete [] targaImage;
    targaImage = 0;

    return true;
}

int TextureClass::GetWidth()
{
    return m_width;
}

int TextureClass::GetHeight()
{
    return m_height;
}

// --------------------------------------------------------------------------------------------------------------------
