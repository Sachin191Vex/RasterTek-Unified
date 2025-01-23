// Filename: light.vs
// GLOBALS
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
    bool calcViewDirection;
    bool padding[3];
};

// TYPEDEFS
// The normal vector is used for calculating the amount of light by using the angle between the direction of the normal and the direction of the light.
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1;
};

// Vertex Shader
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

    // Calculate the normal vector against the world matrix only.
    // The normal vector for this vertex is calculated in world space and then normalized before 
    // being sent as input into the pixel shader.
    // We only calculate against the world matrix as we are just trying to find the lighting values in the 3D world space.
    // Note that sometimes these normals need to be re-normalized inside the pixel shader due to the interpolation that occurs
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    // The viewing direction is calculated here in the vertex shader. We calculate the world position of the vertex and
    // subtract that from the camera position to determine where we are viewing the scene from.
    // The final value is normalized and sent into the pixel shader.
    if (calcViewDirection) {
        // Calculate the position of the vertex in the world.
        worldPosition = mul(input.position, worldMatrix);

        // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
        output.viewDirection = cameraPosition.xyz - worldPosition.xyz;

        // Normalize the viewing direction vector.
        output.viewDirection = normalize(output.viewDirection);
    }

    return output;
}