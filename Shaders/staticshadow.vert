#version 450

#define CUSTOM_MATERIAL_PROPERTY_COUNT 16

layout(set = 0, binding = 0) uniform Mesh
{
    mat4 model;
    vec4 materialColor;
    vec4 materialProperties;
    float customMaterialProperties[CUSTOM_MATERIAL_PROPERTY_COUNT];
};

#include "scene.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inTangent;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec2 inTexCoord;

layout (location = 0) out vec2 outTexCoord;

void main()
{
    // Position
    gl_Position = viewProjection * model * vec4(inPosition, 1.0f);

    // UV
    outTexCoord = inTexCoord;
}
