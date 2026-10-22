#version 460 core
layout(location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 ViewPosition;
} fs_in;

uniform samplerCube uSkybox;

void main() {
    vec3 viewDirection = normalize(fs_in.FragPos - fs_in.ViewPosition);
    vec3 reflection = reflect(viewDirection, normalize(fs_in.Normal));
    FragColor = vec4(texture(uSkybox, reflection).rgb, 1.0);
}
