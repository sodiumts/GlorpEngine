#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 clipPos;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D emissiveMap;
layout(set = 1, binding = 3) uniform sampler2D aoMap;
layout(set = 1, binding = 4) uniform sampler2D metallicRoughnessMap;



struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix; // Projection * view * model
    mat4 normalMatrix;
} push;

void main() {
    // undo perspective correction
    vec2 uv = fragUV / clipPos.w;


    uv = vec2(fract(uv.x), 1 - fract(uv.y)); // return uv to [0,1] so there is no texture repeating

    uv.y = 1.0 - uv.y; // flip the y for correct texture mapping for default gltf export on blender

    vec3 albedo = texture(albedoMap, uv).rgb;
    
        
    outColor = vec4(albedo, 1.0);
}
