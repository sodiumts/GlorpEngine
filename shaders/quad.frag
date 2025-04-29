#version 450
layout(binding = 0) uniform UniformBuffer {
    vec2 windowSize;
    float scale;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D depthSampler;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    bool fogEnabled;
    float nearPlane;
    float farPlane;
    float fogStart;
    float fogEnd;
    vec3 fogColor;
    vec3 cameraPos;
} push;

float gammaFactor = 1.0f;


float LinearizeDepth(float z) {
    return push.nearPlane * push.farPlane / 
        (push.farPlane - z * (push.farPlane - push.nearPlane));
}

void main() {
    vec2 uv = gl_FragCoord.xy / ubo.windowSize;

    
    vec3 albedo = texture(texSampler, uv).rgb;
  
    vec3 foggedColor = albedo;
    if(push.fogEnabled == true) {
        float zVal = texture(depthSampler, uv).r;

        float linearDepth = LinearizeDepth(zVal);

        float fogFactor = clamp((linearDepth - push.fogStart) / 
                                (push.fogEnd - push.fogStart), 0.0, 1.0);

        foggedColor = mix(albedo, push.fogColor, fogFactor);
    }

    int ps1_dither_matrix[16] = int[](
        -4, 0, -3, 1,
        2, -2, 3, -1,
        -3, 1, -4, 0,
        3, -1, 2, -2
    );
    
    vec2 texelCord = uv * vec2(round(ubo.windowSize.x / ubo.scale), round(ubo.windowSize.y / ubo.scale));
    float noise = float(ps1_dither_matrix[(int(texelCord.x) % 4) + 
                        (int(texelCord.y) % 4) * 4]);

    vec3 quantized = pow(foggedColor, vec3(1.0 / gammaFactor));
    quantized = round(quantized * 255.0 + noise);
    quantized = clamp(round(quantized), vec3(0.0), vec3(255.0));
    quantized = clamp(quantized / 8.0, vec3(0), vec3(31));
    quantized /= 31.0;
    quantized = pow(quantized, vec3(gammaFactor));
    
    outColor = vec4(quantized, 1.0);
}
