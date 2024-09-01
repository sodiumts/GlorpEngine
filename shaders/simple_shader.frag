#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBitangent;

layout(push_constant) uniform Push {
    mat4 modelMatrix; // Projection * view * model
    mat4 normalMatrix;
    vec4 useMaps;
} push;

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

layout(set = 1, binding = 0) uniform sampler2D albedo;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D emissiveMap;
layout(set = 1, binding = 3) uniform sampler2D aoMap;
layout(set = 1, binding = 4) uniform sampler2D metallicRoughnessMap;

void main() {
    vec3 specularLight = vec3(0.0);
    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;

    vec3 surfaceNormal = fragNormalWorld;

    if (push.useMaps.x > 0.0) {
        vec3 normalMapNormal = texture(normalMap, fragUV).rgb * 2.0 - 1.0;

        // Transform normal from tangent space to world space
        vec3 T = normalize(fragTangent);
        vec3 B = normalize(fragBitangent);
        vec3 N = normalize(fragNormalWorld);

        mat3 TBN = mat3(T, B, N); // Tangent-Bitangent-Normal matrix
        surfaceNormal = normalize(TBN * normalMapNormal);
    }

    vec3 cameraPositionWorld = ubo.invView[3].xyz;
    vec3 viewDirection = normalize(cameraPositionWorld - fragPosWorld);

    for (int i = 0; i < ubo.numLights; i++) {
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight);
        directionToLight = normalize(directionToLight);

        float cosAngInc = max(dot(surfaceNormal, directionToLight), 0.0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngInc;

        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0.0, 1.0);
        blinnTerm = pow(blinnTerm, 512.0);
        specularLight += intensity * blinnTerm;
    }

    vec3 emissiveColor = vec3(0.0);
    float aoFactor = 1.0;
    vec3 albedoColor = vec3(1.0);

    if (push.useMaps.y > 0.0) {
        albedoColor = texture(albedo, fragUV).rgb;
    }

    if (push.useMaps.z > 0.0) {
        emissiveColor = texture(emissiveMap, fragUV).rgb;
    }

    if (push.useMaps.w > 0.0) {
        aoFactor = texture(aoMap, fragUV).r;
    }

    vec3 finalColor = (diffuseLight * fragColor + specularLight * fragColor) * albedoColor;
    finalColor *= aoFactor;
    finalColor += emissiveColor;

    outColor = vec4(finalColor, 1.0);
}
