#version 460
layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 ViewPosition;
    vec4 FragPosLightSpace;
} fs_in;

struct Material {
    float shininess;
    vec3 ambient;
#ifdef USE_ALBEDO_TEXTURE_MAP
    sampler2D diffuse1;
#else
    vec3 diffuse;
#endif

#ifdef USE_SPECULAR_TEXTURE_MAP
    sampler2D specular1;
#else
    vec3 specular;
#endif

#ifdef USE_NORMAL_TEXTURE_MAP
    sampler2D normal1;
#endif
};

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Object {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 normal;
};

uniform Material uObjectMaterial;
uniform DirectionalLight uDirLight;

uniform sampler2D uShadowMap;

vec4 GetDirectionalLight(DirectionalLight light, Object object);
float ShadowCalculation(DirectionalLight light, Object object);

void main() {
    Object object;
    object.ambient = uObjectMaterial.ambient;

#ifdef USE_ALBEDO_TEXTURE_MAP
    object.diffuse = texture(uObjectMaterial.diffuse1, fs_in.TexCoords).rgb; 
#else
    object.diffuse = uObjectMaterial.diffuse;
#endif

#ifdef USE_SPECULAR_TEXTURE_MAP
    #ifdef SPECULAR_IS_GRAY
        object.specular = texture(uObjectMaterial.specular1, fs_in.TexCoords).rrr; 
    #else
        object.specular = texture(uObjectMaterial.specular1, fs_in.TexCoords).rgb; 
    #endif
#else
    object.specular = uObjectMaterial.specular;
#endif

#ifdef USE_NORMAL_TEXTURE_MAP
    object.normal = normalize(texture(uObjectMaterial.normal1, fs_in.TexCoords).rgb * 2.0 - 1.0);
#else
    object.normal = normalize(fs_in.Normal);
#endif

#ifdef USE_ALBEDO_AS_AMBIENT
    object.ambient = object.diffuse;
#endif

    FragColor = GetDirectionalLight(uDirLight, object);
}

vec4 GetDirectionalLight (DirectionalLight light, Object object) {
#ifndef DISABLE_AMBIENT
    vec3 ambient = object.ambient * light.ambient;
#else
    vec3 ambient = vec3(0.0);
#endif

    vec3 normalDirection = object.normal;
    vec3 lightDirection = normalize(-light.direction);

#ifndef DISABLE_DIFFUSE
    vec3 diffuse = max(dot(normalDirection, lightDirection), 0.0) * object.diffuse * light.diffuse;
#else
    vec3 diffuse = vec3(0.0);
#endif

#ifndef DISABLE_SPECULAR
    vec3 viewDirection = normalize(fs_in.ViewPosition - fs_in.FragPos);
    #ifdef ONLY_PHONG
        vec3 reflectDirection = reflect(-lightDirection, normalDirection);
        vec3 specular = pow(max(dot(viewDirection, reflectDirection), 0.0), uObjectMaterial.shininess) * object.specular * light.specular;
    #else
        vec3 halfwayDirection = normalize(lightDirection + viewDirection);
        vec3 specular = pow(max(dot(normalDirection, halfwayDirection), 0.0), uObjectMaterial.shininess) * object.specular * light.specular;
    #endif
#else
    vec3 specular = vec3(0.0);
#endif

    float shadow = ShadowCalculation(light, object);
    vec3 result = ambient + (diffuse + specular) * (1.0 - shadow);
    return vec4(result, 1.0);
}

float ShadowCalculation(DirectionalLight light, Object object) {
    vec3 normalDirection = object.normal;
    vec3 lightDirection = normalize(-light.direction);

    vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0) 
        return 0.0;

    //float closestDepth = texture(uShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.05f * (1.0 - dot(normalDirection, lightDirection)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0/textureSize(uShadowMap, 0);
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += step(pcfDepth, currentDepth - bias);
        }
    }
    shadow /= 9.0;

    return shadow;
}
