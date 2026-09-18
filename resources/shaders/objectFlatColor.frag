#version 460
layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
} fs_in;

uniform vec3 uAlbedoFlatColor;
uniform sampler2D uAlbedoMap;

uniform vec3 uLightColor;

void main() {
#ifdef USE_ALBEDO_TEXTURE
    vec3 albedoColor = texture(uAlbedoMap, fs_in.TexCoords).rgb;
#else
    vec3 albedoColor = uAlbedoFlatColor;
#endif

    FragColor = vec4(albedoColor * uLightColor, 1.0);
}

