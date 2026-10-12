#version 460 core
layout (local_size_x = 8, local_size_y = 8) in;

struct SingleLeafData {
    float height;
    vec2 direction;
    vec3 position;
};

layout(std430, binding = 0) writeonly buffer Instances {
    SingleLeafData instances[];
};

layout(std430, binding = 1) buffer Indirect {
    uint count;
    uint instanceCount;
    uint firstIndex;
    int baseVertex;
    uint baseInstance;
};

layout(std140, binding = 2) uniform CameraData {
    mat4 view;
    mat4 projection;
    vec3 viewPosition;
};


uniform int uGridX;
uniform int uGridY;
uniform float uSpacing;
uniform vec3 uOrigin;
uniform float uMaxDist;
uniform float uHeight;


vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x * 34.0) + 10.0) * x); }

// Devuelve valores aproximadamente en [-1, 1]
float snoise(vec2 v)
{
    const vec4 C = vec4( 0.211324865405187,   // (3.0 - sqrt(3.0)) / 6.0
                         0.366025403784439,   // 0.5 * (sqrt(3.0) - 1.0)
                        -0.577350269189626,   // -1.0 + 2.0 * C.x
                         0.024390243902439);  // 1.0 / 41.0

    // Primera esquina del simplex
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);

    // Otras dos esquinas
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutaciones
    i = mod289(i);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
                             + i.x + vec3(0.0, i1.x, 1.0));

    vec3 m = max(0.5 - vec3(dot(x0, x0),
                            dot(x12.xy, x12.xy),
                            dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;

    // Gradientes
    vec3 x  = 2.0 * fract(p * C.www) - 1.0;
    vec3 h  = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalización de gradientes
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

    vec3 g;
    g.x  = a0.x  * x0.x   + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;

    return 130.0 * dot(m, g);
}

vec3 snoise3(vec2 p)
{
    return vec3(snoise(p),
            snoise(p + vec2(31.416, -47.853)),
            snoise(p + vec2(-59.230, 19.230)));
}

// --- fBm (varias octavas) ----------------------------------------------
float fbm(vec2 p, int octaves, float lacunarity, float gain)
{
    float sum = 0.0;
    float amp = 0.5;
    for (int i = 0; i < octaves; ++i) {
        sum += amp * snoise(p);
        p   *= lacunarity;   // típico: 2.0
        amp *= gain;         // típico: 0.5
    }
    return sum;
}

bool inFrustum(vec3 position, float height)  {
    vec4 clip = projection * view * vec4(position + vec3(0.0, height * 0.5, 0.0), 1.0);
    float margin = clip.w * 1.1 + 0.5;
    return clip.w > 0.0 && abs(clip.x) <= margin && abs(clip.y) <= margin && clip.z > -margin;
}

void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    if(id.x >= uGridX || id.y >= uGridY) return;

    float basePositionX = (id.x - uGridX / 2) * uSpacing + uOrigin.x;
    float basePositionZ = (id.y - uGridY / 2) * uSpacing + uOrigin.z;

    vec3 randomOffset = snoise3(vec2(id)); 
    float height = uHeight + randomOffset.y + 0.5;

    vec3 position = vec3(basePositionX + randomOffset.x, uOrigin.y, basePositionZ + randomOffset.z);
    if(distance(position, viewPosition) > uMaxDist) return;
    if(!inFrustum(position, height)) return;

    SingleLeafData data;
    data.height = height;
    data.direction = normalize(randomOffset.xz);
    data.position = position;

    uint idx = atomicAdd(instanceCount, 1u);
    instances[idx] = data;
}
