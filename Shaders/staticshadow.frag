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

layout (location = 0) in vec2 inTexCoord;

void main()
{
    vec4 texColor = texture(mainTexture, inTexCoord);
    if (texColor.a < 0.65)
    {
        discard;
    }
}
