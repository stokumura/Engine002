#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 ViewPos;
} vs_out;

layout(std140) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec3 viewPosition;
};

uniform vec3 uLightPosition;
uniform mat4 model;

void main() {
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);

    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.TangentLightPos = TBN * uLightPosition;
    vs_out.TangentViewPos = TBN * viewPosition;
    vs_out.TangentFragPos = TBN * vs_out.FragPos;

    vs_out.ViewPos = viewPosition;

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}

