#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define CUSTOM_MATERIAL_PROPERTY_COUNT 16

layout(set = 0, binding = 0) uniform Mesh
{
    mat4 model;
    vec4 materialColor;
    vec4 materialProperties;
    float customMaterialProperties[CUSTOM_MATERIAL_PROPERTY_COUNT];
};

layout(set = 0, binding = 1) uniform sampler2D mainTexture;

#include "scene.glsl"
#include "lighting.glsl"

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

    vec3 matCol = materialColor.rgb * inColor.rgb;
    vec4 texColor = texture(mainTexture, inTexCoord);
    matCol *= pow(texColor.rgb, vec3(2.2f));

    // Use PBR lighting
    vec3 color = CalculateLighting(inWorldPosition, V, N, metallic, roughness, matCol);
    outColor = vec4(color, texColor.a);
}