// Requires: "#extension GL_EXT_nonuniform_qualifier : enable" at the start of the shader

#define LightType_None 0u
#define LightType_Point 1u
#define LightType_Spot 2u
#define LightType_Directional 3u
#define LightFlags_TypeMask 0x7u
#define LightFlags_ShadowCaster 0x8u

struct Light
{
    uint flags;
    float red;
    float green;
    float blue;
    vec4 position;
    vec4 direction;
    vec4 extraData;
    mat4 shadowMatrix;
};

#ifndef LIGHT_GAMMA_CORRECT
#define LIGHT_GAMMA_CORRECT(c) pow(c, vec3(0.4545f))
#endif

// Define NO_LIGHT_DESCRIPTORS to not create the lighting buffer
// Use LIGHT_DESCRIPTOR_SET to define the set for the lighting information
#ifndef NO_LIGHT_DESCRIPTORS
#ifndef LIGHT_DESCRIPTOR_SET
#define LIGHT_DESCRIPTOR_SET 2
#endif

layout(set = LIGHT_DESCRIPTOR_SET, binding = 0) uniform LightInfo
{
    uint lightCount;
    uint shadowCount;
    vec2 padding;
} lightInfo;

layout(set = LIGHT_DESCRIPTOR_SET, binding = 1) readonly buffer LightBuffer
{
    Light lights[];
} lightBuffer;

layout(set = LIGHT_DESCRIPTOR_SET, binding = 2) uniform sampler2DShadow ShadowTextures[];
layout(set = LIGHT_DESCRIPTOR_SET, binding = 2) uniform samplerCubeShadow ShadowCubeTextures[];

#endif// !NO_LIGHT_DESCRIPTORS

// Disable including PBR (maybe it is included elsewhere or because you have your own BRDF that follows the same call)
#ifndef NO_PBR_INCLUDE
#include "pbr.glsl"
#endif

uint GetLightType(uint lightFlags)
{
    return (lightFlags & LightFlags_TypeMask);
}

float CalculateSpotCone(vec3 spotDirection, vec3 lightDirection, float innerSpotAngle, float outerSpotAngle)
{
    float theta = dot(lightDirection, normalize(spotDirection));
    float epsilon = max(outerSpotAngle - innerSpotAngle, 0.0f);
    return clamp((theta - cos(outerSpotAngle)) / epsilon, 0.0f, 1.0f);
}

// Returns lit color
vec3 CalculateLight(int lightNum, vec3 worldPos, vec3 V, vec3 N, float metallic, float roughness, vec3 materialColor)
{
    const float maxBias = 1e-4;
    const float minBias = 5e-5;
    const float directionalMaxBias = 1e-2;
    const float directionalMinBias = 5e-3;

    Light light = lightBuffer.lights[lightNum];

    vec3 L;
    uint lightType = GetLightType(light.flags);
    float attenuation = 1.0f;
    float shadowBias = 0.0;

    if (lightType != LightType_Directional)
    {
        vec3 lightWorldDiff = light.position.xyz - worldPos.xyz;
        L = normalize(lightWorldDiff);
        float lightDistanceSquared = (lightWorldDiff.x * lightWorldDiff.x) + (lightWorldDiff.y * lightWorldDiff.y) + (lightWorldDiff.z * lightWorldDiff.z);
        attenuation = 1.0f / lightDistanceSquared;

        shadowBias = max(maxBias * (1.0 - dot(N, L)), minBias);
    }
    else // Directional lighting
    {
        L = normalize(light.direction.xyz);
        shadowBias = max(directionalMaxBias * (1.0 - dot(N, L)), directionalMinBias);
    }

    vec3 lightColor = vec3(light.red, light.green, light.blue);
    vec3 outColor = BRDF(L, V, N, metallic, roughness, materialColor, lightColor, attenuation);

    // Adjust light color when spotlight based on the spotlight's cone
    if (lightType == LightType_Spot)
    {
        outColor *= CalculateSpotCone(light.direction.xyz, L.xyz, light.extraData.x, light.extraData.y);
    }

    // Shadow
    bool isShadowCaster = (light.flags & LightFlags_ShadowCaster) != 0;
    if (lightNum < lightInfo.shadowCount && isShadowCaster)
    {
        float shadowFactor = 1.0;

        if (lightType == LightType_Point)
        {
            // Cubemap shadow map (TODO)
            // float dist = distance(light.position.xyz, worldPos.xyz);
            // shadowFactor = texture(ShadowCubeTextures[lightNum], vec4(L.xyz, dist));
        }
        else // Standard shadow map
        {
            vec4 shadowPos = light.shadowMatrix * vec4(worldPos, 1.0);
            shadowPos.xyz /= shadowPos.w;
            shadowPos.xy = shadowPos.xy * 0.5 + 0.5;
            shadowPos.z -= shadowBias;

            // Only apply shadowing when shadowPos is in range
            if (shadowPos.x >= 0.0 && shadowPos.x <= 1.0 && shadowPos.y >= 0.0 && shadowPos.y <= 1.0)
            {
                shadowFactor = texture(ShadowTextures[lightNum], shadowPos.xyz).x;
            }
        }

        // Apply shadow factor
        outColor = mix(outColor * 0.00125, outColor, shadowFactor);
    }

    return outColor;
}

#ifndef NO_FINAL_LIGHT_CALCULATION// Don't define the light calculation if not wanted
vec3 CalculateLighting(vec3 worldPos, vec3 V, vec3 N, float metallic, float roughness, vec3 materialColor)
{
    vec3 Lo = vec3(0.0f);
    for (int l = 0; l < lightInfo.lightCount; l++)
    {
        Lo += CalculateLight(l, worldPos, V, N, metallic, roughness, materialColor);
    }

    vec3 color = materialColor * (PBR_AMBIENT_LIGHT);
    color += Lo;

    // Gamma correct if enabled
    #ifndef NO_LIGHT_GAMMA_CORRECT
    color = (LIGHT_GAMMA_CORRECT(color));
    #endif

    return color;
}
#endif// !NO_FINAL_LIGHT_CALCULATION