#version 460
layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform Uniforms {
    float time;
    float bass;
    float intensity;
    float pad;
    vec2 resolution;
} u;

void main() {
    vec2 p = (uv * 2.0 - 1.0) * vec2(u.resolution.x / u.resolution.y, 1.0);

    float v1 = sin(p.x * 10.0 + u.time * 1.3) * cos(p.y * 8.0 + u.time * 1.1);
    float v2 = sin(p.x * 14.0 - u.time * 2.2) * cos(p.y * 12.0 - u.time * 1.8);
    float v3 = sin(length(p) * 20.0 - u.time * 3.0);

    float plasma = v1 + v2 * 0.6 + v3 * 0.4;
    //plasma *= u.intensity;

    vec3 color = vec3(
        0.5 + 0.5 * sin(plasma * 5.0 + u.time),
        0.4 + 0.6 * cos(plasma * 4.0 + u.time * 1.4),
        0.6 + 0.6 * sin(plasma * 6.0 + u.bass * 3.0)
    );

    color *= 0.7 + u.bass * 1.2;
    color = pow(color, vec3(0.75));

    fragColor = vec4(color, 1.0);
}
