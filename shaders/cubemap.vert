#version 450
layout(location = 0) in vec3 inPosition;

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

layout(location = 0) out vec3 fragTexCoord;

void main() {
    fragTexCoord = inPosition;
    mat4 viewWithoutTranslation = mat4(mat3(ubo.view)); // Remove translation
    vec4 clipPos = ubo.projection * viewWithoutTranslation * vec4(inPosition, 1.0);
    gl_Position = clipPos.xyww; // Force depth to 1.0
}
