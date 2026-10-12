#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

struct SingleLeafData {
    float height;
    vec2 direction;
    vec3 position;
};

layout(std430, binding = 0) readonly buffer Instances {
    SingleLeafData instances[];
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
    SingleLeafData instance = instances[gl_InstanceID];
    vec3 basePos = instance.position;
    float height = instance.height;
    vec3 pos = aPos.yxz / 10.0;
    pos.y *= -1;

    vec3 forward = normalize(vec3(instance.direction.x, 0.0, instance.direction.y));
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, forward);

    vec3 worldPos = basePos + right * pos.x + vec3(0.0, pos.y * height, 0.0);

    float sway = sin(uTime * 2.0 + basePos.x * 0.5 + basePos.z * 0.3) * 0.2;
    worldPos.x += sway * pos.y * pos.y;

    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = worldPos;

    vs_out.Normal = normalize(right * aNormal.x + up * aNormal.y / height + forward * aNormal.z); 

    vs_out.ViewPosition = viewPosition;

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}

