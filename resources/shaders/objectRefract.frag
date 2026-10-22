#version 460 core
layout(location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 ViewPosition;
} fs_in;

uniform samplerCube uSkybox;
uniform float uRefractionRatio;

void main() {
    vec3 viewDirection = normalize(fs_in.FragPos - fs_in.ViewPosition);
    vec3 refraction = refract(viewDirection, normalize(fs_in.Normal), uRefractionRatio);
    FragColor = vec4(texture(uSkybox, refraction).rgb, 1.0);
}
