// Filename: color.vs
// Vertex shaders are small programs that are written mainly for transforming the vertices from the vertex buffer into 3D space.

// --------------------------------------------------------------------------------------------------------------------
// GLOBALS - These globals in the shader can be modified externally from your C++ code.
// Put most globals in buffer object types called "cbuffer" even if it is just a single global variable.
// Logically organizing these buffers is important for efficient execution of shaders as well as how the graphics card will store the buffers.
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// --------------------------------------------------------------------------------------------------------------------
// TYPEDEFS - Defines input paramaters and output return values from vertex shader to Pixel shader
struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


// --------------------------------------------------------------------------------------------------------------------
// Vertex Shader
// Vertex shaders transform the vertices from the vertex buffer into 3D space. It does calculations such as calculating normals for each vertex.
// The vertex shader program will be called by the GPU for each vertex it needs to process.

PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the input color for the pixel shader to use.
    output.color = input.color;

    return output;
}
// --------------------------------------------------------------------------------------------------------------------
