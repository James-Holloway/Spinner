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

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outTangent;
layout (location = 2) out vec3 outBitangent;
layout (location = 3) out vec2 outTexCoord;
layout (location = 4) out vec3 outColor;
layout (location = 5) out vec3 outWorldPosition;
BindShaderInstance
void main()
{
    // Position
    gl_Position = viewProjection * model * vec4(inPosition, 1.0f);
    outWorldPosition = (model * vec4(inPosition, 1.0f)).xyz;

    // TBN
    outNormal = normalize((model * vec4(inNormal, 0.0f)).xyz);
    outTangent = normalize((model * vec4(inTangent.xyz, 0.0f)).xyz);
    outBitangent = normalize(cross(outNormal, outTangent) * inTangent.w);

    // UV + color
    outTexCoord = inTexCoord;
    outColor = inColor;
}
