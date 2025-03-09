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
    mat4 inverseProjection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(set = 0, binding = 1) uniform samplerCube skybox;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D emissiveMap;
layout(set = 1, binding = 3) uniform sampler2D aoMap;
layout(set = 1, binding = 4) uniform sampler2D metallicRoughnessMap;

const float PI = 3.1415926538;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main() {
    vec3 albedo = vec3(1.0);
    if (push.useMaps.y > 0.0) {
        albedo = texture(albedoMap, fragUV).rgb;
    }
    vec3 emmisive = texture(emissiveMap, fragUV).rgb;
    float ao = 1;
    if (push.useMaps.w > 0.0) {
        ao = texture(aoMap, fragUV).r;
    }

    float metallic = texture(metallicRoughnessMap, fragUV).b;
    float roughness = texture(metallicRoughnessMap, fragUV).g;

    vec3 surfaceNormal = fragNormalWorld;

    if (push.useMaps.x > 0.0) {
        vec3 normal = texture(normalMap, fragUV).rgb;
        normal = normalize(normal * 2.0 - 1.0);

        vec3 T = normalize(fragTangent);
        vec3 B = normalize(fragBitangent);
        vec3 N = normalize(fragNormalWorld);

        mat3 TBN = mat3(T, B, N);
        surfaceNormal = normalize(TBN * normal);
    }

    vec3 cameraPositionWorld = ubo.invView[3].xyz;
    vec3 V = normalize(cameraPositionWorld - fragPosWorld);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < ubo.numLights; i++) {
        PointLight light = ubo.pointLights[i];
        vec3 L = normalize(light.position.xyz - fragPosWorld);
        vec3 H = normalize(V + L);
        float distance = length(light.position.xyz - fragPosWorld);
        vec3 radiance = light.color.xyz * light.color.w;

        // cook-torrance brdf
        float NDF = DistributionGGX(surfaceNormal, H, roughness);
        float G = GeometrySmith(surfaceNormal, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(surfaceNormal, V), 0.0) * max(dot(surfaceNormal, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        // add to outgoing radiance Lo
        float NdotL = max(dot(surfaceNormal, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w * albedo * ao;

    vec3 R = reflect(-V, surfaceNormal);
    vec3 reflectionColor = texture(skybox, R).rgb;
    vec3 F_env = fresnelSchlick(max(dot(surfaceNormal, V), 0.0), F0);
    vec3 envSpecular = F_env * reflectionColor * (1.0 - roughness) * ao;

    vec3 color = ambient + Lo + envSpecular;

    if(push.useMaps.z > 0) {
        color += emmisive;
    }
    outColor = vec4(color, 1.0);
}
