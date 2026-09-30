#version 460
layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 ViewPosition;
} fs_in;

uniform vec3 uAlbedoFlatColor;
uniform sampler2D uAlbedoMap;

uniform vec3 uLightColor;
uniform vec3 uLightPosition;

void main() {
#ifdef USE_ALBEDO_TEXTURE_MAP
    vec3 albedoColor = texture(uAlbedoMap, fs_in.TexCoords).rgb;
#else
    vec3 albedoColor = uAlbedoFlatColor;
#endif

#ifndef DISABLE_AMBIENT
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * uLightColor;
#else
    vec3 ambient = vec3(0.0);
#endif

    vec3 normalDirection = normalize(fs_in.Normal);
    vec3 lightDirection = normalize(uLightPosition - fs_in.FragPos);

#ifndef DISABLE_DIFFUSE
    vec3 diffuse = max(dot(normalDirection, lightDirection), 0.0) * uLightColor;
#else
    vec3 diffuse = vec3(0.0);
#endif

#ifndef DISABLE_SPECULAR
    float specularStrength = 0.5;
    vec3 viewDirection = normalize(fs_in.ViewPosition - fs_in.FragPos);
    vec3 reflectDirection = reflect(-lightDirection, normalDirection);

    float shininess = 32.0;
    vec3 specular = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess) * specularStrength * uLightColor;
#else
    vec3 specular = vec3(0.0);
#endif

    vec3 result = (ambient + diffuse + specular) * albedoColor;
    FragColor = vec4(result, 1.0);
}

