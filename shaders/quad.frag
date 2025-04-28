#version 450
layout(binding = 0) uniform UniformBuffer {
    vec2 windowSize;
    float scale;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

float gammaFactor = 1.0f;

void main() {
    vec2 uv = gl_FragCoord.xy;

    uv.x /= round(320.0f * ubo.scale);
    uv.y /= round(240.0f * ubo.scale);
    
    vec3 albedo = texture(texSampler, uv).rgb;
    

    vec3 quantized = albedo;

    int ps1_dither_matrix[16] = {
	    		-4, 0, -3, 1,
	    		2, -2, 3, -1,
	    		-3, 1, -4, 0,
	    		3, -1, 2, -2
		};
    
    vec2 texelCord = uv * vec2(320.0, 240.0);


    float noise = float(ps1_dither_matrix[(int(texelCord.x) % 4) + (int(texelCord.y) % 4) * 4]);
    
    quantized = pow(albedo, vec3(1.0 / gammaFactor));
    quantized = round(quantized * 255.0 + noise);
    quantized = clamp(round(quantized), vec3(0.0), vec3(255.0));
    quantized = clamp(quantized / 8.0, vec3(0), vec3(31));
    quantized /= 31.0;
    quantized = pow(quantized, vec3(gammaFactor));
    


    outColor = vec4(quantized, 1.0);
}
