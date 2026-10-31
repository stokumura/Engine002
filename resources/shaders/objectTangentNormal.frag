#version 460 core
layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 ViewPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

#ifdef USE_PARALLAX_MAP
    uniform sampler2D depthMap;
    uniform float uHeightScale;
#endif

uniform vec3 uLightPosition;

#ifdef USE_PARALLAX_MAP
    vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);
#endif

void main() {
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
#ifdef USE_PARALLAX_MAP
    vec2 texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;
#else
    vec2 texCoords = fs_in.TexCoords;
#endif

    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 color = texture(diffuseMap, texCoords).rgb;
    vec3 ambient = color * 0.1;
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(viewDir + reflectDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}

#ifdef USE_PARALLAX_MAP
#ifdef USE_STEEP_PARALLAX_MAP
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;

    vec2 P = viewDir.xy / viewDir.z * uHeightScale;
    vec2 deltaTexCoords = P / numLayers;

    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue) {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}
#else
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    float height = texture(depthMap, texCoords).r;
    vec2 P = viewDir.xy / viewDir.z * (height * uHeightScale);
    return texCoords - P;
}
#endif
#endif
