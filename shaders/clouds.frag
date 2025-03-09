#version 450

layout(set = 1, binding = 0) uniform sampler3D worleyNoise;
layout(set = 1, binding = 1) uniform sampler3D erodeNoise;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.0, 0.0, 0.0, 0.0);
}
