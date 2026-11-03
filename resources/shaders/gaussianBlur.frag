#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uScreenTexture;

uniform bool uHorizontal;
uniform float weight[5] = float[] ( 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 );

void main() {
    vec2 texOffset = 1.0 / textureSize(uScreenTexture, 0);
    texOffset = mix(vec2(0, texOffset.y), vec2(texOffset.x, 0), float(uHorizontal));

    vec3 result = texture(uScreenTexture, TexCoords).rgb * weight[0];
    for(int i = 1; i < 5; i++) {
        result += texture(uScreenTexture, TexCoords + texOffset * i).rgb * weight[i];
        result += texture(uScreenTexture, TexCoords - texOffset * i).rgb * weight[i];
    }

    FragColor = vec4(result, 1.0);
}
