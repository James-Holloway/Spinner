#version 450

#include "pbr.glsl"

layout(set = 0, binding = 0) uniform UBOScene {
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    vec4 lightPositions[4];
    vec4 lightColors[4];
    ivec2 lightShadowCount;
} scene;

layout (location = 0) in vec2 inTexCoord;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec3 inWorldPos;

layout (binding = 2) uniform sampler2D texSampler;

layout (push_constant) uniform MaterialPushConstants
{
    float roughness;
    float metallic;
    float red;
    float green;
    float blue;
} material;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 N = normalize(inNormal);
	vec3 V = normalize(scene.cameraPosition - inWorldPos);

    vec3 materialColor = pow(texture(texSampler, inTexCoord).xyz, vec3(2.2)) * vec3(material.red, material.green, material.blue);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < scene.lightShadowCount.x; i++)
    {
        vec3 lightPos = scene.lightPositions[i].xyz;
        float lightDistance = length(lightPos - inWorldPos);
        float attenuation = 1.0 / (lightDistance * lightDistance);
        vec3 L = normalize(scene.lightPositions[i].xyz - inWorldPos);
        vec3 lightColor = scene.lightColors[i].xyz * scene.lightColors[i].w;
        vec3 light = BRDF(L, V, N, material.metallic, material.roughness, materialColor, lightColor, attenuation);

        Lo += light;
    }

    // Combine light with ambient
    vec3 color = materialColor * ambient;
    color += Lo;

    // Gamma correct
    color = pow(color, vec3(0.4545));

    outColor = vec4(color, 1.0);
}
