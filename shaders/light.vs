// Filename: light.vs

// DEFINES
#define MAX_DIFFUSE_LIGHTS 4

// GLOBALS
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer LightPositionBuffer
{
    float4 diffuseLightPosDir[MAX_DIFFUSE_LIGHTS];
    unsigned int numDiffuseLights;
    unsigned int isLightPos;
    unsigned int padding[2];
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
    unsigned int calcViewDirection;
};

// TYPEDEFS
// The normal vector is used for calculating the amount of light by using the angle between the direction of the normal and the direction of the light.
struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1;
    float3 diffuseLightDir[MAX_DIFFUSE_LIGHTS] : TEXCOORD2;
};

// Vertex Shader
PixelInputType LightVertexShader(VertexInputType input)
{
    unsigned int i;
    PixelInputType output;
    float4 worldPosition;

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the input color for the pixel shader to use
    output.color = input.color;

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

    // Calculate the position of the vertex in the world.
    worldPosition = mul(input.position, worldMatrix);

    // The position of all the lights in the world in relation to the vertex must be calculated, normalized
    // final directions then sent into the pixel shader
    // The direction of lights is specified then it can be used directly and sent to pixel shader
    for (i=0; i<numDiffuseLights; i++) {
        if (isLightPos) {
            // Determine the light positions based on the position of the lights and the position of the vertex in the world.
            output.diffuseLightDir[i] = diffuseLightPosDir[i].xyz - worldPosition.xyz;

            // Normalize the light position vectors
            output.diffuseLightDir[i] = normalize(output.diffuseLightDir[i]);
        } else {
            // Invert the light direction for calculations.
            output.diffuseLightDir[i] = -diffuseLightPosDir[i].xyz;
        }
    }

    // VideDirection calculations are needed if Specular lighting is used
    // The viewing direction is calculated here in the vertex shader. We calculate the world position of the vertex and
    // subtract that from the camera position to determine where we are viewing the scene from.
    // The final value is normalized and sent into the pixel shader.
    if (calcViewDirection) {
        // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
        output.viewDirection = cameraPosition.xyz - worldPosition.xyz;

        // Normalize the viewing direction vector.
        output.viewDirection = normalize(output.viewDirection);
    }

    return output;
}