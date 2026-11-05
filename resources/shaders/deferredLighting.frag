#version 460 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPosition;

void main() {
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec4 AlbedoSpec = texture(gAlbedoSpec, TexCoords);
    vec3 Albedo = AlbedoSpec.rgb;
    float Specular = AlbedoSpec.a;

    vec3 lighting = Albedo * 0.1;
    vec3 viewDir = normalize(viewPosition - FragPos);
    for(int i = 0; i < NR_LIGHTS; i++) {
        float lightDistance = length(lights[i].Position - FragPos);
        if(lightDistance < lights[i].Radius) {
            vec3 lightDir = normalize(lights[i].Position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lights[i].Color;

            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].Color * spec * Specular;

            float attenuation = 1.0 / (1.0 + lights[i].Linear * lightDistance + lights[i].Quadratic * lightDistance * lightDistance);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }

    FragColor = vec4(lighting, 1.0);
}
