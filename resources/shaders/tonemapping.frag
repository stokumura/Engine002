#version 460 core
layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uScreenTexture;
#ifdef USE_BLOOM
uniform sampler2D uBloomBlur;
#endif
uniform float uExposure;

void main() {
    const float gamma = 2.2;
    vec3 hdrColor = texture(uScreenTexture, TexCoords).rgb;
#ifdef USE_BLOOM
    vec3 bloomColor = texture(uBloomBlur, TexCoords).rgb;
    hdrColor += bloomColor;
#endif

    vec3 mapped = vec3(1.0) - exp(-hdrColor * uExposure);

#ifdef USE_GAMMA_CORRECTION
    mapped = pow(mapped, vec3(1.0 / gamma));
#endif

    FragColor = vec4(mapped, 1.0);
}
