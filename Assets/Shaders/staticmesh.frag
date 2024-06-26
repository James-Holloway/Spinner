#version 450

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
};

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBitangent;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) out vec3 inColor;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(inNormal, 1.0);
}