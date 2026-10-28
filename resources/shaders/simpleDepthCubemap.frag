#version 460 core
in vec4 FragPos;

uniform vec3 uLightPosition;
uniform float uFarPlane;

void main() {
    float lightDistance = length(FragPos.xyz - uLightPosition);
    lightDistance = lightDistance / uFarPlane;
    gl_FragDepth = lightDistance;
}
