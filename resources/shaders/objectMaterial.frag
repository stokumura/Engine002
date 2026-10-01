#version 460
#ifndef MAX_DIRECTIONAL_LIGHTS_CAPACITY
    #define MAX_DIRECTIONAL_LIGHTS_CAPACITY 4
#endif
#ifndef MAX_POINT_LIGHTS_CAPACITY
    #define MAX_POINT_LIGHTS_CAPACITY 4
#endif
#ifndef MAX_SPOT_LIGHTS_CAPACITY
    #define MAX_SPOT_LIGHTS_CAPACITY 4
#endif
layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 ViewPosition;
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
};

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct Object {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material uObjectMaterial;

uniform int uNumDirLights;
uniform int uNumPointLights;
uniform int uNumSpotLights;

uniform DirectionalLight uDirLights[MAX_DIRECTIONAL_LIGHTS_CAPACITY];
uniform PointLight uPointLights[MAX_POINT_LIGHTS_CAPACITY];
uniform SpotLight uSpotLights[MAX_SPOT_LIGHTS_CAPACITY];

vec4 GetPointLight(PointLight light, Object object);
vec4 GetDirectionalLight(DirectionalLight light, Object object);
vec4 GetSpotLight(SpotLight light, Object object);

void main() {
    Object object;
    object.ambient = uObjectMaterial.ambient;

#ifdef USE_ALBEDO_TEXTURE_MAP
    object.diffuse = texture(uObjectMaterial.diffuse1, fs_in.TexCoords).rgb; 
#else
    object.diffuse = uObjectMaterial.diffuse;
#endif

#ifdef USE_SPECULAR_TEXTURE_MAP
    object.specular = texture(uObjectMaterial.specular1, fs_in.TexCoords).rgb; 
#else
    object.specular = uObjectMaterial.specular;
#endif

#ifdef USE_ALBEDO_AS_AMBIENT
    object.ambient = object.diffuse;
#endif

    vec4 result = vec4(0.0);
    for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS_CAPACITY && i < uNumDirLights; i++) {
        result += GetDirectionalLight(uDirLights[i], object);
    }
    for(int i = 0; i < MAX_POINT_LIGHTS_CAPACITY && i < uNumPointLights; i++) {
        result += GetPointLight(uPointLights[i], object);
    }
    for(int i = 0; i < MAX_SPOT_LIGHTS_CAPACITY && i < uNumSpotLights; i++) {
        result += GetSpotLight(uSpotLights[i], object);
    }
    FragColor = result;
}

vec4 GetPointLight (PointLight light, Object object) {
#ifndef DISABLE_AMBIENT
    vec3 ambient = object.ambient * light.ambient;
#else
    vec3 ambient = vec3(0.0);
#endif

    vec3 normalDirection = normalize(fs_in.Normal);
    vec3 lightDirection = normalize(light.position - fs_in.FragPos);
    float distance = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

#ifndef DISABLE_DIFFUSE
    vec3 diffuse = max(dot(normalDirection, lightDirection), 0.0) * object.diffuse * light.diffuse;
#else
    vec3 diffuse = vec3(0.0);
#endif

#ifndef DISABLE_SPECULAR
    vec3 viewDirection = normalize(fs_in.ViewPosition - fs_in.FragPos);
    vec3 reflectDirection = reflect(-lightDirection, normalDirection);

    vec3 specular = pow(max(dot(viewDirection, reflectDirection), 0.0), uObjectMaterial.shininess) * object.specular * light.specular;
#else
    vec3 specular = vec3(0.0);
#endif

    vec3 result = ambient + diffuse + specular;
    result *= attenuation;
    return vec4(result, 1.0);
}

vec4 GetDirectionalLight (DirectionalLight light, Object object) {
#ifndef DISABLE_AMBIENT
    vec3 ambient = object.ambient * light.ambient;
#else
    vec3 ambient = vec3(0.0);
#endif

    vec3 normalDirection = normalize(fs_in.Normal);
    vec3 lightDirection = normalize(-light.direction);

#ifndef DISABLE_DIFFUSE
    vec3 diffuse = max(dot(normalDirection, lightDirection), 0.0) * object.diffuse * light.diffuse;
#else
    vec3 diffuse = vec3(0.0);
#endif

#ifndef DISABLE_SPECULAR
    vec3 viewDirection = normalize(fs_in.ViewPosition - fs_in.FragPos);
    vec3 reflectDirection = reflect(-lightDirection, normalDirection);

    vec3 specular = pow(max(dot(viewDirection, reflectDirection), 0.0), uObjectMaterial.shininess) * object.specular * light.specular;
#else
    vec3 specular = vec3(0.0);
#endif

    vec3 result = ambient + diffuse + specular;
    return vec4(result, 1.0);
}

vec4 GetSpotLight(SpotLight light, Object object) {
#ifndef DISABLE_AMBIENT
    vec3 ambient = object.ambient * light.ambient;
#else
    vec3 ambient = vec3(0.0);
#endif

    vec3 normalDirection = normalize(fs_in.Normal);
    vec3 lightDirection = normalize(light.position - fs_in.FragPos);
    float distance = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

#ifndef DISABLE_DIFFUSE
    vec3 diffuse = max(dot(normalDirection, lightDirection), 0.0) * object.diffuse * light.diffuse;
#else
    vec3 diffuse = vec3(0.0);
#endif

#ifndef DISABLE_SPECULAR
    vec3 viewDirection = normalize(fs_in.ViewPosition - fs_in.FragPos);
    vec3 reflectDirection = reflect(-lightDirection, normalDirection);

    vec3 specular = pow(max(dot(viewDirection, reflectDirection), 0.0), uObjectMaterial.shininess) * object.specular * light.specular;
#else
    vec3 specular = vec3(0.0);
#endif

    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff)/epsilon, 0.0, 1.0);

    ambient *= intensity;
    specular *= intensity;

    vec3 result = ambient + diffuse + specular;
    result *= attenuation;
    return vec4(result, 1.0);
}
