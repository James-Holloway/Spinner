#ifndef SCENE_DESCRIPTOR_SET
#define SCENE_DESCRIPTOR_SET 1
#endif

layout(set = SCENE_DESCRIPTOR_SET, binding = 0) uniform Scene {
    mat4 viewProjection;
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    vec2 cameraExtent;
    float time;
    float deltaTime;
};
