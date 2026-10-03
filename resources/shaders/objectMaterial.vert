#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 ViewPosition;
} vs_out;

layout(std140) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec3 viewPosition;
};

uniform mat4 model;
uniform mat3 normal;

void main() {
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));

#ifndef USE_NORMAL_TEXTURE_MAP
    #ifdef USE_NORMAL_MATRIX
        vs_out.Normal = normal * aNormal;
    #else
        vs_out.Normal = mat3(transpose(inverse(model))) * aNormal; 
    #endif
#endif

    vs_out.ViewPosition = viewPosition;

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}

