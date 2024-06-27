#version 450

#define CUSTOM_MATERIAL_PROPERTY_COUNT 16

layout(set = 0, binding = 0) uniform Scene {
    mat4 viewProjection;
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    vec2 cameraExtent;
    float time;
    float deltaTime;
};

layout(set = 0, binding = 1) uniform Mesh
{
    mat4 model;
    vec4 materialColor;
    vec4 materialProperties;
    float customMaterialProperties[CUSTOM_MATERIAL_PROPERTY_COUNT];
};

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBitangent;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) in vec3 inColor;
layout (location = 5) in vec3 inWorldPosition;

layout (location = 0) out vec4 outColor;

void main()
{
    vec3 N = normalize(inNormal);
    vec3 V = normalize(cameraPosition - inWorldPosition);

    float roughness = materialProperties.x;
    float metallic = materialProperties.y;

    vec3 matCol = materialColor.xyz * inColor.xyz;
    outColor = vec4(matCol, 1.0);
}