#version 460
layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 ViewPosition;
} fs_in;

uniform sampler2D uAlbedoMap;

void main() {
    vec4 texColor = texture(uAlbedoMap, fs_in.TexCoords);
    if(texColor.a < 0.1)
       discard;
    FragColor = texColor;
}
