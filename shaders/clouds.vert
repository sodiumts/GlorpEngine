#version 450

layout(location = 0) in vec3 position;

layout(push_constant) uniform Push {
    vec3 sunPos;
} push;

struct PointLight {
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 inverseProjection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

void main() {
    gl_Position = vec4(position.xy, 0.0, 1.0);
}
