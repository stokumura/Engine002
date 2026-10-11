#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

layout(std430, binding = 0) readonly buffer Instances {
    vec4 instances[];
};

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

uniform vec3 uCamRight;
uniform float uTime;

void main() {
    vec4 instance = instances[gl_InstanceID];
    vec3 basePos = instance.xyz;
    float height = instance.w;

    vec3 right = normalize(vec3(uCamRight.x, 0.0, uCamRight.z));
    vec3 worldPos = basePos + right * aPos.x + vec3(0.0, aPos.y * height, 0.0);

    float sway = sin(uTime * 2.0 + basePos.x * 0.5 + basePos.z * 0.3) * 0.15;
    worldPos.x += sway * aPos.y * aPos.y;

    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = worldPos;

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 forward = cross(right, up);

    vs_out.Normal = normalize(right * aNormal.x + up * aNormal.y / height + forward * aNormal.z); 

    vs_out.ViewPosition = viewPosition;

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}

