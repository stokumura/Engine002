#version 460
layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 ViewPosition;
} fs_in;

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight uDirLight;
uniform float uSpecular;
uniform float uShininess;

vec4 GetDirectionalLight (DirectionalLight light, vec3 albedo) {
#ifndef DISABLE_AMBIENT
    vec3 ambient = albedo * light.ambient;
#else
    vec3 ambient = vec3(0.0);
#endif

    vec3 normalDirection = fs_in.Normal;
    vec3 lightDirection = normalize(-light.direction);

#ifndef DISABLE_DIFFUSE
    vec3 diffuse = max(dot(normalDirection, lightDirection), 0.0) * albedo * light.diffuse;
#else
    vec3 diffuse = vec3(0.0);
#endif

#ifndef DISABLE_SPECULAR
    vec3 viewDirection = normalize(fs_in.ViewPosition - fs_in.FragPos);
    vec3 reflectDirection = reflect(-lightDirection, normalDirection);

    vec3 specular = pow(max(dot(viewDirection, reflectDirection), 0.0), uShininess) * uSpecular * light.specular;
#else
    vec3 specular = vec3(0.0);
#endif

    vec3 result = ambient + diffuse + specular;
    return vec4(result, 1.0);
}

void main() {
    FragColor = GetDirectionalLight(uDirLight, vec3(0.0, 0.65, 0.0));
}
