#version 460 core
layout (location = 0) out vec4 FragColor;

in vec3 ViewPosition;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
};

uniform Light light;

void main() {
    vec2 TexCoords = gl_FragCoord.xy / vec2(textureSize(gPosition, 0));
    vec4 FragPos = texture(gPosition, TexCoords);
    if(FragPos.a == 0.0) discard;

    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec4 AlbedoSpec = texture(gAlbedoSpec, TexCoords);
    vec3 Albedo = AlbedoSpec.rgb;
    float Specular = AlbedoSpec.a;

    vec3 lighting = vec3(0.0);
    vec3 viewDir = normalize(ViewPosition - FragPos.xyz);
    float lightDistance = length(light.Position - FragPos.xyz);
    vec3 lightDir = normalize(light.Position - FragPos.xyz);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * light.Color;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = light.Color * spec * Specular;

    float attenuation = 1.0 / (1.0 + light.Linear * lightDistance + light.Quadratic * lightDistance * lightDistance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    FragColor = vec4(lighting, 1.0);
}
