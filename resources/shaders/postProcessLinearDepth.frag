#version 460 core
layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uScreenTexture;

uniform float near = 0.1;
uniform float far = 100.0;

float LinearizeDepth(float depth) {
    float ndc = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - ndc * (far - near));
}

void main() {
    float depth = LinearizeDepth(texture(uScreenTexture, TexCoords).r) / far;
    FragColor = vec4(vec3(depth), 1.0);
}
