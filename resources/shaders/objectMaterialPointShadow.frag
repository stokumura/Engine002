#version 460
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

#ifdef USE_NORMAL_TEXTURE_MAP
    sampler2D normal1;
#endif
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

struct Object {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 normal;
};

uniform Material uObjectMaterial;
uniform PointLight uPointLight;

uniform samplerCube uShadowMap;
uniform float uFarPlaneShadow;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

vec4 GetPointLight(PointLight light, Object object);
float ShadowCalculation(PointLight light, Object object);

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

    FragColor = GetPointLight(uPointLight, object);
}

vec4 GetPointLight (PointLight light, Object object) {
#ifndef DISABLE_AMBIENT
    vec3 ambient = object.ambient * light.ambient;
#else
    vec3 ambient = vec3(0.0);
#endif

    vec3 normalDirection = object.normal;
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
    result *= attenuation;
    return vec4(result, 1.0);
}

float ShadowCalculation(PointLight light, Object object) {
    vec3 fragToLight = fs_in.FragPos - light.position;
    float closestDepth = texture(uShadowMap, fragToLight).r;
    closestDepth *= uFarPlaneShadow;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.05;
    int samples = 20;
    float viewDistance = length(fs_in.ViewPosition - fs_in.FragPos);
    float diskRadius = (1.0 + (viewDistance / uFarPlaneShadow)) / 25.0;
    for(int i = 0; i < samples; i++) {
        float closestDepth = texture(uShadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= uFarPlaneShadow;
        shadow += step(closestDepth, currentDepth - bias);
    }
    shadow /= float(samples);

    return shadow;
}
