#version 450

layout(set = 0, binding = 0) uniform UBOScene {
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    vec4 lightPositions[4];
    vec4 lightColors[4];
    ivec2 lightShadowCount;
} scene;

layout(set = 0, binding = 1) uniform UBOInstance
{
    mat4 model;
} instance;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inTexCoord;

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec3 outWorldPos;

void main() {
    gl_Position = scene.projection * scene.view * instance.model * vec4(inPosition, 1.0);
    outNormal = inNormal;
    outColor = inColor;
    outTexCoord = inTexCoord;

    vec4 pos = instance.model * vec4(inPosition, 1.0);
	outWorldPos = vec3(instance.model * vec4(inPosition, 1.0));
    outNormal = mat3(instance.model) * inNormal;
}
