#version 460 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

struct Material {
    sampler2D diffuse1;
    sampler2D specular1;
    sampler2D normal1;
};

uniform Material uObjectMaterial;

void main() {
    gPosition = vec4(fs_in.FragPos, 1.0);

    vec3 normal = texture(uObjectMaterial.normal1, fs_in.TexCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(fs_in.TBN * normal);
    gNormal = normal;

    gAlbedoSpec.rgb = texture(uObjectMaterial.diffuse1, fs_in.TexCoords).rgb;
    gAlbedoSpec.a = texture(uObjectMaterial.specular1, fs_in.TexCoords).r;
}
