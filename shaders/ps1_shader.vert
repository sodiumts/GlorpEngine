#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec4 clipPos;

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
    vec2 viewportSize;
} push;

void main() {
    mat4 model_matrix = push.modelMatrix;

    vec4 world_space = model_matrix * vec4(position, 1.0);

    vec4 clip = ubo.projection * ubo.view * world_space;

    vec4 vertex = clip;

    vertex.xy = round(clip.xy / clip.w * push.viewportSize.xy) / push.viewportSize.xy * clip.w; // Integer round to snap vertices
 
    clipPos = vertex;

    gl_Position = vertex;

    fragPos = vertex.xyz;
    fragColor = color * clip.w; // For affine mapping 
    fragUV = uv * clip.w;
}
