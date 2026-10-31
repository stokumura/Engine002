#version 460 core
layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uScreenTexture;

void main() {
    vec3 color = vec3(texture(uScreenTexture, TexCoords));
    float average = (color.r + color.g + color.b) / 3.0;
    FragColor = vec4(vec3(average), 1.0);
}
